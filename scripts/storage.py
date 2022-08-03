#!/usr/bin/env python3

from flipper.app import App
from flipper.storage import FlipperStorage
from flipper.utils.cdc import resolve_port

import logging
import os
import binascii
import filecmp
import tempfile


class Main(App):
    def init(self):
        self.parser.add_argument("-p", "--port", help="CDC Port", default="auto")

        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_mkdir = self.subparsers.add_parser("mkdir", help="Create directory")
        self.parser_mkdir.add_argument("flipper_path", help="Flipper path")
        self.parser_mkdir.set_defaults(func=self.mkdir)

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

    def _get_storage(self):
        if not (port := resolve_port(self.logger, self.args.port)):
            return None

        storage = FlipperStorage(port)
        storage.start()
        return storage

    def mkdir(self):
        if not (storage := self._get_storage()):
            return 1

        self.logger.debug(f'Creating "{self.args.flipper_path}"')
        if not storage.mkdir(self.args.flipper_path):
            self.logger.error(f"Error: {storage.last_error}")
        storage.stop()
        return 0

    def remove(self):
        if not (storage := self._get_storage()):
            return 1

        self.logger.debug(f'Removing "{self.args.flipper_path}"')
        if not storage.remove(self.args.flipper_path):
            self.logger.error(f"Error: {storage.last_error}")
        storage.stop()
        return 0

    def receive(self):
        if not (storage := self._get_storage()):
            return 1

        if storage.exist_dir(self.args.flipper_path):
            for dirpath, dirnames, filenames in storage.walk(self.args.flipper_path):
                self.logger.debug(
                    f'Processing directory "{os.path.normpath(dirpath)}"'.replace(
                        os.sep, "/"
                    )
                )
                dirnames.sort()
                filenames.sort()

                rel_path = os.path.relpath(dirpath, self.args.flipper_path)

                for dirname in dirnames:
                    local_dir_path = os.path.join(
                        self.args.local_path, rel_path, dirname
                    )
                    local_dir_path = os.path.normpath(local_dir_path)
                    os.makedirs(local_dir_path, exist_ok=True)

                for filename in filenames:
                    local_file_path = os.path.join(
                        self.args.local_path, rel_path, filename
                    )
                    local_file_path = os.path.normpath(local_file_path)
                    flipper_file_path = os.path.normpath(
                        os.path.join(dirpath, filename)
                    ).replace(os.sep, "/")
                    self.logger.info(
                        f'Receiving "{flipper_file_path}" to "{local_file_path}"'
                    )
                    if not storage.receive_file(flipper_file_path, local_file_path):
                        self.logger.error(f"Error: {storage.last_error}")

        else:
            self.logger.info(
                f'Receiving "{self.args.flipper_path}" to "{self.args.local_path}"'
            )
            if not storage.receive_file(self.args.flipper_path, self.args.local_path):
                self.logger.error(f"Error: {storage.last_error}")
        storage.stop()
        return 0

    def send(self):
        if not (storage := self._get_storage()):
            return 1

        self.send_to_storage(
            storage, self.args.flipper_path, self.args.local_path, self.args.force
        )
        storage.stop()
        return 0

    # send file or folder recursively
    def send_to_storage(self, storage, flipper_path, local_path, force):
        if not os.path.exists(local_path):
            self.logger.error(f'Error: "{local_path}" is not exist')

        if os.path.isdir(local_path):
            # create parent dir
            self.mkdir_on_storage(storage, flipper_path)

            for dirpath, dirnames, filenames in os.walk(local_path):
                self.logger.debug(f'Processing directory "{os.path.normpath(dirpath)}"')
                dirnames.sort()
                filenames.sort()
                rel_path = os.path.relpath(dirpath, local_path)

                # create subdirs
                for dirname in dirnames:
                    flipper_dir_path = os.path.join(flipper_path, rel_path, dirname)
                    flipper_dir_path = os.path.normpath(flipper_dir_path).replace(
                        os.sep, "/"
                    )
                    self.mkdir_on_storage(storage, flipper_dir_path)

                # send files
                for filename in filenames:
                    flipper_file_path = os.path.join(flipper_path, rel_path, filename)
                    flipper_file_path = os.path.normpath(flipper_file_path).replace(
                        os.sep, "/"
                    )
                    local_file_path = os.path.normpath(os.path.join(dirpath, filename))
                    self.send_file_to_storage(
                        storage, flipper_file_path, local_file_path, force
                    )
        else:
            self.send_file_to_storage(storage, flipper_path, local_path, force)

    # make directory with exist check
    def mkdir_on_storage(self, storage, flipper_dir_path):
        if not storage.exist_dir(flipper_dir_path):
            self.logger.debug(f'"{flipper_dir_path}" does not exist, creating')
            if not storage.mkdir(flipper_dir_path):
                self.logger.error(f"Error: {storage.last_error}")
        else:
            self.logger.debug(f'"{flipper_dir_path}" already exists')

    # send file with exist check and hash check
    def send_file_to_storage(self, storage, flipper_file_path, local_file_path, force):
        if not storage.exist_file(flipper_file_path):
            self.logger.debug(
                f'"{flipper_file_path}" does not exist, sending "{local_file_path}"'
            )
            self.logger.info(f'Sending "{local_file_path}" to "{flipper_file_path}"')
            if not storage.send_file(local_file_path, flipper_file_path):
                self.logger.error(f"Error: {storage.last_error}")
        elif force:
            self.logger.debug(
                f'"{flipper_file_path}" exists, but will be overwritten by "{local_file_path}"'
            )
            self.logger.info(f'Sending "{local_file_path}" to "{flipper_file_path}"')
            if not storage.send_file(local_file_path, flipper_file_path):
                self.logger.error(f"Error: {storage.last_error}")
        else:
            self.logger.debug(
                f'"{flipper_file_path}" exists, compare hash with "{local_file_path}"'
            )
            hash_local = storage.hash_local(local_file_path)
            hash_flipper = storage.hash_flipper(flipper_file_path)

            if not hash_flipper:
                self.logger.error(f"Error: {storage.last_error}")

            if hash_local == hash_flipper:
                self.logger.debug(
                    f'"{flipper_file_path}" is equal to "{local_file_path}"'
                )
            else:
                self.logger.debug(
                    f'"{flipper_file_path}" is NOT equal to "{local_file_path}"'
                )
                self.logger.info(
                    f'Sending "{local_file_path}" to "{flipper_file_path}"'
                )
                if not storage.send_file(local_file_path, flipper_file_path):
                    self.logger.error(f"Error: {storage.last_error}")

    def read(self):
        if not (storage := self._get_storage()):
            return 1

        self.logger.debug(f'Reading "{self.args.flipper_path}"')
        data = storage.read_file(self.args.flipper_path)
        if not data:
            self.logger.error(f"Error: {storage.last_error}")
        else:
            try:
                print("Text data:")
                print(data.decode())
            except:
                print("Binary hexadecimal data:")
                print(binascii.hexlify(data).decode())
        storage.stop()
        return 0

    def size(self):
        if not (storage := self._get_storage()):
            return 1

        self.logger.debug(f'Getting size of "{self.args.flipper_path}"')
        size = storage.size(self.args.flipper_path)
        if size < 0:
            self.logger.error(f"Error: {storage.last_error}")
        else:
            print(size)
        storage.stop()
        return 0

    def list(self):
        if not (storage := self._get_storage()):
            return 1

        self.logger.debug(f'Listing "{self.args.flipper_path}"')
        storage.list_tree(self.args.flipper_path)
        storage.stop()
        return 0

    def stress(self):
        self.logger.error("This test is wearing out flash memory.")
        self.logger.error("Never use it with internal storage(/int)")

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

            storage = self._get_storage()
            if not storage:
                return 1

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
            storage.stop()
            return 0


if __name__ == "__main__":
    Main()()
