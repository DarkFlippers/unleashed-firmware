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
        with open(filename, "wb") as file:
            file.write(self.data)

    def data_as_carray(self):
        return (
            "{" + "".join("0x{:02x},".format(img_byte) for img_byte in self.data) + "}"
        )


def is_file_an_icon(filename):
    extension = filename.lower().split(".")[-1]
    return extension in ICONS_SUPPORTED_FORMATS


class ImageTools:
    __pil_unavailable = False
    __hs2_unavailable = False

    @staticmethod
    def is_processing_slow():
        try:
            from PIL import Image, ImageOps
            import heatshrink2

            return False
        except ImportError as e:
            return True

    def __init__(self):
        self.logger = logging.getLogger()

    def png2xbm(self, file):
        if self.__pil_unavailable:
            return subprocess.check_output(["convert", file, "xbm:-"])

        try:
            from PIL import Image, ImageOps
        except ImportError as e:
            self.__pil_unavailable = True
            self.logger.info("pillow module is missing, using convert cli util")
            return self.png2xbm(file)

        with Image.open(file) as im:
            with io.BytesIO() as output:
                bw = im.convert("1")
                bw = ImageOps.invert(bw)
                bw.save(output, format="XBM")
                return output.getvalue()

    def xbm2hs(self, data):
        if self.__hs2_unavailable:
            return subprocess.check_output(
                ["heatshrink", "-e", "-w8", "-l4"], input=data
            )

        try:
            import heatshrink2
        except ImportError as e:
            self.__hs2_unavailable = True
            self.logger.info("heatshrink2 module is missing, using heatshrink cli util")
            return self.xbm2hs(data)

        return heatshrink2.compress(data, window_sz2=8, lookahead_sz2=4)


__tools = ImageTools()


def file2image(file):
    output = __tools.png2xbm(file)
    assert output

    # Extract data from text
    f = io.StringIO(output.decode().strip())
    width = int(f.readline().strip().split(" ")[2])
    height = int(f.readline().strip().split(" ")[2])
    data = f.read().strip().replace("\n", "").replace(" ", "").split("=")[1][:-1]
    data_str = data[1:-1].replace(",", " ").replace("0x", "")

    data_bin = bytearray.fromhex(data_str)

    # Encode icon data with LZSS
    data_encoded_str = __tools.xbm2hs(data_bin)

    assert data_encoded_str

    data_enc = bytearray(data_encoded_str)
    data_enc = bytearray([len(data_enc) & 0xFF, len(data_enc) >> 8]) + data_enc

    # Use encoded data only if its length less than original, including header
    if len(data_enc) + 2 < len(data_bin) + 1:
        data = b"\x01\x00" + data_enc
    else:
        data = b"\x00" + data_bin

    return Image(width, height, data)
