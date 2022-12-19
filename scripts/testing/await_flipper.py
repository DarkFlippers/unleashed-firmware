#!/usr/bin/env python3

import sys, os, time


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


UPDATE_TIMEOUT = 60


def main():
    flipper_name = sys.argv[1]
    elapsed = 0
    flipper = flp_serial_by_name(flipper_name)

    while flipper == "" and elapsed < UPDATE_TIMEOUT:
        elapsed += 1
        time.sleep(1)
        flipper = flp_serial_by_name(flipper_name)

    if flipper == "":
        print(f"Cannot find {flipper_name} flipper. Guess your flipper swam away")
        sys.exit(1)

    sys.exit(0)


if __name__ == "__main__":
    main()
