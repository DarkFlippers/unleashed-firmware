import hashlib
import os
import struct
from typing import TypedDict, List


class File(TypedDict):
    path: str
    size: int
    content_path: str


class Dir(TypedDict):
    path: str


class FileBundler:
    """
    u32 magic
    u32 version
    u32 dirs_count
    u32 files_count
    u32 signature_size
    u8[] signature
    Dirs:
      u32 dir_name length
      u8[] dir_name
    Files:
      u32 file_name length
      u8[] file_name
      u32 file_content_size
      u8[] file_content
    """

    def __init__(self, assets_dirs: List[object]):
        self.src_dirs = list(assets_dirs)

    def _gather(self, directory_path: str):
        if not os.path.isdir(directory_path):
            raise Exception(f"Assets directory {directory_path} does not exist")
        for root, dirs, files in os.walk(directory_path):
            for file_info in files:
                file_path = os.path.join(root, file_info)
                file_size = os.path.getsize(file_path)
                self.file_list.append(
                    {
                        "path": os.path.relpath(file_path, directory_path),
                        "size": file_size,
                        "content_path": file_path,
                    }
                )

            for dir_info in dirs:
                dir_path = os.path.join(root, dir_info)
                # dir_size = sum(
                #     os.path.getsize(os.path.join(dir_path, f)) for f in os.listdir(dir_path)
                # )
                self.directory_list.append(
                    {"path": os.path.relpath(dir_path, directory_path)}
                )

        self.file_list.sort(key=lambda f: f["path"])
        self.directory_list.sort(key=lambda d: d["path"])

    def _process_src_dirs(self):
        self.file_list: list[File] = []
        self.directory_list: list[Dir] = []
        for directory_path in self.src_dirs:
            self._gather(directory_path)

    def export(self, target_path: str):
        self._process_src_dirs()
        self._md5_hash = hashlib.md5()
        with open(target_path, "wb") as f:
            # Write header magic and version
            f.write(struct.pack("<II", 0x4F4C5A44, 0x01))

            # Write dirs count
            f.write(struct.pack("<I", len(self.directory_list)))

            # Write files count
            f.write(struct.pack("<I", len(self.file_list)))

            md5_hash_size = len(self._md5_hash.digest())

            # write signature size and null signature, we'll fill it in later
            f.write(struct.pack("<I", md5_hash_size))
            signature_offset = f.tell()
            f.write(b"\x00" * md5_hash_size)

            self._write_contents(f)

            f.seek(signature_offset)
            f.write(self._md5_hash.digest())

    def _write_contents(self, f):
        for dir_info in self.directory_list:
            f.write(struct.pack("<I", len(dir_info["path"]) + 1))
            f.write(dir_info["path"].encode("ascii") + b"\x00")
            self._md5_hash.update(dir_info["path"].encode("ascii") + b"\x00")

        # Write files
        for file_info in self.file_list:
            f.write(struct.pack("<I", len(file_info["path"]) + 1))
            f.write(file_info["path"].encode("ascii") + b"\x00")
            f.write(struct.pack("<I", file_info["size"]))
            self._md5_hash.update(file_info["path"].encode("ascii") + b"\x00")

            with open(file_info["content_path"], "rb") as content_file:
                content = content_file.read()
                f.write(content)
                self._md5_hash.update(content)
