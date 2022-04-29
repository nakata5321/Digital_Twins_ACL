# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
import time
from EventToInternet.KeyboardListener import KeyboardListener
import robonomicsinterface as RI
from queue import SimpleQueue, Empty
import typing as tp
import json

from config import read_config

# Queue to collect  data
DATA_QUEUE: SimpleQueue = SimpleQueue()


# class to read data from
class BarcodeKeyboardListener(KeyboardListener):
    """reader class"""
    def dict_handler(self, dict_event):
        """
        :param dict_event: json string with data and information about device
        """
        logging.debug(dict_event)
        DATA_QUEUE.put_nowait(dict_event)


def comparing_addresses(interface: RI.RobonomicsInterface, dt_id: int,  address: str):
    """

    :param interface: robonomics interface
    :param dt_id: number of digital twin
    :param address: address to compare
    :return: True, if found the same address, else false
    """
    dt_info = interface.dt_info(dt_id)
    for i in dt_info:
        if i[1] == address:
            return True
    return False


def get_data(block: bool = False) -> tp.Optional[str]:
    """
    :param block: bool< if TRue block queue
    :return:  json data
    """
    try:
        raw_data = DATA_QUEUE.get(block)
        logging.debug(f"get command: {raw_data}")
        return raw_data
    except Empty:
        logging.debug("the queue is empty")
        return None


def main():
    """
    script for comparing given address with addresses in digital twin
    """
    logging.info("starting program")
    config: tp.Dict[str, str] = read_config()
    logging.debug(f"the config is {config}")
    logging.info("connect to robonomics parachain")
    interface = RI.RobonomicsInterface(remote_ws=config["remote_ws"])

    # create instance of class
    BarcodeKeyboardListener()

    logging.info("waiting to address:")
    try:
        while True:
            raw_data = get_data()
            if raw_data is None:
                time.sleep(4)
            else:
                data = json.load(raw_data)
                if data["name"] == config["name_device"]:
                    result = comparing_addresses(interface, config["dt_id"],
                                                 data["string"])
                    logging.info(f"account in Digital Twin database: {result}")
            time.sleep(1)

    except KeyboardInterrupt:
        logging.debug("shutting down")
        exit()


if __name__ == "__main__":
    main()
