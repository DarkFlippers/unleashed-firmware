#!/usr/bin/env python3

import operator
from functools import reduce
import time

from flipper.app import App
from flipper.storage import FlipperStorage, FlipperStorageOperations
from flipper.utils.cdc import resolve_port


class Main(App):
    APP_POST_CLOSE_DELAY_SEC = 0.2

    def init(self):
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")
        self.parser.add_argument(
            "--sources",
            "-s",
            nargs="+",
            action="append",
            default=[],
            help="Files to send",
        )
        self.parser.add_argument(
            "--targets",
            "-t",
            nargs="+",
            action="append",
            default=[],
            help="File destinations (must be same length as -s)",
        )
        self.parser.add_argument(
            "--host-app",
            "-a",
            help="Host app to launch",
        )

        self.parser.set_defaults(func=self.install)

    @staticmethod
    def flatten(item_list):
        return reduce(operator.concat, item_list, [])

    def install(self):
        self.args.sources = self.flatten(self.args.sources)
        self.args.targets = self.flatten(self.args.targets)

        if len(self.args.sources) != len(self.args.targets):
            self.logger.error(
                f"Error: sources ({self.args.sources}) and targets ({self.args.targets}) must be same length"
            )
            return 1

        if not (port := resolve_port(self.logger, self.args.port)):
            return 2

        try:
            with FlipperStorage(port) as storage:
                storage_ops = FlipperStorageOperations(storage)
                for fap_local_path, fap_dst_path in zip(
                    self.args.sources, self.args.targets
                ):
                    self.logger.info(f'Installing "{fap_local_path}" to {fap_dst_path}')

                    storage_ops.recursive_send(fap_dst_path, fap_local_path, False)

                fap_host_app = self.args.targets[0]
                startup_command = f"{fap_host_app}"
                if self.args.host_app:
                    startup_command = self.args.host_app

                self.logger.info("Closing current app, if any")
                for _ in range(10):
                    storage.send_and_wait_eol("loader close\r")
                    result = storage.read.until(storage.CLI_EOL)
                    if b"was closed" in result:
                        self.logger.info("App closed")
                        storage.read.until(storage.CLI_EOL)
                        time.sleep(self.APP_POST_CLOSE_DELAY_SEC)
                    elif result.startswith(b"No application"):
                        storage.read.until(storage.CLI_EOL)
                        break
                    else:
                        self.logger.error(
                            f"Unexpected response: {result.decode('ascii')}"
                        )
                        return 4

                self.logger.info(f"Launching app: {startup_command}")
                storage.send_and_wait_eol(f'loader open "{startup_command}"\r')

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
