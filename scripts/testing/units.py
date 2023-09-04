#!/usr/bin/env python3
import logging
import re
import sys

import serial
from await_flipper import flp_serial_by_name


def main():
    logging.basicConfig(
        format="%(asctime)s %(levelname)-8s %(message)s",
        level=logging.INFO,
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    logging.info("Trying to run units on flipper")
    flp_serial = flp_serial_by_name(sys.argv[1])

    if flp_serial == "":
        logging.error("Flipper not found!")
        sys.exit(1)

    with serial.Serial(flp_serial, timeout=150) as flipper:
        logging.info(f"Found Flipper at {flp_serial}")
        flipper.baudrate = 230400
        flipper.flushOutput()
        flipper.flushInput()

        flipper.read_until(b">: ").decode("utf-8")
        flipper.write(b"unit_tests\r")
        data = flipper.read_until(b">: ").decode("utf-8")

        lines = data.split("\r\n")

        tests_re = r"Failed tests: \d{0,}"
        time_re = r"Consumed: \d{0,}"
        leak_re = r"Leaked: \d{0,}"
        status_re = r"Status: \w{3,}"

        tests_pattern = re.compile(tests_re)
        time_pattern = re.compile(time_re)
        leak_pattern = re.compile(leak_re)
        status_pattern = re.compile(status_re)

        tests, time, leak, status = None, None, None, None
        total = 0

        for line in lines:
            logging.info(line)
            if "()" in line:
                total += 1

            if not tests:
                tests = re.match(tests_pattern, line)
            if not time:
                time = re.match(time_pattern, line)
            if not leak:
                leak = re.match(leak_pattern, line)
            if not status:
                status = re.match(status_pattern, line)

        if None in (tests, time, leak, status):
            logging.error(f"Failed to parse output: {leak} {time} {leak} {status}")
            sys.exit(1)

        leak = int(re.findall(r"[- ]\d+", leak.group(0))[0])
        status = re.findall(r"\w+", status.group(0))[1]
        tests = int(re.findall(r"\d+", tests.group(0))[0])
        time = int(re.findall(r"\d+", time.group(0))[0])

        if tests > 0 or status != "PASSED":
            logging.error(f"Got {tests} failed tests.")
            logging.error(f"Leaked (not failing on this stat): {leak}")
            logging.error(f"Status: {status}")
            logging.error(f"Time: {time/1000} seconds")
            sys.exit(1)

        logging.info(f"Leaked (not failing on this stat): {leak}")
        logging.info(
            f"Tests ran successfully! Time elapsed {time/1000} seconds. Passed {total} tests."
        )

        sys.exit(0)


if __name__ == "__main__":
    main()
