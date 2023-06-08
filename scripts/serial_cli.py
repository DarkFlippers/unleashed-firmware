import argparse
import logging
import os
import subprocess
import sys

from flipper.utils.cdc import resolve_port


def main():
    logger = logging.getLogger()
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--port", help="CDC Port", default="auto")
    args = parser.parse_args()
    if not (port := resolve_port(logger, args.port)):
        logger.error("Is Flipper connected via USB and not in DFU mode?")
        return 1
    subprocess.call(
        [
            os.path.basename(sys.executable),
            "-m",
            "serial.tools.miniterm",
            "--raw",
            port,
            "230400",
        ]
    )


if __name__ == "__main__":
    main()
