import logging
import subprocess
from flipper.utils.cdc import resolve_port


def main():
    logger = logging.getLogger()
    if not (port := resolve_port(logger, "auto")):
        return 1
    subprocess.call(["python3", "-m", "serial.tools.miniterm", "--raw", port, "230400"])


if __name__ == "__main__":
    main()
