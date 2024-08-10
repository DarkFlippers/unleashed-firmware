#!/usr/bin/env python3

import re
import sys
import time
from typing import Optional

from flipper.app import App
from flipper.storage import FlipperStorage
from flipper.utils.cdc import resolve_port


class Main(App):
    # this is basic use without sub-commands, simply to reboot flipper / power it off, not meant as a full CLI wrapper
    def init(self):
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")
        self.parser.add_argument(
            "-t", "--timeout", help="Timeout in seconds", type=int, default=10
        )

        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_await_flipper = self.subparsers.add_parser(
            "await_flipper", help="Wait for Flipper to connect or reconnect"
        )
        self.parser_await_flipper.set_defaults(func=self.await_flipper)

        self.parser_run_units = self.subparsers.add_parser(
            "run_units", help="Run unit tests and post result"
        )
        self.parser_run_units.set_defaults(func=self.run_units)

    def _get_flipper(self, retry_count: Optional[int] = 1):
        port = None
        self.logger.info(f"Attempting to find flipper with {retry_count} attempts.")

        for i in range(retry_count):
            self.logger.info(f"Attempt to find flipper #{i}.")

            if port := resolve_port(self.logger, self.args.port):
                self.logger.info(f"Found flipper at {port}")
                time.sleep(1)
                break

            time.sleep(1)

        if not port:
            self.logger.info(f"Failed to find flipper {port}")
            return None

        flipper = FlipperStorage(port)
        flipper.start()
        return flipper

    def await_flipper(self):
        if not (flipper := self._get_flipper(retry_count=self.args.timeout)):
            return 1

        self.logger.info("Flipper started")
        flipper.stop()
        return 0

    def run_units(self):
        if not (flipper := self._get_flipper(retry_count=10)):
            return 1

        self.logger.info("Running unit tests")
        flipper.send("unit_tests" + "\r")
        self.logger.info("Waiting for unit tests to complete")
        data = flipper.read.until(">: ")
        self.logger.info("Parsing result")

        lines = data.decode().split("\r\n")

        tests_re = r"Failed tests: \d{0,}"
        time_re = r"Consumed: \d{0,}"
        leak_re = r"Leaked: \d{0,}"
        status_re = r"Status: \w{3,}"

        tests_pattern = re.compile(tests_re)
        time_pattern = re.compile(time_re)
        leak_pattern = re.compile(leak_re)
        status_pattern = re.compile(status_re)

        tests, elapsed_time, leak, status = None, None, None, None
        total = 0

        for line in lines:
            self.logger.info(line)
            if "()" in line:
                total += 1

            if not tests:
                tests = re.match(tests_pattern, line)
            if not elapsed_time:
                elapsed_time = re.match(time_pattern, line)
            if not leak:
                leak = re.match(leak_pattern, line)
            if not status:
                status = re.match(status_pattern, line)

        if None in (tests, elapsed_time, leak, status):
            self.logger.error(
                f"Failed to parse output: {tests} {elapsed_time} {leak} {status}"
            )
            sys.exit(1)

        leak = int(re.findall(r"[- ]\d+", leak.group(0))[0])
        status = re.findall(r"\w+", status.group(0))[1]
        tests = int(re.findall(r"\d+", tests.group(0))[0])
        elapsed_time = int(re.findall(r"\d+", elapsed_time.group(0))[0])

        if tests > 0 or status != "PASSED":
            self.logger.error(f"Got {tests} failed tests.")
            self.logger.error(f"Leaked (not failing on this stat): {leak}")
            self.logger.error(f"Status: {status}")
            self.logger.error(f"Time: {elapsed_time/1000} seconds")
            flipper.stop()
            return 1

        self.logger.info(f"Leaked (not failing on this stat): {leak}")
        self.logger.info(
            f"Tests ran successfully! Time elapsed {elapsed_time/1000} seconds. Passed {total} tests."
        )

        flipper.stop()
        return 0


if __name__ == "__main__":
    Main()()
