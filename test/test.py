# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import asyncio
from EventToInternet.KeyboardListener import KeyboardListener


class BarcodeKeyboardListener(KeyboardListener):
    async def dict_handler(self, dict_event):
        data = json.load(dict_event)
        print(f" Your device name is {data['name']}")


BarcodeKeyboardListener()

loop = asyncio.get_event_loop()
loop.run_forever()
