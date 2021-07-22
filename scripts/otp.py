#!/usr/bin/env python3

import logging
import argparse
import subprocess
import os
import sys
import re
import struct
import datetime

OTP_MAGIC = 0xBABE
OTP_VERSION = 0x01
OTP_RESERVED = 0x00

OTP_COLORS = {
    "unknown": 0x00,
    "black": 0x01,
    "white": 0x02,
}
OTP_REGIONS = {
    "unknown": 0x00,
    "europe": 0x01,
    "usa": 0x02,
}

BOARD_RESERVED = 0x0000


class Main:
    def __init__(self):
        # command args
        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("-d", "--debug", action="store_true", help="Debug")
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        # Generate
        self.parser_generate = self.subparsers.add_parser(
            "generate", help="Generate OTP binary"
        )
        self._add_args(self.parser_generate)
        self.parser_generate.add_argument("file", help="Output file")
        self.parser_generate.set_defaults(func=self.generate)
        # Flash
        self.parser_flash = self.subparsers.add_parser(
            "flash", help="Flash OTP to device"
        )
        self._add_args(self.parser_flash)
        self.parser_flash.add_argument(
            "--port", type=str, help="Port to connect: swd or usb1", default="swd"
        )
        self.parser_flash.set_defaults(func=self.flash)
        # logging
        self.logger = logging.getLogger()
        self.timestamp = datetime.datetime.now().timestamp()

    def __call__(self):
        self.args = self.parser.parse_args()
        if "func" not in self.args:
            self.parser.error("Choose something to do")
        # configure log output
        self.log_level = logging.DEBUG if self.args.debug else logging.INFO
        self.logger.setLevel(self.log_level)
        self.handler = logging.StreamHandler(sys.stdout)
        self.handler.setLevel(self.log_level)
        self.formatter = logging.Formatter("%(asctime)s [%(levelname)s] %(message)s")
        self.handler.setFormatter(self.formatter)
        self.logger.addHandler(self.handler)
        # execute requested function
        self.args.func()

    def _add_args(self, parser):
        parser.add_argument("--version", type=int, help="Version", default=10)
        parser.add_argument("--firmware", type=int, help="Firmware", default=6)
        parser.add_argument("--body", type=int, help="Body", default=8)
        parser.add_argument("--connect", type=int, help="Connect", default=5)
        parser.add_argument("--color", type=str, help="Color", default="unknown")
        parser.add_argument("--region", type=str, help="Region", default="unknown")
        parser.add_argument("--name", type=str, help="Name", required=True)

    def _process_args(self):
        if self.args.color not in OTP_COLORS:
            self.parser.error(f"Invalid color. Use one of {OTP_COLORS.keys()}")
        self.args.color = OTP_COLORS[self.args.color]

        if self.args.region not in OTP_REGIONS:
            self.parser.error(f"Invalid region. Use one of {OTP_REGIONS.keys()}")
        self.args.region = OTP_REGIONS[self.args.region]

        if len(self.args.name) > 8:
            self.parser.error("Name is too long. Max 8 symbols.")
        if re.match(r"[a-zA-Z0-9]+", self.args.name) is None:
            self.parser.error(
                "Name contains incorrect symbols. Only a-zA-Z0-9 allowed."
            )

    def _pack_struct(self):
        return struct.pack(
            "<" "HBBL" "BBBBBBH" "8s",
            OTP_MAGIC,
            OTP_VERSION,
            OTP_RESERVED,
            int(self.timestamp),
            self.args.version,
            self.args.firmware,
            self.args.body,
            self.args.connect,
            self.args.color,
            self.args.region,
            BOARD_RESERVED,
            self.args.name.encode("ascii"),
        )

    def generate(self):
        self.logger.debug(f"Generating OTP")
        self._process_args()
        data = self._pack_struct()
        open(self.args.file, "wb").write(data)

    def flash(self):
        self.logger.debug(f"Flashing OTP")
        self._process_args()
        data = self._pack_struct()
        filename = f"otp_{self.args.name}_{self.timestamp}.bin"
        open(filename, "wb").write(data)

        try:
            output = subprocess.check_output(
                [
                    "STM32_Programmer_CLI",
                    "-q",
                    "-c",
                    f"port={self.args.port}",
                    "-d",
                    filename,
                    "0x1FFF7000",
                ]
            )
            assert output
            self.logger.info(f"Success")
        except subprocess.CalledProcessError as e:
            self.logger.error(e.output.decode())
            self.logger.error(f"Failed to call STM32_Programmer_CLI")
            return
        except Exception as e:
            self.logger.error(f"Failed to call STM32_Programmer_CLI")
            self.logger.exception(e)
            return
        # reboot
        subprocess.check_output(
            [
                "STM32_Programmer_CLI",
                "-q",
                "-c",
                f"port={self.args.port}",
            ]
        )
        os.remove(filename)


if __name__ == "__main__":
    Main()()
