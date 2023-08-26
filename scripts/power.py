#!/usr/bin/env python3

import time
from typing import Optional

from flipper.app import App
from flipper.storage import FlipperStorage
from flipper.utils.cdc import resolve_port


class Main(App):
    # this is basic use without sub-commands, simply to reboot flipper / power it off, not meant as a full CLI wrapper
    def init(self):
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")

        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_power_off = self.subparsers.add_parser(
            "power_off", help="Power off command, won't return to CLI"
        )
        self.parser_power_off.set_defaults(func=self.power_off)

        self.parser_reboot = self.subparsers.add_parser(
            "reboot", help="Reboot command help"
        )
        self.parser_reboot.set_defaults(func=self.reboot)

        self.parser_reboot2dfu = self.subparsers.add_parser(
            "reboot2dfu", help="Reboot to DFU, won't return to CLI"
        )
        self.parser_reboot2dfu.set_defaults(func=self.reboot2dfu)

    def _get_flipper(self, retry_count: Optional[int] = 1):
        port = None
        self.logger.info(f"Attempting to find flipper with {retry_count} attempts.")

        for i in range(retry_count):
            time.sleep(1)
            self.logger.info(f"Attempting to find flipper #{i}.")

            if port := resolve_port(self.logger, self.args.port):
                self.logger.info(f"Found flipper at {port}")
                break

        if not port:
            self.logger.info(f"Failed to find flipper")
            return None

        flipper = FlipperStorage(port)
        flipper.start()
        return flipper

    def power_off(self):
        if not (flipper := self._get_flipper(retry_count=10)):
            return 1

        self.logger.info("Powering off")
        flipper.send("power off" + "\r")
        flipper.stop()
        return 0

    def reboot(self):
        if not (flipper := self._get_flipper(retry_count=10)):
            return 1

        self.logger.info("Rebooting")
        flipper.send("power reboot" + "\r")
        flipper.stop()
        return 0

    def reboot2dfu(self):
        if not (flipper := self._get_flipper(retry_count=10)):
            return 1

        self.logger.info("Rebooting to DFU")
        flipper.send("power reboot2dfu" + "\r")
        flipper.stop()

        return 0


if __name__ == "__main__":
    Main()()
