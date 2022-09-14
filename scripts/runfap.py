#!/usr/bin/env python3

import posixpath
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

        self.parser.add_argument("fap_src_path", help="App file to upload")
        self.parser.add_argument(
            "--fap_dst_dir", help="Upload path", default="/ext/apps", required=False
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
            fap_local_path = self.args.fap_src_path
            self.args.fap_dst_dir = self.args.fap_dst_dir.rstrip("/\\")

            if not os.path.isfile(fap_local_path):
                self.logger.error(f"Error: source .fap ({fap_local_path}) not found")
                return -1

            fap_dst_path = posixpath.join(
                self.args.fap_dst_dir, os.path.basename(fap_local_path)
            )

            self.logger.info(f'Installing "{fap_local_path}" to {fap_dst_path}')

            if not self.mkdir_on_storage(storage, self.args.fap_dst_dir):
                self.logger.error(f"Error: cannot create dir: {storage.last_error}")
                return -2

            if not self.send_file_to_storage(
                storage, fap_dst_path, fap_local_path, False
            ):
                self.logger.error(f"Error: upload failed: {storage.last_error}")
                return -3

            storage.send_and_wait_eol(f'loader open "Applications" {fap_dst_path}\r')
            result = storage.read.until(storage.CLI_EOL)
            if len(result):
                self.logger.error(f"Unexpected response: {result.decode('ascii')}")
                return -4

            return 0
        finally:
            storage.stop()


if __name__ == "__main__":
    Main()()
