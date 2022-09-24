import logging
import subprocess
from flipper.utils.cdc import resolve_port
import os
import sys


def main():
    logger = logging.getLogger()
    if not (port := resolve_port(logger, "auto")):
        logger.error("Is Flipper connected over USB and isn't in DFU mode?")
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
