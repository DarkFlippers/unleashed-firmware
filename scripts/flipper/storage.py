import os
import sys
import serial
import time
import hashlib
import math


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


class BufferedRead:
    def __init__(self, stream):
        self.buffer = bytearray()
        self.stream = stream

    def until(self, eol="\n", cut_eol=True):
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
        self.last_error = ""
        self.chunk_size = chunk_size

    def start(self):
        self.port.open()
        self.port.reset_input_buffer()
        # Send a command with a known syntax to make sure the buffer is flushed
        self.send("device_info\r")
        self.read.until("hardware_model")
        # And read buffer until we get prompt
        self.read.until(self.CLI_PROMPT)

    def stop(self):
        self.port.close()

    def send(self, line):
        self.port.write(line.encode("ascii"))

    def send_and_wait_eol(self, line):
        self.send(line)
        return self.read.until(self.CLI_EOL)

    def send_and_wait_prompt(self, line):
        self.send(line)
        return self.read.until(self.CLI_PROMPT)

    def has_error(self, data):
        """Is data has error"""
        if data.find(b"Storage error") != -1:
            return True
        else:
            return False

    def get_error(self, data):
        """Extract error text from data and print it"""
        error, error_text = data.decode("ascii").split(": ")
        return error_text.strip()

    def list_tree(self, path="/", level=0):
        """List files and dirs on Flipper"""
        path = path.replace("//", "/")

        self.send_and_wait_eol('storage list "' + path + '"\r')

        data = self.read.until(self.CLI_PROMPT)
        lines = data.split(b"\r\n")

        for line in lines:
            try:
                # TODO: better decoding, considering non-ascii characters
                line = line.decode("ascii")
            except:
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

    def walk(self, path="/"):
        dirs = []
        nondirs = []
        walk_dirs = []

        path = path.replace("//", "/")
        self.send_and_wait_eol(f'storage list "{path}"\r')
        data = self.read.until(self.CLI_PROMPT)
        lines = data.split(b"\r\n")

        for line in lines:
            try:
                # TODO: better decoding, considering non-ascii characters
                line = line.decode("ascii")
            except:
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

        # topdown walk, yield before recursy
        yield path, dirs, nondirs
        for new_path in walk_dirs:
            yield from self.walk(new_path)

    def send_file(self, filename_from, filename_to):
        """Send file from local device to Flipper"""
        self.remove(filename_to)

        with open(filename_from, "rb") as file:
            filesize = os.fstat(file.fileno()).st_size

            buffer_size = self.chunk_size
            while True:
                filedata = file.read(buffer_size)
                size = len(filedata)
                if size == 0:
                    break

                self.send_and_wait_eol(f'storage write_chunk "{filename_to}" {size}\r')
                answer = self.read.until(self.CLI_EOL)
                if self.has_error(answer):
                    self.last_error = self.get_error(answer)
                    self.read.until(self.CLI_PROMPT)
                    return False

                self.port.write(filedata)
                self.read.until(self.CLI_PROMPT)

                percent = str(math.ceil(file.tell() / filesize * 100))
                total_chunks = str(math.ceil(filesize / buffer_size))
                current_chunk = str(math.ceil(file.tell() / buffer_size))
                sys.stdout.write(
                    f"\r{percent}%, chunk {current_chunk} of {total_chunks}"
                )
                sys.stdout.flush()
        print()
        return True

    def read_file(self, filename):
        """Receive file from Flipper, and get filedata (bytes)"""
        buffer_size = self.chunk_size
        self.send_and_wait_eol(
            'storage read_chunks "' + filename + '" ' + str(buffer_size) + "\r"
        )
        answer = self.read.until(self.CLI_EOL)
        filedata = bytearray()
        if self.has_error(answer):
            self.last_error = self.get_error(answer)
            self.read.until(self.CLI_PROMPT)
            return filedata
        size = int(answer.split(b": ")[1])
        read_size = 0

        while read_size < size:
            self.read.until("Ready?" + self.CLI_EOL)
            self.send("y")
            read_size = min(size - read_size, buffer_size)
            filedata.extend(self.port.read(read_size))
            read_size = read_size + read_size

            percent = str(math.ceil(read_size / size * 100))
            total_chunks = str(math.ceil(size / buffer_size))
            current_chunk = str(math.ceil(read_size / buffer_size))
            sys.stdout.write(f"\r{percent}%, chunk {current_chunk} of {total_chunks}")
            sys.stdout.flush()
        print()
        self.read.until(self.CLI_PROMPT)
        return filedata

    def receive_file(self, filename_from, filename_to):
        """Receive file from Flipper to local storage"""
        with open(filename_to, "wb") as file:
            data = self.read_file(filename_from)
            if not data:
                return False
            else:
                file.write(data)
                return True

    def exist(self, path):
        """Is file or dir exist on Flipper"""
        self.send_and_wait_eol('storage stat "' + path + '"\r')
        answer = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        if self.has_error(answer):
            self.last_error = self.get_error(answer)
            return False
        else:
            return True

    def exist_dir(self, path):
        """Is dir exist on Flipper"""
        self.send_and_wait_eol('storage stat "' + path + '"\r')
        answer = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        if self.has_error(answer):
            self.last_error = self.get_error(answer)
            return False
        else:
            if answer.find(b"Directory") != -1:
                return True
            elif answer.find(b"Storage") != -1:
                return True
            else:
                return False

    def exist_file(self, path):
        """Is file exist on Flipper"""
        self.send_and_wait_eol('storage stat "' + path + '"\r')
        answer = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        if self.has_error(answer):
            self.last_error = self.get_error(answer)
            return False
        else:
            if answer.find(b"File, size:") != -1:
                return True
            else:
                return False

    def size(self, path):
        """file size on Flipper"""
        self.send_and_wait_eol('storage stat "' + path + '"\r')
        answer = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        if self.has_error(answer):
            self.last_error = self.get_error(answer)
            return False
        else:
            if answer.find(b"File, size:") != -1:
                size = int(
                    "".join(
                        ch
                        for ch in answer.split(b": ")[1].decode("ascii")
                        if ch.isdigit()
                    )
                )
                return size
            else:
                self.last_error = "access denied"
                return -1

    def mkdir(self, path):
        """Create a directory on Flipper"""
        self.send_and_wait_eol('storage mkdir "' + path + '"\r')
        answer = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        if self.has_error(answer):
            self.last_error = self.get_error(answer)
            return False
        else:
            return True

    def remove(self, path):
        """Remove file or directory on Flipper"""
        self.send_and_wait_eol('storage remove "' + path + '"\r')
        answer = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        if self.has_error(answer):
            self.last_error = self.get_error(answer)
            return False
        else:
            return True

    def hash_local(self, filename):
        """Hash of local file"""
        hash_md5 = hashlib.md5()
        with open(filename, "rb") as f:
            for chunk in iter(lambda: f.read(self.chunk_size), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()

    def hash_flipper(self, filename):
        """Get hash of file on Flipper"""
        self.send_and_wait_eol('storage md5 "' + filename + '"\r')
        hash = self.read.until(self.CLI_EOL)
        self.read.until(self.CLI_PROMPT)

        if self.has_error(hash):
            self.last_error = self.get_error(hash)
            return ""
        else:
            return hash.decode("ascii")
