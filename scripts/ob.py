#!/usr/bin/env python3

import logging
import argparse
import subprocess
import sys
import os


class Main:
    def __init__(self):
        # command args
        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("-d", "--debug", action="store_true", help="Debug")
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        self.parser_check = self.subparsers.add_parser(
            "check", help="Check Option Bytes"
        )
        self.parser_check.add_argument(
            "--port", type=str, help="Port to connect: swd or usb1", default="swd"
        )
        self.parser_check.set_defaults(func=self.check)
        self.parser_set = self.subparsers.add_parser("set", help="Set Option Bytes")
        self.parser_set.add_argument(
            "--port", type=str, help="Port to connect: swd or usb1", default="swd"
        )
        self.parser_set.set_defaults(func=self.set)
        # logging
        self.logger = logging.getLogger()
        # OB
        self.ob = {}

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
        self.loadOB()
        self.args.func()

    def loadOB(self):
        self.logger.info(f"Loading Option Bytes data")
        file_path = os.path.join(os.path.dirname(sys.argv[0]), "ob.data")
        file = open(file_path, "r")
        for line in file.readlines():
            k, v, o = line.split(":")
            self.ob[k.strip()] = v.strip(), o.strip()

    def check(self):
        self.logger.info(f"Checking Option Bytes")
        try:
            output = subprocess.check_output(
                [
                    "STM32_Programmer_CLI",
                    "-q",
                    "-c",
                    f"port={self.args.port}",
                    "-ob displ",
                ]
            )
            assert output
        except subprocess.CalledProcessError as e:
            self.logger.error(e.output.decode())
            self.logger.error(f"Failed to call STM32_Programmer_CLI")
            return
        except Exception as e:
            self.logger.error(f"Failed to call STM32_Programmer_CLI")
            self.logger.exception(e)
            return
        ob_correct = True
        for line in output.decode().split("\n"):
            line = line.strip()
            if not ":" in line:
                self.logger.debug(f"Skipping line: {line}")
                continue
            key, data = line.split(":", 1)
            key = key.strip()
            data = data.strip()
            if not key in self.ob.keys():
                self.logger.debug(f"Skipping key: {key}")
                continue
            self.logger.debug(f"Processing key: {key} {data}")
            value, comment = data.split(" ", 1)
            value = value.strip()
            comment = comment.strip()
            if self.ob[key][0] != value:
                self.logger.error(
                    f"Invalid OB: {key} {value}, expected: {self.ob[key][0]}"
                )
                ob_correct = False
        if ob_correct:
            self.logger.info(f"OB Check OK")
        else:
            self.logger.error(f"OB Check FAIL")
            exit(255)

    def set(self):
        self.logger.info(f"Setting Option Bytes")
        options = []
        for key, (value, attr) in self.ob.items():
            if "w" in attr:
                options.append(f"{key}={value}")
        try:
            output = subprocess.check_output(
                [
                    "STM32_Programmer_CLI",
                    "-q",
                    "-c",
                    f"port={self.args.port}",
                    "-ob",
                    *options,
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


if __name__ == "__main__":
    Main()()
