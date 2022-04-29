# twin scanner

## Installation

First of all:
Go to submodule and install daemon. instruction with installation you can find in submodule [README][db1].

After that go to the main directory with code and install requirements:
```shell
pip install -r requirements.txt
```
then go to `config` subdirectory and create `config.yml` file:
```shell
cd config
cp config_temlate.yml config.yml
```
And fill in config file. To find `name_device` parameter you should connect your device, and run test script in  `test` repository.
```
python3 test/test.py
```
You will se your device name. Please copy it to config file.

## Start
To start the script run in shell next command:
```shell
python3 main.py
```





[db1]: <./feecc-hid-reader-daemon/README.md>