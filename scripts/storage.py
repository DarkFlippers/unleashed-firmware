#!/usr/bin/env python3

from flipper.app import App
from flipper.storage import FlipperStorage, FlipperStorageOperations
from flipper.utils.cdc import resolve_port

import os
import binascii
import filecmp
import tempfile


def WrapStorageOp(func):
    def wrapper(*args, **kwargs):
        try:
            func(*args, **kwargs)
            return 0
        except Exception as e:
            print(f"Error: {e}")
            # raise  # uncomment to debug
            return 1

    return wrapper


class Main(App):
    def init(self):
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")

        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_mkdir = self.subparsers.add_parser("mkdir", help="Create directory")
        self.parser_mkdir.add_argument("flipper_path", help="Flipper path")
        self.parser_mkdir.set_defaults(func=self.mkdir)

        self.parser_format = self.subparsers.add_parser(
            "format_ext", help="Format flash card"
        )
        self.parser_format.set_defaults(func=self.format_ext)

        self.parser_remove = self.subparsers.add_parser(
            "remove", help="Remove file/directory"
        )
        self.parser_remove.add_argument("flipper_path", help="Flipper path")
        self.parser_remove.set_defaults(func=self.remove)

        self.parser_read = self.subparsers.add_parser("read", help="Read file")
        self.parser_read.add_argument("flipper_path", help="Flipper path")
        self.parser_read.set_defaults(func=self.read)

        self.parser_size = self.subparsers.add_parser("size", help="Size of file")
        self.parser_size.add_argument("flipper_path", help="Flipper path")
        self.parser_size.set_defaults(func=self.size)

        self.parser_receive = self.subparsers.add_parser("receive", help="Receive file")
        self.parser_receive.add_argument("flipper_path", help="Flipper path")
        self.parser_receive.add_argument("local_path", help="Local path")
        self.parser_receive.set_defaults(func=self.receive)

        self.parser_send = self.subparsers.add_parser(
            "send", help="Send file or directory"
        )
        self.parser_send.add_argument(
            "-f", "--force", help="Force sending", action="store_true"
        )
        self.parser_send.add_argument("local_path", help="Local path")
        self.parser_send.add_argument("flipper_path", help="Flipper path")
        self.parser_send.set_defaults(func=self.send)

        self.parser_list = self.subparsers.add_parser(
            "list", help="Recursively list files and dirs"
        )
        self.parser_list.add_argument("flipper_path", help="Flipper path", default="/")
        self.parser_list.set_defaults(func=self.list)

        self.parser_stress = self.subparsers.add_parser("stress", help="Stress test")
        self.parser.add_argument(
            "-c", "--count", type=int, default=10, help="Iteration count"
        )
        self.parser_stress.add_argument("flipper_path", help="Flipper path")
        self.parser_stress.add_argument(
            "file_size", type=int, help="Test file size in bytes"
        )
        self.parser_stress.set_defaults(func=self.stress)

    def _get_port(self):
        if not (port := resolve_port(self.logger, self.args.port)):
            raise Exception("Failed to resolve port")
        return port

    @WrapStorageOp
    def mkdir(self):
        self.logger.debug(f'Creating "{self.args.flipper_path}"')
        with FlipperStorage(self._get_port()) as storage:
            storage.mkdir(self.args.flipper_path)

    @WrapStorageOp
    def remove(self):
        self.logger.debug(f'Removing "{self.args.flipper_path}"')
        with FlipperStorage(self._get_port()) as storage:
            storage.remove(self.args.flipper_path)

    @WrapStorageOp
    def receive(self):
        with FlipperStorage(self._get_port()) as storage:
            FlipperStorageOperations(storage).recursive_receive(
                self.args.flipper_path, self.args.local_path
            )

    @WrapStorageOp
    def send(self):
        with FlipperStorage(self._get_port()) as storage:
            FlipperStorageOperations(storage).recursive_send(
                self.args.flipper_path, self.args.local_path, self.args.force
            )

    @WrapStorageOp
    def read(self):
        self.logger.debug(f'Reading "{self.args.flipper_path}"')
        with FlipperStorage(self._get_port()) as storage:
            data = storage.read_file(self.args.flipper_path)
            try:
                print("Text data:")
                print(data.decode())
            except:
                print("Binary hexadecimal data:")
                print(binascii.hexlify(data).decode())

    @WrapStorageOp
    def size(self):
        self.logger.debug(f'Getting size of "{self.args.flipper_path}"')
        with FlipperStorage(self._get_port()) as storage:
            print(storage.size(self.args.flipper_path))

    @WrapStorageOp
    def list(self):
        self.logger.debug(f'Listing "{self.args.flipper_path}"')
        with FlipperStorage(self._get_port()) as storage:
            storage.list_tree(self.args.flipper_path)

    @WrapStorageOp
    def format_ext(self):
        self.logger.debug("Formatting /ext SD card")
        with FlipperStorage(self._get_port()) as storage:
            storage.format_ext()

    @WrapStorageOp
    def stress(self):
        self.logger.error("This test is wearing out flash memory.")
        self.logger.error("Never use it with internal storage (/int)")

        if self.args.flipper_path.startswith(
            "/int"
        ) or self.args.flipper_path.startswith("/any"):
            self.logger.error("Stop at this point or device warranty will be void")
            say = input("Anything to say? ").strip().lower()
            if say != "void":
                return 2
            say = input("Why, Mr. Anderson? ").strip().lower()
            if say != "because":
                return 3

        with tempfile.TemporaryDirectory() as tmpdirname:
            send_file_name = os.path.join(tmpdirname, "send")
            receive_file_name = os.path.join(tmpdirname, "receive")
            with open(send_file_name, "w") as fout:
                fout.write("A" * self.args.file_size)

            with FlipperStorage(self._get_port()) as storage:
                if storage.exist_file(self.args.flipper_path):
                    self.logger.error("File exists, remove it first")
                    return
                while self.args.count > 0:
                    storage.send_file(send_file_name, self.args.flipper_path)
                    storage.receive_file(self.args.flipper_path, receive_file_name)
                    if not filecmp.cmp(receive_file_name, send_file_name):
                        self.logger.error("Files mismatch")
                        break
                    storage.remove(self.args.flipper_path)
                    os.unlink(receive_file_name)
                    self.args.count -= 1


if __name__ == "__main__":
    Main()()
