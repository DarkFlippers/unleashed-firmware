import enum
import hashlib
import logging
import math
import os
import posixpath
import sys
import time

import serial


def timing(func):
    """
    Speedometer decorator
    """

    def wrapper(*args, **kwargs):
        time1 = time.monotonic()
        ret = func(*args, **kwargs)
        time2 = time.monotonic()
        print(
            "{:s} function took {:.3f} ms".format(
                func.__name__, (time2 - time1) * 1000.0
            )
        )
        return ret

    return wrapper


class StorageErrorCode(enum.Enum):
    OK = "OK"
    NOT_READY = "filesystem not ready"
    EXIST = "file/dir already exist"
    NOT_EXIST = "file/dir not exist"
    INVALID_PARAMETER = "invalid parameter"
    DENIED = "access denied"
    INVALID_NAME = "invalid name/path"
    INTERNAL = "internal error"
    NOT_IMPLEMENTED = "function not implemented"
    ALREADY_OPEN = "file is already open"
    UNKNOWN = "unknown error"

    @property
    def is_error(self):
        return self != self.OK

    @classmethod
    def from_value(cls, s: str | bytes):
        if isinstance(s, bytes):
            s = s.decode("ascii")
        for code in cls:
            if code.value == s:
                return code
        return cls.UNKNOWN


class FlipperStorageException(Exception):
    @staticmethod
    def from_error_code(path: str, error_code: StorageErrorCode):
        return FlipperStorageException(
            f"Storage error: path '{path}': {error_code.value}"
        )


class BufferedRead:
    def __init__(self, stream):
        self.buffer = bytearray()
        self.stream = stream

    def until(self, eol: str = "\n", cut_eol: bool = True):
        eol = eol.encode("ascii")
        while True:
            # search in buffer
            i = self.buffer.find(eol)
            if i >= 0:
                if cut_eol:
                    read = self.buffer[:i]
                else:
                    read = self.buffer[: i + len(eol)]
                self.buffer = self.buffer[i + len(eol) :]
                return read

            # read and append to buffer
            i = max(1, self.stream.in_waiting)
            data = self.stream.read(i)
            self.buffer.extend(data)


