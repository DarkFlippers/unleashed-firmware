#!/usr/bin/env python3

from flipper.app import App
from flipper.storage import FlipperStorage, FlipperStorageOperations
from flipper.utils.cdc import resolve_port

import os
import posixpath


class Main(App):
    def init(self):
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")
        self.parser.add_argument(
            "-n",
            "--no-launch",
            dest="launch_app",
            action="store_false",
            help="Don't launch app",
        )

        self.parser.add_argument("fap_src_path", help="App file to upload")
        self.parser.add_argument(
            "--fap_dst_dir", help="Upload path", default="/ext/apps", required=False
        )
        self.parser.set_defaults(func=self.install)

    def install(self):
        if not (port := resolve_port(self.logger, self.args.port)):
            return 1

        try:
            with FlipperStorage(port) as storage:
                storage_ops = FlipperStorageOperations(storage)
                fap_local_path = self.args.fap_src_path
                self.args.fap_dst_dir = self.args.fap_dst_dir.rstrip("/\\")

                if not os.path.isfile(fap_local_path):
                    self.logger.error(
                        f"Error: source .fap ({fap_local_path}) not found"
                    )
                    return 2

                fap_dst_path = posixpath.join(
                    self.args.fap_dst_dir, os.path.basename(fap_local_path)
                )

                self.logger.info(f'Installing "{fap_local_path}" to {fap_dst_path}')

                storage_ops.recursive_send(fap_dst_path, fap_local_path, False)

                if not self.args.launch_app:
                    return 0

                storage.send_and_wait_eol(
                    f'loader open "Applications" {fap_dst_path}\r'
                )

                if len(result := storage.read.until(storage.CLI_EOL)):
                    self.logger.error(f"Unexpected response: {result.decode('ascii')}")
                    return 3
                return 0

        except Exception as e:
            self.logger.error(f"Error: {e}")
            # raise
            return 4


if __name__ == "__main__":
    Main()()
