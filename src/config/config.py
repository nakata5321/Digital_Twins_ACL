import typing as tp
import logging
import yaml
import importlib.resources as pkg_resources
import config as cg


def read_config() -> tp.Union[tp.Dict[str, str], tp.Any]:
    """
    Static function to get dictionary from config file
    :rtype: dict
    :return:dictionary with config parameters
    """
    try:
        content = pkg_resources.read_text(cg, "config.yaml")
        config = yaml.load(content, Loader=yaml.FullLoader)
        logging.debug(f"Configuration dict: {content}")
        return config
    except OSError as e:
        logging.error("Error in configuration file!")
        logging.error(e)
        exit()


if __name__ == "__main__":
    read_config()
