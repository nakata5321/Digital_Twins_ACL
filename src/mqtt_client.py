# -*- coding: utf-8 -*-

import logging
import random
import typing as tp
import config as cg
from paho.mqtt import client as mqtt_client
from paho.mqtt.client import MQTTMessage
from queue import SimpleQueue

TOPIC_QUEUE: "SimpleQueue[MQTTMessage]" = SimpleQueue()


def _connect_mqtt(client_id: str, broker: str, port: int, username: str, password: str) -> mqtt_client.Client:
    """
    Create and return instance of mqtt client object
    :param client_id: Unique number of client
    :param broker: url of  mqtt broker
    :param port: port of mqtt broker
    :param username: user name for authorize
    :param password: password for authorize
    :return: instance of mqtt client object
    """

    def on_connect(client: mqtt_client.Client, userdata: tp.Any, flags: tp.Dict[tp.Any, tp.Any], rc: int) -> None:
        """
        This is system package function.
        Need to check the success of connection. To more info read doc of package paho_mqtt

        :param client: mqtt client object
        :param userdata: check package documentation for explanation
        :param flags: check package documentation for explanation
        :param rc: status message
        """
        if rc == 0:
            logging.info("Connected to MQTT Broker!")
        else:
            logging.info("Failed to connect, return code %d\n", rc)

    client: mqtt_client.Client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def create_client() -> mqtt_client.Client:
    """
    public function to create mqtt client
    :return: mqtt client object
    """
    config: tp.Dict[str, tp.Dict[str, str]] = cg.read_config()
    # get data from config
    logging.info(type(config["mqtt"]["broker"]))
    broker: str = str(config["mqtt"]["broker"])
    port: int = int(config["mqtt"]["port"])
    username: str = str(config["mqtt"]["username"])
    password: str = str(config["mqtt"]["password"])
    client_id: str = f"python-mqtt-{random.randint(0, 1000)}"

    client: mqtt_client.Client = _connect_mqtt(client_id, broker, port, username, password)
    return client


def subscribe_to_topic(client: mqtt_client.Client, topic: str) -> None:
    """
    in this function client will subscribe to given topic
    and will save all incoming messages from this topic to queue.
    :param client: mqtt client
    :param topic: name of topic
    """

    def on_message(client: mqtt_client.Client, userdata: None, msg: MQTTMessage) -> None:
        """

        :param client: mqtt client object
        :param userdata: check package documentation for explanation
        :param msg: MQTTMessage class. check package documentation for explanation

        """
        global TOPIC_QUEUE
        logging.debug(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        TOPIC_QUEUE.put_nowait(msg)

    client.subscribe(topic)
    client.on_message = on_message