class FlipperStorage:
    CLI_PROMPT = ">: "
    CLI_EOL = "\r\n"

    def __init__(self, portname: str, chunk_size: int = 8192):
        self.port = serial.Serial()
        self.port.port = portname
        self.port.timeout = 2
        self.port.baudrate = 115200  # Doesn't matter for VCP
        self.read = BufferedRead(self.port)
        self.chunk_size = chunk_size

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.stop()

    def start(self):
        self.port.open()
        self.port.reset_input_buffer()
        # Send a command with a known syntax to make sure the buffer is flushed
        self.send("device_info\r")
        self.read.until("hardware_model")
        # And read buffer until we get prompt
        self.read.until(self.CLI_PROMPT)

    def stop(self) -> None:
        self.port.close()

    def send(self, line: str) -> None:
        self.port.write(line.encode("ascii"))

    def send_and_wait_eol(self, line: str):
        self.send(line)
        return self.read.until(self.CLI_EOL)

    def send_and_wait_prompt(self, line: str):
        self.send(line)
        return self.read.until(self.CLI_PROMPT)

    def has_error(self, data: bytes | str) -> bool:
        """Is data an error message"""
        return data.find(b"Storage error:") != -1

    def get_error(self, data: bytes) -> StorageErrorCode:
        """Extract error text from data and print it"""
        _, error_text = data.decode("ascii").split(": ")
        return StorageErrorCode.from_value(error_text.strip())

    def list_tree(self, path: str = "/", level: int = 0):
        """List files and dirs on Flipper"""
        path = path.replace("//", "/")

        self.send_and_wait_eol(f'storage list "{path}"\r')

        data = self.read.until(self.CLI_PROMPT)
        lines = data.split(b"\r\n")

        for line in lines:
            try:
                # TODO FL-3539: better decoding, considering non-ascii characters
                line = line.decode("ascii")
            except Exception:
                continue

            line = line.strip()

            if len(line) == 0:
                continue

            if self.has_error(line.encode("ascii")):
                print(self.get_error(line.encode("ascii")))
                continue

            if line == "Empty":
                continue

            type, info = line.split(" ", 1)
            if type == "[D]":
                # Print directory name
                print((path + "/" + info).replace("//", "/"))
                # And recursively go inside
                self.list_tree(path + "/" + info, level + 1)
            elif type == "[F]":
                name, size = info.rsplit(" ", 1)
                # Print file name and size
                print((path + "/" + name).replace("//", "/") + ", size " + size)
            else:
                # Something wrong, pass
                pass

    def walk(self, path: str = "/"):
        dirs = []
        nondirs = []
        walk_dirs = []

        path = path.replace("//", "/")
        self.send_and_wait_eol(f'storage list "{path}"\r')
        data = self.read.until(self.CLI_PROMPT)
        lines = data.split(b"\r\n")

        for line in lines:
            try:
                # TODO FL-3539: better decoding, considering non-ascii characters
                line = line.decode("ascii")
            except Exception:
                continue

            line = line.strip()

            if len(line) == 0:
                continue

            if self.has_error(line.encode("ascii")):
                continue

            if line == "Empty":
                continue

            type, info = line.split(" ", 1)
            if type == "[D]":
                # Print directory name
                dirs.append(info)
                walk_dirs.append((path + "/" + info).replace("//", "/"))

            elif type == "[F]":
                name, size = info.rsplit(" ", 1)
                # Print file name and size
                nondirs.append(name)
            else:
                # Something wrong, pass
                pass

        # topdown walk, yield before recursing
        yield path, dirs, nondirs
        for new_path in walk_dirs:
            yield from self.walk(new_path)

    def send_file(self, filename_from: str, filename_to: str):
        """Send file from local device to Flipper"""
        if self.exist_file(filename_to):
            self.remove(filename_to)

        with open(filename_from, "rb") as file:
            filesize = os.fstat(file.fileno()).st_size

            buffer_size = self.chunk_size
            start_time = time.time()
            while True:
                filedata = file.read(buffer_size)
                size = len(filedata)
                if size == 0:
                    break

                self.send_and_wait_eol(f'storage write_chunk "{filename_to}" {size}\r')
                answer = self.read.until(self.CLI_EOL)
                if self.has_error(answer):
                    last_error = self.get_error(answer)
                    self.read.until(self.CLI_PROMPT)
                    raise FlipperStorageException.from_error_code(
                        filename_to, last_error
                    )

                self.port.write(filedata)
                self.read.until(self.CLI_PROMPT)

                ftell = file.tell()
                percent = math.ceil(ftell / filesize * 100)
                total_chunks = math.ceil(filesize / buffer_size)
                current_chunk = math.ceil(ftell / buffer_size)
                approx_speed = ftell / (time.time() - start_time + 0.0001)
                sys.stdout.write(
                    f"\r<{percent:3d}%, chunk {current_chunk:2d} of {total_chunks:2d} @ {approx_speed/1024:.2f} kb/s"
                )
                sys.stdout.flush()
        print()

    def read_file(self, filename: str):
        """Receive file from Flipper, and get filedata (bytes)"""
        buffer_size = self.chunk_size
        start_time = time.time()
        self.send_and_wait_eol(
            'storage read_chunks "' + filename + '" ' + str(buffer_size) + "\r"
        )
        answer = self.read.until(self.CLI_EOL)
        filedata = bytearray()
        if self.has_error(answer):
            last_error = self.get_error(answer)
            self.read.until(self.CLI_PROMPT)
            raise FlipperStorageException(filename, last_error)
            # return filedata
        size = int(answer.split(b": ")[1])
        read_size = 0

        while read_size < size:
            self.read.until("Ready?" + self.CLI_EOL)
            self.send("y")
            chunk_size = min(size - read_size, buffer_size)
            filedata.extend(self.port.read(chunk_size))
            read_size = read_size + chunk_size

            percent = math.ceil(read_size / size * 100)
            total_chunks = math.ceil(size / buffer_size)
            current_chunk = math.ceil(read_size / buffer_size)
            approx_speed = read_size / (time.time() - start_time + 0.0001)
            sys.stdout.write(
                f"\r>{percent:3d}%, chunk {current_chunk:2d} of {total_chunks:2d} @ {approx_speed/1024:.2f} kb/s"
            )
            sys.stdout.flush()
        print()
        self.read.until(self.CLI_PROMPT)
        return filedata

    def receive_file(self, filename_from: str, filename_to: str):
        """Receive file from Flipper to local storage"""
        with open(filename_to, "wb") as file:
            data = self.read_file(filename_from)
            file.write(data)

    def exist(self, path: str):
        """Does file or dir exist on Flipper"""
        self.send_and_wait_eol(f'storage stat "{path}"\r')
        response = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        return not self.has_error(response)

    def exist_dir(self, path: str):
        """Does dir exist on Flipper"""
        self.send_and_wait_eol(f'storage stat "{path}"\r')
        response = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)
        if self.has_error(response):
            error_code = self.get_error(response)
            if error_code in (
                StorageErrorCode.NOT_EXIST,
                StorageErrorCode.INVALID_NAME,
            ):
                return False
            raise FlipperStorageException.from_error_code(path, error_code)

        return response == b"Directory" or response.startswith(b"Storage")

    def exist_file(self, path: str):
        """Does file exist on Flipper"""
        self.send_and_wait_eol(f'storage stat "{path}"\r')
        response = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        return response.find(b"File, size:") != -1

    def _check_no_error(self, response, path=None):
        if self.has_error(response):
            raise FlipperStorageException.from_error_code(
                path, self.get_error(response)
            )

    def size(self, path: str):
        """file size on Flipper"""
        self.send_and_wait_eol(f'storage stat "{path}"\r')
        response = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        self._check_no_error(response, path)
        if response.find(b"File, size:") != -1:
            size = int(
                "".join(
                    ch
                    for ch in response.split(b": ")[1].decode("ascii")
                    if ch.isdigit()
                )
            )
            return size
        raise FlipperStorageException("Not a file")

    def mkdir(self, path: str):
        """Create a directory on Flipper"""
        self.send_and_wait_eol(f'storage mkdir "{path}"\r')
        response = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)
        self._check_no_error(response, path)

    def format_ext(self):
        """Format external storage on Flipper"""
        self.send_and_wait_eol("storage format /ext\r")
        self.send_and_wait_eol("y\r")
        response = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)
        self._check_no_error(response, "/ext")

    def remove(self, path: str):
        """Remove file or directory on Flipper"""
        self.send_and_wait_eol(f'storage remove "{path}"\r')
        response = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)
        self._check_no_error(response, path)

    def hash_local(self, filename: str):
        """Hash of local file"""
        hash_md5 = hashlib.md5()
        with open(filename, "rb") as f:
            for chunk in iter(lambda: f.read(self.chunk_size), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()

    def hash_flipper(self, filename: str):
        """Get hash of file on Flipper"""
        self.send_and_wait_eol('storage md5 "' + filename + '"\r')
        hash = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)
        self._check_no_error(hash, filename)
        return hash.decode("ascii")


