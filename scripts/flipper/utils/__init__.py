import datetime
import hashlib


def timestamp():
    return int(datetime.datetime.now().timestamp())


def file_hash(path: str, algo: str, block_size: int = 4096):
    h = hashlib.new(algo)
    with open(path, "rb") as fd:
        while True:
            data = fd.read(block_size)
            if len(data) > 0:
                h.update(data)
            else:
                break
    return h.hexdigest()


def file_md5(path, block_size=4096):
    return file_hash(path, "md5", block_size)


def file_sha256(path, block_size=4096):
    return file_hash(path, "sha256", block_size)
