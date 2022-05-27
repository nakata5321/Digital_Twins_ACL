# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import logging
import typing as tp
from queue import SimpleQueue, Empty
from paho.mqtt.client import MQTTMessage
from robonomicsinterface import Account, DigitalTwin, utils
from mqtt_client import create_client, TOPIC_QUEUE, subscribe_to_topic

import config as cg


logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s: %(message)s",
)


def comparing_addresses(interface: DigitalTwin, dt_id: int, address: str) -> bool:
    """

    :param interface: robonomics interface
    :param dt_id: number of digital twin
    :param address: address to compare
    :return: True, if found the same address, else false
    """
    try:
        dt_info = interface.get_info(dt_id)
        for i in dt_info:
            if i[1] == address:
                return True
        return False
    except ConnectionRefusedError:
        logging.error("Not connected to the network!")
        exit()
    except Exception as e:
        logging.error(f"get error: {e}")
        exit()


def get_data(queue: SimpleQueue, block: bool = False) -> tp.Optional[MQTTMessage]:
    """
    function to collect data from queue
    :param queue: the queue from with we wil take data
    :param block: bool< if TRue block queue
    :return:  json data
    """
    try:
        raw_data: MQTTMessage = queue.get(block)
        logging.debug(f"get command: {raw_data}")
        return raw_data
    except Empty:
        logging.debug("the queue is empty")
        return None


def main() -> None:
    """
    script for comparing given address with addresses in digital twin
    """
    logging.info("starting program")
    config: tp.Dict[str, str] = cg.read_config()
    logging.debug(f"the config is {config}")
    logging.info("connect to robonomics parachain")

    interface = Account(remote_ws=config["chain"]["remote_ws"])
    twin_interface = DigitalTwin(interface)
    mqtt_client = create_client()
    mqtt_client.loop_start()
    subscribe_to_topic(mqtt_client, config["mqtt"]["topic"])

    logging.info("waiting to address:")

    try:
        while True:
            topic: MQTTMessage = get_data(TOPIC_QUEUE)
            if topic is None:
                time.sleep(4)
            else:
                logging.info(f"topic message is: {topic.payload.decode()}")
                try:
                    public_key = utils.create_keypair(topic.payload.decode()).ss58_address
                except ValueError:
                    logging.error("get not a private key. sent a private key to the topic")
                    continue
                logging.info(f"pu key is: {public_key}")
                result: bool = comparing_addresses(twin_interface, int(config["chain"]["dt_id"]), public_key)
                logging.info(f"account in Digital Twin database: {result}")

    except KeyboardInterrupt:
        logging.debug("shutting down")
        exit()


if __name__ == "__main__":
    main()