class FlipperStorageOperations:
    def __init__(self, storage):
        self.storage: FlipperStorage = storage
        self.logger = logging.getLogger("FStorageOps")

    def send_file_to_storage(
        self, flipper_file_path: str, local_file_path: str, force: bool = False
    ):
        self.logger.debug(
            f"* send_file_to_storage:  {local_file_path}->{flipper_file_path}, {force=}"
        )
        exists = self.storage.exist_file(flipper_file_path)
        do_upload = not exists
        if exists:
            hash_local = self.storage.hash_local(local_file_path)
            hash_flipper = self.storage.hash_flipper(flipper_file_path)
            self.logger.debug(f"hash check: local {hash_local}, flipper {hash_flipper}")
            do_upload = force or (hash_local != hash_flipper)

        if do_upload:
            self.logger.info(f'Sending "{local_file_path}" to "{flipper_file_path}"')
            self.storage.send_file(local_file_path, flipper_file_path)

    # make directory with exist check
    def mkpath(self, flipper_dir_path: str):
        path_components, dirs_to_create = flipper_dir_path.split("/"), []
        while not self.storage.exist_dir(dir_path := "/".join(path_components)):
            self.logger.debug(f'"{dir_path}" does not exist, will create')
            dirs_to_create.append(path_components.pop())
        for dir_to_create in reversed(dirs_to_create):
            path_components.append(dir_to_create)
            self.storage.mkdir("/".join(path_components))

    # send file or folder recursively
    def recursive_send(self, flipper_path: str, local_path: str, force: bool = False):
        if not os.path.exists(local_path):
            raise FlipperStorageException(f'"{local_path}" does not exist')

        if os.path.isdir(local_path):
            # create parent dir
            self.mkpath(flipper_path)

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
                    self.mkpath(flipper_dir_path)

                # send files
                for filename in filenames:
                    flipper_file_path = os.path.join(flipper_path, rel_path, filename)
                    flipper_file_path = os.path.normpath(flipper_file_path).replace(
                        os.sep, "/"
                    )
                    local_file_path = os.path.normpath(os.path.join(dirpath, filename))
                    self.send_file_to_storage(flipper_file_path, local_file_path, force)
        else:
            self.mkpath(posixpath.dirname(flipper_path))
            self.send_file_to_storage(flipper_path, local_path, force)

    def recursive_receive(self, flipper_path: str, local_path: str):
        if self.storage.exist_dir(flipper_path):
            for dirpath, dirnames, filenames in self.storage.walk(flipper_path):
                self.logger.debug(
                    f'Processing directory "{os.path.normpath(dirpath)}"'.replace(
                        os.sep, "/"
                    )
                )
                dirnames.sort()
                filenames.sort()

                rel_path = os.path.relpath(dirpath, flipper_path)

                for dirname in dirnames:
                    local_dir_path = os.path.join(local_path, rel_path, dirname)
                    local_dir_path = os.path.normpath(local_dir_path)
                    os.makedirs(local_dir_path, exist_ok=True)

                for filename in filenames:
                    local_file_path = os.path.join(local_path, rel_path, filename)
                    local_file_path = os.path.normpath(local_file_path)
                    flipper_file_path = os.path.normpath(
                        os.path.join(dirpath, filename)
                    ).replace(os.sep, "/")
                    self.logger.info(
                        f'Receiving "{flipper_file_path}" to "{local_file_path}"'
                    )
                    self.storage.receive_file(flipper_file_path, local_file_path)

        else:
            self.logger.info(f'Receiving "{flipper_path}" to "{local_path}"')
            self.storage.receive_file(flipper_path, local_path)
