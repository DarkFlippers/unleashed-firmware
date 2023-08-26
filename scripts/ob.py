#!/usr/bin/env python3

from os import path

from flipper.app import App
from flipper.utils.programmer_openocd import OpenOCDProgrammer


class Main(App):
    def init(self):
        # Subparsers
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # Check command
        self.parser_check = self.subparsers.add_parser(
            "check", help="Check Option Bytes"
        )
        self._add_args(self.parser_check)
        self.parser_check.set_defaults(func=self.check)

        # Set command
        self.parser_set = self.subparsers.add_parser("set", help="Set Option Bytes")
        self._add_args(self.parser_set)
        self.parser_set.set_defaults(func=self.set)
        # Set command
        self.parser_recover = self.subparsers.add_parser(
            "recover", help="Recover Option Bytes"
        )
        self._add_args(self.parser_recover)
        self.parser_recover.set_defaults(func=self.recover)

    def _add_args(self, parser):
        parser.add_argument(
            "--port-base", type=int, help="OpenOCD port base", default=3333
        )
        parser.add_argument(
            "--interface",
            type=str,
            help="OpenOCD interface",
            default="interface/cmsis-dap.cfg",
        )
        parser.add_argument(
            "--serial", type=str, help="OpenOCD interface serial number"
        )
        parser.add_argument(
            "--ob-path",
            type=str,
            help="Option bytes file",
            default=path.join(path.dirname(__file__), "ob.data"),
        )

    def check(self):
        self.logger.info("Checking Option Bytes")

        # OpenOCD
        openocd = OpenOCDProgrammer(
            self.args.interface,
            self.args.port_base,
            self.args.serial,
        )

        return_code = 1
        if openocd.option_bytes_validate(self.args.ob_path):
            return_code = 0

        return return_code

    def set(self):
        self.logger.info("Setting Option Bytes")

        # OpenOCD
        openocd = OpenOCDProgrammer(
            self.args.interface,
            self.args.port_base,
            self.args.serial,
        )

        return_code = 1
        if openocd.option_bytes_set(self.args.ob_path):
            return_code = 0

        return return_code

    def recover(self):
        self.logger.info("Setting Option Bytes")

        # OpenOCD
        openocd = OpenOCDProgrammer(
            self.args.interface,
            self.args.port_base,
            self.args.serial,
        )

        openocd.option_bytes_recover()

        return 0


if __name__ == "__main__":
    Main()()
