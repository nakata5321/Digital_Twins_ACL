 #include <ESP_EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <stdlib.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>


#define SENSOR__NAME "jf13uzaeum"
#define PN532_SCK  (14)
#define PN532_MOSI (13)
#define PN532_SS   (15)
#define PN532_MISO (12)


//login for WI-FI
char wifi_ssid[20]; 
char wifi_password[20];

//MQTT server
char mqtt_server[20];
long mqtt_port = 1883;

const char* AIO_USERNAME = "nakata";
const char* AIO_KEY = "qwerty12345";


char ssid[25];
//SSID for local server


/* Put IP Address details */
IPAddress local_ip(192,168,1,2);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WiFiClient espClient;
PubSubClient client(espClient);
ESP8266WebServer server(80);
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);


void handle_OnConnect();
void handle_submit();
void handle_NotFound();
void MQTTcallback(char* topic, byte* payload, unsigned int length);



void setup() {
  Serial.begin(115200);

  delay(5000);

  strncpy(ssid, "ESP", sizeof(ssid));
  strncat(ssid, SENSOR__NAME, (sizeof(ssid) - strlen(ssid)));

  EEPROM.begin(100);  //Initialize EEPROM

  int check;
  EEPROM.get(0, check);
  if(check !=0){
    Serial.println("read EEPROM");
    EEPROM.get(4, wifi_ssid);
    Serial.print("wi-fi ssid is: ");
    Serial.println(wifi_ssid);

    EEPROM.get(24, wifi_password);
    Serial.print("wi-fi password is: ");
    Serial.println(wifi_password);

    EEPROM.get(44, mqtt_server);
    Serial.print("mqtt_server is: ");
    Serial.println(mqtt_server);

    EEPROM.get(64, mqtt_port);

  }
  else{
    Serial.println("EEPROM empty");
  }


  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);


  for (int i = 0; i < 10; i++){
    if(WiFi.isConnected()){
      Serial.print("Connected to WiFi :");
      Serial.println(WiFi.SSID());
      break;
    }
    else{
      Serial.println("Connecting to WiFi..");
      delay(1000);
    }
  }
  if (WiFi.isConnected()){
    Serial.print("Connected to WiFi :");
    Serial.println(WiFi.SSID());
    client.setKeepAlive(30);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(MQTTcallback);
    while (!client.connected()){
      Serial.println("Connecting to MQTT...");
      if (client.connect(SENSOR__NAME, AIO_USERNAME, AIO_KEY)){
        Serial.println("connected");
      }
      else
      {
        Serial.print("failed with state ");
        Serial.println(client.state());
        delay(2000);
      }
    }
    client.subscribe(SENSOR__NAME);

    //start nfc reader
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (! versiondata) {
      Serial.print("Didn't find PN53x board");
      while (10); // halt
    }
    // Got ok data, print it out!
    Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

    // configure board to read RFID tags
    nfc.SAMConfig();
    Serial.println("Waiting for an ISO14443A Card ...");
  }
  else{
    WiFi.disconnect(true);
     WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);
    WiFi.softAP(ssid);

    server.on("/", handle_OnConnect);
    server.on("/submit", handle_submit);
    server.onNotFound(handle_NotFound);
    server.begin();
    Serial.println("local HTTP server started");
  }
}


uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0};  // Buffer to store the returned UID
uint8_t uidLength;
uint8_t data[16];
uint8_t result[66];


