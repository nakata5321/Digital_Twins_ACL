# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import logging
from robonomicsinterface import Account, DigitalTwin
import typing as tp


from config.config import read_config


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
    dt_info = interface.get_info(dt_id)
    for i in dt_info:
        if i[1] == address:
            return True
    return False


def main() -> None:
    """
    script for comparing given address with addresses in digital twin
    """
    logging.info("starting program")
    config: tp.Dict[str, str] = read_config()
    logging.debug(f"the config is {config}")
    logging.info("connect to robonomics parachain")

    interface = Account(remote_ws=config["remote_ws"])
    twin_interface = DigitalTwin(interface)

    data = "4H5zezUf4zT1f2k3KYP2o6BZNhBh29jhsFQdwrfmdFDo58An"

    logging.info("waiting to address:")
    try:
        result = comparing_addresses(twin_interface, int(config["dt_id"]), data)
        logging.info(f"account in Digital Twin database: {result}")

    except KeyboardInterrupt:
        logging.debug("shutting down")
        exit()


if __name__ == "__main__":
    main()
