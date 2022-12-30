#!/usr/bin/env python3
import logging
import os
import sys
import time


def flp_serial_by_name(flp_name):
    if sys.platform == "darwin":  # MacOS
        flp_serial = "/dev/cu.usbmodemflip_" + flp_name + "1"
    elif sys.platform == "linux":  # Linux
        flp_serial = (
            "/dev/serial/by-id/usb-Flipper_Devices_Inc._Flipper_"
            + flp_name
            + "_flip_"
            + flp_name
            + "-if00"
        )

    if os.path.exists(flp_serial):
        return flp_serial
    else:
        if os.path.exists(flp_name):
            return flp_name
        else:
            return ""


UPDATE_TIMEOUT = 60 * 4  # 4 minutes


def main():
    flipper_name = sys.argv[1]
    elapsed = 0
    flipper = flp_serial_by_name(flipper_name)
    logging.basicConfig(
        format="%(asctime)s %(levelname)-8s %(message)s",
        level=logging.INFO,
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    logging.info("Waiting for Flipper to be ready...")

    while flipper == "" and elapsed < UPDATE_TIMEOUT:
        elapsed += 1
        time.sleep(1)
        flipper = flp_serial_by_name(flipper_name)

    if flipper == "":
        logging.error("Flipper not found!")
        sys.exit(1)

    logging.info(f"Found Flipper at {flipper}")

    sys.exit(0)


if __name__ == "__main__":
    main()
