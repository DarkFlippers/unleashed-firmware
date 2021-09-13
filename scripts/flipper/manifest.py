import datetime
import logging
import os

from .utils import *
from .fstree import *

MANIFEST_VERSION = 0


class ManifestRecord:
    tag = None

    @staticmethod
    def fromLine(line):
        raise NotImplementedError

    def toLine(self):
        raise NotImplementedError

    def _unpack(self, manifest, key, type):
        key, value = manifest.readline().split(":", 1)
        assert key == key
        return type(value)


MANIFEST_TAGS_RECORDS = {}


def addManifestRecord(record: ManifestRecord):
    assert record.tag
    MANIFEST_TAGS_RECORDS[record.tag] = record


class ManifestRecordVersion(ManifestRecord):
    tag = "V"

    def __init__(self, version):
        self.version = version

    @staticmethod
    def fromLine(line):
        return ManifestRecordVersion(int(line))

    def toLine(self):
        return f"{self.tag}:{self.version}\n"


addManifestRecord(ManifestRecordVersion)


class ManifestRecordTimestamp(ManifestRecord):
    tag = "T"

    def __init__(self, timestamp: int):
        self.timestamp = int(timestamp)

    @staticmethod
    def fromLine(line):
        return ManifestRecordTimestamp(int(line))

    def toLine(self):
        return f"{self.tag}:{self.timestamp}\n"


addManifestRecord(ManifestRecordTimestamp)


class ManifestRecordDirectory(ManifestRecord):
    tag = "D"

    def __init__(self, path: str):
        self.path = path

    @staticmethod
    def fromLine(line):
        return ManifestRecordDirectory(line)

    def toLine(self):
        return f"{self.tag}:{self.path}\n"


addManifestRecord(ManifestRecordDirectory)


class ManifestRecordFile(ManifestRecord):
    tag = "F"

    def __init__(self, path: str, md5: str, size: int):
        self.path = path
        self.md5 = md5
        self.size = size

    @staticmethod
    def fromLine(line):
        data = line.split(":", 3)
        return ManifestRecordFile(data[2], data[0], data[1])

    def toLine(self):
        return f"{self.tag}:{self.md5}:{self.size}:{self.path}\n"


addManifestRecord(ManifestRecordFile)


class Manifest:
    def __init__(self):
        self.version = None
        self.records = []
        self.records.append(ManifestRecordVersion(MANIFEST_VERSION))
        self.records.append(ManifestRecordTimestamp(timestamp()))
        self.logger = logging.getLogger(self.__class__.__name__)

    def load(self, filename):
        manifest = open(filename, "r")
        for line in manifest.readlines():
            line = line.strip()
            if len(line) == 0:
                continue
            tag, line = line.split(":", 1)
            record = MANIFEST_TAGS_RECORDS[tag].fromLine(line)
            self.records.append(record)

    def save(self, filename):
        manifest = open(filename, "w+")
        for record in self.records:
            manifest.write(record.toLine())
        manifest.close()

    def addDirectory(self, path):
        self.records.append(ManifestRecordDirectory(path))

    def addFile(self, path, md5, size):
        self.records.append(ManifestRecordFile(path, md5, size))

    def create(self, directory_path):
        for root, dirs, files in os.walk(directory_path):
            relative_root = root.replace(directory_path, "", 1)
            if relative_root.startswith("/"):
                relative_root = relative_root[1:]
            # process directories
            for dir in dirs:
                relative_dir_path = os.path.join(relative_root, dir)
                self.logger.info(f'Adding directory: "{relative_dir_path}"')
                self.addDirectory(relative_dir_path)
            # Process files
            for file in files:
                relative_file_path = os.path.join(relative_root, file)
                full_file_path = os.path.join(root, file)
                self.logger.info(f'Adding file: "{relative_file_path}"')
                self.addFile(
                    relative_file_path,
                    file_md5(full_file_path),
                    os.path.getsize(full_file_path),
                )

    def toFsTree(self):
        root = FsNode("", FsNode.Type.Directory)
        for record in self.records:
            if isinstance(record, ManifestRecordDirectory):
                root.addDirectory(record.path)
            elif isinstance(record, ManifestRecordFile):
                root.addFile(record.path, record.md5, record.size)
        return root

    @staticmethod
    def compare(left: "Manifest", right: "Manifest"):
        return compare_fs_trees(left.toFsTree(), right.toFsTree())