void loop() {
  // Check that all connections alive:
  Serial.printf("Connection WIFI status: %d\n", WiFi.status());
  if (!WiFi.isConnected()){
      WiFi.begin(wifi_ssid, wifi_password);
      while (!WiFi.isConnected()){
      Serial.println("Connecting to WIFI...");
      if (WiFi.isConnected()){
        Serial.println("wifi connected");
      }
      else
      {
        Serial.print("failed with state ");
        Serial.println(WiFi.status());
        delay(2000);
      }
    }
  }

  Serial.printf("Connection MQTT status: %d\n", client.state());
  if (!client.connected()){
      while (!client.connected()){
      Serial.println("Connecting to MQTT...");
      if (client.connect(SENSOR__NAME, AIO_USERNAME, AIO_KEY)){
        Serial.println("MQTT connected");
      }
      else
      {
        Serial.print("failed with state ");
        Serial.println(client.state());
        delay(2000);
      }
    }
    client.subscribe(SENSOR__NAME);
  }



  if(WiFi.isConnected()){
    client.loop();
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    if (success) {
      // Display some basic information about the card
      Serial.println("Found an ISO14443A card");
      Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("  UID Value: ");
      nfc.PrintHex(uid, uidLength);
      Serial.println("");

      Serial.println("Trying to authenticate block 4 with default KEYA value");
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

      //Here we need to collect data from blocks 4, 5, 6, 8, 9
      for(int i = 0; i < 3; i++) {
        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, (i + 4), 0, keya);
        if(success){
          Serial.println("Block " + (String)(i + 4) + "has been authenticated");
          success = nfc.mifareclassic_ReadDataBlock((i + 4), data);
          if(success){
            int shift = i * 16;
            for(int j = 0; j < 16; j++){
              result[shift+j] = data[j];
            }

          } else{
            Serial.println("Ooops ... unable to read the requested block.  Try another key?");
          }
        } else{
          Serial.println("Ooops ... authentication failed: Try another key?");
        }

      }
      //block 8
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 8, 0, keya);
      if(success){
          Serial.println("Block 8 has been authenticated");
          success = nfc.mifareclassic_ReadDataBlock(8, data);
          if(success){
            int shift = 3 * 16;
            for(int j = 0; j < 16; j++){
              result[shift+j] = data[j];
            }

          } else{
            Serial.println("Ooops ... unable to read the requested block.  Try another key?");
          }
      }
      else{
          Serial.println("Ooops ... authentication failed: Try another key?");
      }

      //block 9
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 9, 0, keya);
      if(success){
          Serial.println("Block 9 has been authenticated");
          success = nfc.mifareclassic_ReadDataBlock(9, data);
          if(success){
            int shift = 4 * 16;
            for(int j = 0; j < 2; j++){
              result[shift+j] = data[j];
            }

          } else{
            Serial.println("Ooops ... unable to read the requested block.  Try another key?");
          }
      }
      else{
          Serial.println("Ooops ... authentication failed: Try another key?");
      }

      //publish to mqtt
      Serial.println("Publish to mqtt message:");
      for(int u = 0; u < 66; u++){
        Serial.print(result[u]);
      }
      Serial.println("");
      delay(1000);
      bool f = client.publish(SENSOR__NAME, result, 66);
      Serial.println(f);

      delay(3000);

      
    }
  
  }else{
    server.handleClient(); 

  }
}

// TODO: put SENSORS__NAME inside
const char index_html[] = R"rawliteral(
<!DOCTYPE html> <html>
<head></head>
<body width="100vw" height="100vh">
<h1>SAMP switch settings</h1>
<h4><p>Device UID: jf13uzaeum</p>
<p> Use it to add a new device in mobile app</p>
</h4>

<form action="/submit" method="get">
    <label for="wifi_login">Insert your Wi-Fi SSID:</label>
    <br><input name="wifi_login" id="wifi_login">
  <br>
    <label for="wifi_password">Insert your Wi-Fi password:</label>
    <br>
    <input name="wifi_password" id="wifi_password">
  <br>
    <label for="broker_id">Insert your mqtt broker ip:</label>
    <br>
    <input name="broker_id" id="broker_id">
  <br>
    <label for="broker_port">Insert your mqtt broker port:</label>
    <br>
    <input name="broker_port" id="broker_port" value="1883">
    <br>
    <p>When you are done filling in all forms press restart button.</p> 
    <input type="submit" value="restart">
</form>

</body>
</html>)rawliteral";


void handle_OnConnect() {
  Serial.println("Connect to /");
  server.send(200, "text/html", index_html); 
}

void handle_submit() {
  EEPROM.put(0, 1);
  String str;
  
  //List all parameters
  for (uint8_t i = 0; i < server.args(); i++) {
    
    if (server.argName(i) == "wifi_password"){
      Serial.println("argName is wifi_password");
      str = server.arg(i);
      Serial.print("arg is");
      Serial.println(str);
      
      str.toCharArray(wifi_password , 20);
      EEPROM.put(24, wifi_password);
    }
    
    if (server.argName(i) == "wifi_login"){
      Serial.println("argName is wifi_login");
      str = server.arg(i);
      Serial.print("arg is");
      Serial.println(str);

      str.toCharArray(wifi_ssid , 20);
      EEPROM.put(4, wifi_ssid);
    }

    if (server.argName(i) == "broker_id"){
      Serial.println("argName is mqtt_server");
      str = server.arg(i);
      Serial.print("arg is");
      Serial.println(str);

      str.toCharArray(mqtt_server , 20);
      EEPROM.put(44, mqtt_server);
    }

    if (server.argName(i) == "broker_port"){
      Serial.println("argName is mqtt_port");
      str = server.arg(i);
      Serial.print("arg is");
      Serial.println(str);

      EEPROM.put(64, str.toInt()); 
    }
    
  }

  EEPROM.commit();
  delay(2000);
  server.send(200, "text/plain", "Restarting");
  delay(2000);
  ESP.restart();
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message received in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  String message;
  for (int i = 0; i < length; i++) 
  {
    message = message + (char)payload[i];
  }
  Serial.print(message);
  
  Serial.println();
  Serial.println("-----------------------");
}