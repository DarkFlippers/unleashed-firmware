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
OTP_VERSION = 0x02
OTP_RESERVED = 0x00

OTP_COLORS = {
    "unknown": 0x00,
    "black": 0x01,
    "white": 0x02,
}

OTP_REGIONS = {
    "unknown": 0x00,
    "eu_ru": 0x01,
    "us_ca_au": 0x02,
    "jp": 0x03,
}

OTP_DISPLAYS = {
    "unknown": 0x00,
    "erc": 0x01,
    "mgg": 0x02,
}

from flipper.app import App
from flipper.cube import CubeProgrammer


class Main(App):
    def init(self):
        # SubParsers
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        # Generate All
        self.parser_generate_all = self.subparsers.add_parser(
            "generate", help="Generate OTP binary"
        )
        self._add_first_args(self.parser_generate_all)
        self._add_second_args(self.parser_generate_all)
        self.parser_generate_all.add_argument("file", help="Output file")
        self.parser_generate_all.set_defaults(func=self.generate_all)
        # Flash First
        self.parser_flash_first = self.subparsers.add_parser(
            "flash_first", help="Flash first block of OTP to device"
        )
        self._add_swd_args(self.parser_flash_first)
        self._add_first_args(self.parser_flash_first)
        self.parser_flash_first.set_defaults(func=self.flash_first)
        # Flash Second
        self.parser_flash_second = self.subparsers.add_parser(
            "flash_second", help="Flash second block of OTP to device"
        )
        self._add_swd_args(self.parser_flash_second)
        self._add_second_args(self.parser_flash_second)
        self.parser_flash_second.set_defaults(func=self.flash_second)
        # Flash All
        self.parser_flash_all = self.subparsers.add_parser(
            "flash_all", help="Flash OTP to device"
        )
        self._add_swd_args(self.parser_flash_all)
        self._add_first_args(self.parser_flash_all)
        self._add_second_args(self.parser_flash_all)
        self.parser_flash_all.set_defaults(func=self.flash_all)
        # logging
        self.logger = logging.getLogger()
        self.timestamp = datetime.datetime.now().timestamp()

    def _add_swd_args(self, parser):
        parser.add_argument(
            "--port", type=str, help="Port to connect: swd or usb1", default="swd"
        )

    def _add_first_args(self, parser):
        parser.add_argument("--version", type=int, help="Version", required=True)
        parser.add_argument("--firmware", type=int, help="Firmware", required=True)
        parser.add_argument("--body", type=int, help="Body", required=True)
        parser.add_argument("--connect", type=int, help="Connect", required=True)
        parser.add_argument("--display", type=str, help="Display", required=True)

    def _add_second_args(self, parser):
        parser.add_argument("--color", type=str, help="Color", required=True)
        parser.add_argument("--region", type=str, help="Region", required=True)
        parser.add_argument("--name", type=str, help="Name", required=True)

    def _process_first_args(self):
        if self.args.display not in OTP_DISPLAYS:
            self.parser.error(f"Invalid display. Use one of {OTP_DISPLAYS.keys()}")
        self.args.display = OTP_DISPLAYS[self.args.display]

    def _process_second_args(self):
        if self.args.color not in OTP_COLORS:
            self.parser.error(f"Invalid color. Use one of {OTP_COLORS.keys()}")
        self.args.color = OTP_COLORS[self.args.color]

        if self.args.region not in OTP_REGIONS:
            self.parser.error(f"Invalid region. Use one of {OTP_REGIONS.keys()}")
        self.args.region = OTP_REGIONS[self.args.region]

        if len(self.args.name) > 8:
            self.parser.error("Name is too long. Max 8 symbols.")
        if re.match(r"^[a-zA-Z0-9.]+$", self.args.name) is None:
            self.parser.error(
                "Name contains incorrect symbols. Only a-zA-Z0-9 allowed."
            )

    def _pack_first(self):
        return struct.pack(
            "<" "HBBL" "BBBBBBH",
            OTP_MAGIC,
            OTP_VERSION,
            OTP_RESERVED,
            int(self.timestamp),
            self.args.version,
            self.args.firmware,
            self.args.body,
            self.args.connect,
            self.args.display,
            OTP_RESERVED,
            OTP_RESERVED,
        )

    def _pack_second(self):
        return struct.pack(
            "<" "BBHL" "8s",
            self.args.color,
            self.args.region,
            OTP_RESERVED,
            OTP_RESERVED,
            self.args.name.encode("ascii"),
        )

    def generate_all(self):
        self.logger.info(f"Generating OTP")
        self._process_first_args()
        self._process_second_args()
        open(f"{self.args.file}_first.bin", "wb").write(self._pack_first())
        open(f"{self.args.file}_second.bin", "wb").write(self._pack_second())
        self.logger.info(
            f"Generated files: {self.args.file}_first.bin and {self.args.file}_second.bin"
        )

        return 0

    def flash_first(self):
        self.logger.info(f"Flashing first block of OTP")

        self._process_first_args()

        filename = f"otp_unknown_first_{self.timestamp}.bin"

        try:
            self.logger.info(f"Packing binary data")
            file = open(filename, "wb")
            file.write(self._pack_first())
            file.close()
            self.logger.info(f"Flashing OTP")
            cp = CubeProgrammer(self.args.port)
            cp.flashBin("0x1FFF7000", filename)
            cp.resetTarget()
            self.logger.info(f"Flashed Successfully")
            os.remove(filename)
        except Exception as e:
            self.logger.exception(e)
            return 1

        return 0

    def flash_second(self):
        self.logger.info(f"Flashing second block of OTP")

        self._process_second_args()

        filename = f"otp_{self.args.name}_second_{self.timestamp}.bin"

        try:
            self.logger.info(f"Packing binary data")
            file = open(filename, "wb")
            file.write(self._pack_second())
            file.close()
            self.logger.info(f"Flashing OTP")
            cp = CubeProgrammer(self.args.port)
            cp.flashBin("0x1FFF7010", filename)
            cp.resetTarget()
            self.logger.info(f"Flashed Successfully")
            os.remove(filename)
        except Exception as e:
            self.logger.exception(e)
            return 1

        return 0

    def flash_all(self):
        self.logger.info(f"Flashing OTP")

        self._process_first_args()
        self._process_second_args()

        filename = f"otp_{self.args.name}_whole_{self.timestamp}.bin"

        try:
            self.logger.info(f"Packing binary data")
            file = open(filename, "wb")
            file.write(self._pack_first())
            file.write(self._pack_second())
            file.close()
            self.logger.info(f"Flashing OTP")
            cp = CubeProgrammer(self.args.port)
            cp.flashBin("0x1FFF7000", filename)
            cp.resetTarget()
            self.logger.info(f"Flashed Successfully")
            os.remove(filename)
        except Exception as e:
            self.logger.exception(e)
            return 1

        return 0


if __name__ == "__main__":
    Main()()
