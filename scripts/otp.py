#!/usr/bin/env python3

import logging
import argparse
import os
import sys
import re
import struct
import datetime


class Main:
    def __init__(self):
        # command args
        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("-d", "--debug", action="store_true", help="Debug")
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        self.parser_generate = self.subparsers.add_parser(
            "generate", help="OTP HW version generator"
        )
        self.parser_generate.add_argument(
            "--version", type=int, help="Version", required=True
        )
        self.parser_generate.add_argument(
            "--firmware", type=int, help="Firmware", required=True
        )
        self.parser_generate.add_argument(
            "--body", type=int, help="Body", required=True
        )
        self.parser_generate.add_argument(
            "--connect", type=int, help="Connect", required=True
        )
        self.parser_generate.add_argument(
            "--name", type=str, help="Name", required=True
        )
        self.parser_generate.add_argument("file", help="Output file")
        self.parser_generate.set_defaults(func=self.generate)
        # logging
        self.logger = logging.getLogger()

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

    def generate(self):
        self.logger.debug(f"Generating OTP")

        if self.args.name:
            name = re.sub(
                "[^a-zA-Z0-9.]", "", self.args.name
            )  # Filter all special characters
            name = list(
                map(str, map(ord, name[0:8]))
            )  # Strip to 8 chars and map to ascii codes
            while len(name) < 8:
                name.append("0")

            n1, n2, n3, n4, n5, n6, n7, n8 = map(int, name)

        data = struct.pack(
            "<BBBBLBBBBBBBB",
            self.args.version,
            self.args.firmware,
            self.args.body,
            self.args.connect,
            int(datetime.datetime.now().timestamp()),
            n1,
            n2,
            n3,
            n4,
            n5,
            n6,
            n7,
            n8,
        )
        open(self.args.file, "wb").write(data)


if __name__ == "__main__":
    Main()()
