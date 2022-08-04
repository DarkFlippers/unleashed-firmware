#!/usr/bin/env python3

from typing import final
from flipper.app import App
from flipper.storage import FlipperStorage
from flipper.utils.cdc import resolve_port

import logging
import os
import pathlib
import serial.tools.list_ports as list_ports


class Main(App):
    def init(self):
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")

        self.parser.add_argument("manifest_path", help="Manifest path")
        self.parser.add_argument(
            "--pkg_dir_name", help="Update dir name", default="pcbundle", required=False
        )
        self.parser.set_defaults(func=self.install)

        # logging
        self.logger = logging.getLogger()

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

    def install(self):
        if not (port := resolve_port(self.logger, self.args.port)):
            return 1

        storage = FlipperStorage(port)
        storage.start()

        try:
            if not os.path.isfile(self.args.manifest_path):
                self.logger.error("Error: manifest not found")
                return 2

            manifest_path = pathlib.Path(os.path.abspath(self.args.manifest_path))
            manifest_name, pkg_name = manifest_path.parts[-1], manifest_path.parts[-2]

            pkg_dir_name = self.args.pkg_dir_name or pkg_name
            update_root = "/ext/update"
            flipper_update_path = f"{update_root}/{pkg_dir_name}"

            self.logger.info(f'Installing "{pkg_name}" from {flipper_update_path}')
            # if not os.path.exists(self.args.manifest_path):
            # self.logger.error("Error: package not found")
            if not self.mkdir_on_storage(
                storage, update_root
            ) or not self.mkdir_on_storage(storage, flipper_update_path):
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

                # return -11
                storage.send_and_wait_eol(
                    f"update install {flipper_update_path}/{manifest_name}\r"
                )
                result = storage.read.until(storage.CLI_EOL)
                if not b"Verifying" in result:
                    self.logger.error(f"Unexpected response: {result.decode('ascii')}")
                    return -4
                result = storage.read.until(storage.CLI_EOL)
                if not result.startswith(b"OK"):
                    self.logger.error(result.decode("ascii"))
                    return -5
                break
            return 0
        finally:
            storage.stop()


if __name__ == "__main__":
    Main()()
