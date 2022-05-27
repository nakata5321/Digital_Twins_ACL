#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_SCK  (14)
#define PN532_MOSI (13)
#define PN532_SS   (15)
#define PN532_MISO (12)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);


////////////////////////////////////////////////////////
//set private key
String key = "";

uint8_t data1[16] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
uint8_t data2[16] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
uint8_t data3[16] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
uint8_t data4[16] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
uint8_t data5[16] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
///////////////////////////////////////////////////

void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");

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


void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
      Serial.println("Trying to authenticate block 4 with default KEYA value");
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

      //block 4
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);

      if (success)
      {
        Serial.println("Sector 1 (Blocks 4) has been authenticated");
        success = nfc.mifareclassic_WriteDataBlock (4, data1);
        Serial.println("Write data:");
        nfc.PrintHexChar(data1, 16);
        Serial.println("");
      }
      else
      {
        Serial.println("Ooops ... authentication failed: Try another key?");
      }

      //block 5
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 5, 0, keya);

      if (success)
      {
        Serial.println("Sector 1 (Blocks 5) has been authenticated");
        success = nfc.mifareclassic_WriteDataBlock (5, data2);
        Serial.println("Write data:");
        nfc.PrintHexChar(data2, 16);
        Serial.println("");
      }
      else
      {
        Serial.println("Ooops ... authentication failed: Try another key?");
      }

      //block 6
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 6, 0, keya);

      if (success)
      {
        Serial.println("Sector 1 (Blocks 6) has been authenticated");
        success = nfc.mifareclassic_WriteDataBlock (6, data3);
        Serial.println("Write data:");
        nfc.PrintHexChar(data3, 16);
        Serial.println("");
      }
      else
      {
        Serial.println("Ooops ... authentication failed: Try another key?");
      }
      
      //block 8
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 8, 0, keya);

      if (success)
      {
        Serial.println("Sector 2 (Blocks 8) has been authenticated");
        success = nfc.mifareclassic_WriteDataBlock (8, data4);
        Serial.println("Write data:");
        nfc.PrintHexChar(data4, 16);
        Serial.println("");
      }
      else
      {
        Serial.println("Ooops ... authentication failed: Try another key?");
      }

      //block 9
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 9, 0, keya);

      if (success)
      {
        Serial.println("Sector 2 (Blocks9) has been authenticated");
        success = nfc.mifareclassic_WriteDataBlock (9, data5);
        Serial.println("Write data:");
        nfc.PrintHexChar(data5, 16);
        Serial.println("");
      }
      else
      {
        Serial.println("Ooops ... authentication failed: Try another key?");
      }
     
  }
        Serial.println("Card has been written.");
  delay(10000);

}
