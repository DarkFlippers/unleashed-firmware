from enum import Enum

import serial


class RemoteIOError(IOError):
    pass


class UnsupportedOperation(RemoteIOError):
    # TODO: Tell more on error
    pass


class ReadWriteError(RemoteIOError):
    pass


class FileMode(Enum):
    READ = "r"
    WRITE = "w"
    # APPEND = "a" - Not Impl
    # READ_WRITE = "w+"
    # READ_APPEND = "a+"


class File:
    opened: bool = False
    filename: str
    conn: serial.Serial
    mode = FileMode.READ

    def _open(self):
        # Opening file on Flipper's end
        self.conn.write(f"f_open {self.filename} {self.mode}\n".encode())

        # Checking, if everything is OK.
        self.conn.write(b"f_valid\n")
        return bool(int(self.conn.read_until().decode()))

    def __init__(self, conn: serial.Serial, filename: str, mode=FileMode.READ):
        self.conn = conn
        self.mode = mode
        self.filename = filename

        if not self._open():
            raise RemoteIOError
        self.opened = True

    def lseek(self, size: int):
        self.conn.write(f"f_lseek {size}\n".encode())

    def tell(self):
        self.conn.write(b"f_tell\n")
        return int(self.conn.read_until().decode())

    def size(self):
        self.conn.write(b"f_size\n")
        return int(self.conn.read_until().decode())

    def write(self, data: bytes):
        if self.mode != FileMode.WRITE:
            raise UnsupportedOperation
        self.conn.write(f"f_write {len(data)}\n".encode())
        self.conn.write(data)
        self.conn.write(b"\n")

        if self.conn.read_until().decode().strip() != "OK":
            raise ReadWriteError

    def read(self, size: int) -> bytes:
        self.conn.write(f"f_read {size}\n".encode())
        return self.conn.read(size)

    def _close(self):
        self.conn.write(b"f_close\n")

    def close(self):
        self._close()
        # TODO: Add termination of remote file process.
        del self

    def read_all(self, block_size=128) -> bytes:
        """
        Function to simplify reading of file over serial.

        :return: content of file represented in `bytes`
        """
        size = self.size()
        a = bytearray()
        seek = 0
        # Reading in blocks of block_size
        while seek < block_size * (size // block_size - 1):
            seek += block_size
            self.lseek(seek)
            a.extend(self.read(block_size))

        seek += block_size
        self.lseek(seek)
        a.extend(self.read(size - seek))  # Appending mod

        self.lseek(0)  # Resetting seek to default-position

        return bytes(a)


def push(ser: serial.Serial, local_path, remote_path):
    with open(local_path, "rb") as local_f:  # Reading local file
        a = local_f.read()

    remote_f = File(ser, remote_path, FileMode.WRITE)
    remote_f.write(a)
    remote_f.close()


def pull(ser: serial.Serial, remote_path, local_path):
    remote_f = File(ser, remote_path, FileMode.READ)

    with open(local_path, "wb") as f:
        f.write(remote_f.read_all())

    remote_f.close()
