import logging
import argparse
import subprocess
import io
import os
import sys

ICONS_SUPPORTED_FORMATS = ["png"]


class Image:
    def __init__(self, width: int, height: int, data: bytes):
        self.width = width
        self.height = height
        self.data = data

    def write(self, filename):
        file = open(filename, "wb")
        file.write(self.data)


def is_file_an_icon(filename):
    extension = filename.lower().split(".")[-1]
    return extension in ICONS_SUPPORTED_FORMATS


def file2image(file):
    output = subprocess.check_output(["convert", file, "xbm:-"])
    assert output

    # Extract data from text
    f = io.StringIO(output.decode().strip())
    width = int(f.readline().strip().split(" ")[2])
    height = int(f.readline().strip().split(" ")[2])
    data = f.read().strip().replace("\n", "").replace(" ", "").split("=")[1][:-1]
    data_str = data[1:-1].replace(",", " ").replace("0x", "")

    data_bin = bytearray.fromhex(data_str)

    # Encode icon data with LZSS
    data_encoded_str = subprocess.check_output(
        ["heatshrink", "-e", "-w8", "-l4"], input=data_bin
    )

    assert data_encoded_str

    data_enc = bytearray(data_encoded_str)
    data_enc = bytearray([len(data_enc) & 0xFF, len(data_enc) >> 8]) + data_enc

    # Use encoded data only if its lenght less than original, including header
    if len(data_enc) < len(data_bin) + 1:
        data = b"\x01\x00" + data_enc
    else:
        data = b"\x00" + data_bin

    return Image(width, height, data)
