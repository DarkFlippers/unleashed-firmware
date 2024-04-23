#!/usr/bin/env python3

import json
import logging
import os
import subprocess
import tarfile
import tempfile

import requests
import serial.tools.list_ports as list_ports
from flipper.app import App
from serial.tools.list_ports_common import ListPortInfo


class UpdateDownloader:
    UPDATE_SERVER = "https://update.flipperzero.one"
    UPDATE_PROJECT = "/blackmagic-firmware"
    UPDATE_INDEX = UPDATE_SERVER + UPDATE_PROJECT + "/directory.json"
    UPDATE_TYPE = "full_tgz"

    CHANNEL_ID_ALIAS = {
        "dev": "development",
        "rc": "release-candidate",
        "r": "release",
        "rel": "release",
    }

    def __init__(self):
        self.logger = logging.getLogger()

    def download(self, channel_id: str, target_dir: str) -> bool:
        # Aliases
        if channel_id in self.CHANNEL_ID_ALIAS:
            channel_id = self.CHANNEL_ID_ALIAS[channel_id]

        # Make directory
        if not os.path.exists(target_dir):
            self.logger.info(f"Creating directory {target_dir}")
            os.makedirs(target_dir)

        # Download json index
        self.logger.info(f"Downloading {self.UPDATE_INDEX}")
        response = requests.get(self.UPDATE_INDEX)
        if response.status_code != 200:
            self.logger.error(f"Failed to download {self.UPDATE_INDEX}")
            return False

        # Parse json index
        try:
            index = json.loads(response.content)
        except Exception as e:
            self.logger.error(f"Failed to parse json index: {e}")
            return False

        # Find channel
        channel = None
        for channel_candidate in index["channels"]:
            if channel_candidate["id"] == channel_id:
                channel = channel_candidate
                break

        # Check if channel found
        if channel is None:
            self.logger.error(
                f"Channel '{channel_id}' not found. Valid channels: {', '.join([c['id'] for c in index['channels']])}"
            )
            return False

        self.logger.info(f"Using channel '{channel_id}'")

        # Get latest version
        try:
            version = channel["versions"][0]
        except Exception as e:
            self.logger.error(f"Failed to get version: {e}")
            return False

        self.logger.info(f"Using version '{version['version']}'")

        # Get changelog
        if changelog := version.get("changelog"):
            self.logger.info(f"Changelog:")
            for line in changelog.split("\n"):
                if line.strip() == "":
                    continue
                self.logger.info(f"  {line}")
        else:
            self.logger.warning(f"Changelog not found")

        # Find file
        file_url = None
        for file_candidate in version["files"]:
            if file_candidate["type"] == self.UPDATE_TYPE:
                file_url = file_candidate["url"]
                break

        if file_url is None:
            self.logger.error(f"File not found")
            return False

        # Make file path
        file_name = file_url.split("/")[-1]
        file_path = os.path.join(target_dir, file_name)

        # Download file
        self.logger.info(f"Downloading {file_url} to {file_path}")
        with open(file_path, "wb") as f:
            response = requests.get(file_url)
            f.write(response.content)

        # Unzip tgz
        self.logger.info(f"Unzipping {file_path}")
        with tarfile.open(file_path, "r") as tar:
            tar.extractall(target_dir)

        return True


class Main(App):
    def init(self):
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")
        self.parser.add_argument(
            "-c", "--channel", help="Channel name", default="release"
        )
        self.parser.set_defaults(func=self.update)

        # logging
        self.logger = logging.getLogger()

    @staticmethod
    def _grep_ports(regexp: str) -> list[ListPortInfo]:
        # idk why, but python thinks that list_ports.grep returns tuple[str, str, str]
        return list(list_ports.grep(regexp))  # type: ignore

    def is_wifi_board_connected(self) -> bool:
        return (
            len(self._grep_ports("ESP32-S2")) > 0
            or len(self._grep_ports("CMSIS-DAP")) > 0
        )

    @staticmethod
    def is_windows() -> bool:
        return os.name == "nt"

    @classmethod
    def find_port(cls, regexp: str) -> str:
        ports: list[ListPortInfo] = cls._grep_ports(regexp)

        if len(ports) == 0:
            # Blackmagic probe serial port not found, will be handled later
            pass
        elif len(ports) > 1:
            raise Exception("More than one WiFi board found")
        else:
            port = ports[0]
            return f"\\\\.\\{port.device}" if cls.is_windows() else port.device

    def find_wifi_board_bootloader_port(self):
        return self.find_port("ESP32-S2")

    def find_wifi_board_bootloader_port_damn_windows(self):
        self.logger.info("Trying to find WiFi board using VID:PID")
        return self.find_port("VID:PID=303A:0002")

    def update(self):
        try:
            port = self.find_wifi_board_bootloader_port()

            # Damn windows fix
            if port is None and self.is_windows():
                port = self.find_wifi_board_bootloader_port_damn_windows()
        except Exception as e:
            self.logger.error(f"{e}")
            return 1

        if port is None:
            if self.is_wifi_board_connected():
                self.logger.error("WiFi board found, but not in bootloader mode.")
                self.logger.info("Please hold down BOOT button and press RESET button")
            else:
                self.logger.error("WiFi board not found")
                self.logger.info(
                    "Please connect WiFi board to your computer, hold down BOOT button and press RESET button"
                )
                if not self.is_windows():
                    self.logger.info(
                        "If you are using Linux, you may need to add udev rules to access the device"
                    )
                    self.logger.info(
                        "Check out 41-flipper.rules & README in scripts/debug folder"
                    )
            return 1

        # get temporary dir
        with tempfile.TemporaryDirectory() as temp_dir:
            downloader = UpdateDownloader()

            # download latest channel update
            try:
                if not downloader.download(self.args.channel, temp_dir):
                    self.logger.error(f"Cannot download update")
                    return 1
            except Exception as e:
                self.logger.error(f"Cannot download update: {e}")
                return 1

            with open(os.path.join(temp_dir, "flash.command"), "r") as f:
                flash_command = f.read()

            replacements = (
                ("\n", ""),
                ("\r", ""),
                ("(PORT)", port),
                # We can't reset the board after flashing via usb
                ("--after hard_reset", "--after no_reset_stub"),
            )

            # hellish toolchain fix
            if self.is_windows():
                replacements += (("esptool.py", "python -m esptool"),)
            else:
                replacements += (("esptool.py", "python3 -m esptool"),)

            for old, new in replacements:
                flash_command = flash_command.replace(old, new)

            args = list(filter(None, flash_command.split()))

            self.logger.info(f'Running command: "{" ".join(args)}" in "{temp_dir}"')

            process = subprocess.Popen(
                args,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                cwd=temp_dir,
                bufsize=1,
                universal_newlines=True,
            )

            while process.poll() is None:
                if process.stdout is not None:
                    for line in process.stdout:
                        self.logger.debug(f"{line.strip()}")

        if process.returncode != 0:
            self.logger.error(f"Failed to flash WiFi board")
        else:
            self.logger.info("WiFi board flashed successfully")
            self.logger.info("Press RESET button on WiFi board to start it")

        return process.returncode


if __name__ == "__main__":
    Main()()
