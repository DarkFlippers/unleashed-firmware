#!/usr/bin/env python3

import logging
import os
import pathlib

from flipper.app import App
from flipper.storage import FlipperStorage, FlipperStorageOperations
from flipper.utils.cdc import resolve_port


class Main(App):
    def init(self):
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")

        self.parser.add_argument("manifest_path", help="Manifest path")
        self.parser.add_argument(
            "--pkg_dir_name", help="Update dir name", default=None, required=False
        )
        self.parser.set_defaults(func=self.install)

        # logging
        self.logger = logging.getLogger()

    def install(self):
        if not (port := resolve_port(self.logger, self.args.port)):
            return 1

        if not os.path.isfile(self.args.manifest_path):
            self.logger.error("Error: manifest not found")
            return 2

        manifest_path = pathlib.Path(os.path.abspath(self.args.manifest_path))
        manifest_name, pkg_name = manifest_path.parts[-1], manifest_path.parts[-2]

        pkg_dir_name = self.args.pkg_dir_name or pkg_name
        update_root = "/ext/update"
        flipper_update_path = f"{update_root}/{pkg_dir_name}"

        self.logger.info(f'Installing "{pkg_name}" from {flipper_update_path}')

        try:
            with FlipperStorage(port) as storage:
                storage_ops = FlipperStorageOperations(storage)
                storage_ops.mkpath(update_root)
                storage_ops.mkpath(flipper_update_path)
                storage_ops.recursive_send(
                    flipper_update_path, manifest_path.parents[0]
                )

                storage.send_and_wait_eol(
                    f"update install {flipper_update_path}/{manifest_name}\r"
                )
                result = storage.read.until(storage.CLI_EOL)
                if b"Verifying" not in result:
                    self.logger.error(f"Unexpected response: {result.decode('ascii')}")
                    return 3
                result = storage.read.until(storage.CLI_EOL)
                if not result.startswith(b"OK"):
                    self.logger.error(result.decode("ascii"))
                    return 4
                return 0
        except Exception as e:
            self.logger.error(e)
            return 5


if __name__ == "__main__":
    Main()()
