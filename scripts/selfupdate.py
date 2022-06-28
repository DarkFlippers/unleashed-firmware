#!/usr/bin/env python3

from flipper.storage import FlipperStorage

import logging
import argparse
import os
import sys
import pathlib
import serial.tools.list_ports as list_ports


class Main:
    def __init__(self):
        # command args
        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("-d", "--debug", action="store_true", help="Debug")
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")
        self.parser.add_argument(
            "-b",
            "--baud",
            help="Port Baud rate",
            required=False,
            default=115200 * 4,
            type=int,
        )

        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_install = self.subparsers.add_parser(
            "install", help="Install OTA package"
        )
        self.parser_install.add_argument("manifest_path", help="Manifest path")
        self.parser_install.add_argument(
            "--pkg_dir_name", help="Update dir name", default="pcbundle", required=False
        )
        self.parser_install.set_defaults(func=self.install)

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

    # make directory with exist check
    def mkdir_on_storage(self, storage, flipper_dir_path):
        if not storage.exist_dir(flipper_dir_path):
            self.logger.debug(f'"{flipper_dir_path}" does not exist, creating')
            if not storage.mkdir(flipper_dir_path):
                self.logger.error(f"Error: {storage.last_error}")
                return False
        else:
            self.logger.debug(f'"{flipper_dir_path}" already exists')
        return True

    # send file with exist check and hash check
    def send_file_to_storage(self, storage, flipper_file_path, local_file_path, force):
        exists = storage.exist_file(flipper_file_path)
        do_upload = not exists
        if exists:
            hash_local = storage.hash_local(local_file_path)
            hash_flipper = storage.hash_flipper(flipper_file_path)
            self.logger.debug(f"hash check: local {hash_local}, flipper {hash_flipper}")
            do_upload = force or (hash_local != hash_flipper)

        if do_upload:
            self.logger.info(f'Sending "{local_file_path}" to "{flipper_file_path}"')
            if not storage.send_file(local_file_path, flipper_file_path):
                self.logger.error(f"Error: {storage.last_error}")
                return False
        return True

    def _get_port(self):
        if self.args.port != "auto":
            return self.args.port
        # Try guessing
        flippers = list(list_ports.grep("flip"))
        if len(flippers) == 1:
            flipper = flippers[0]
            self.logger.info(f"Using {flipper.serial_number} on {flipper.device}")
            return flipper.device
        elif len(flippers) == 0:
            self.logger.error("Failed to find connected Flipper")
        elif len(flippers) > 1:
            self.logger.error("More than one Flipper is attached")
        self.logger.error("Failed to guess which port to use. Specify --port")

    def install(self):
        if not (port := self._get_port()):
            return 1

        storage = FlipperStorage(port, self.args.baud)
        storage.start()

        if not os.path.isfile(self.args.manifest_path):
            self.logger.error("Error: manifest not found")
            return 2

        manifest_path = pathlib.Path(os.path.abspath(self.args.manifest_path))
        manifest_name, pkg_name = manifest_path.parts[-1], manifest_path.parts[-2]

        pkg_dir_name = self.args.pkg_dir_name or pkg_name
        flipper_update_path = f"/ext/update/{pkg_dir_name}"

        self.logger.info(f'Installing "{pkg_name}" from {flipper_update_path}')
        # if not os.path.exists(self.args.manifest_path):
        # self.logger.error("Error: package not found")
        if not self.mkdir_on_storage(storage, flipper_update_path):
            self.logger.error(f"Error: cannot create {storage.last_error}")
            return -2

        for dirpath, dirnames, filenames in os.walk(manifest_path.parents[0]):
            for fname in filenames:
                self.logger.debug(f"Uploading {fname}")
                local_file_path = os.path.join(dirpath, fname)
                flipper_file_path = f"{flipper_update_path}/{fname}"
                if not self.send_file_to_storage(
                    storage, flipper_file_path, local_file_path, False
                ):
                    self.logger.error(f"Error: {storage.last_error}")
                    return -3

            storage.send_and_wait_eol(
                f"update install {flipper_update_path}/{manifest_name}\r"
            )
            break
        storage.stop()


if __name__ == "__main__":
    Main()()
