#!/usr/bin/env python3

from flipper.app import App
from flipper.utils.fff import FlipperFormatFile
from os.path import basename, join, exists
import os
import shutil
import zlib
import tarfile


class Main(App):
    #  No compression, plain tar
    ASSET_TAR_MODE = "w:"

    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # generate
        self.parser_generate = self.subparsers.add_parser(
            "generate", help="Generate update description file"
        )

        self.parser_generate.add_argument("-d", dest="directory", required=True)
        self.parser_generate.add_argument("-v", dest="version", required=True)
        self.parser_generate.add_argument("-t", dest="target", required=True)
        self.parser_generate.add_argument("-dfu", dest="dfu", required=False)
        self.parser_generate.add_argument("-a", dest="assets", required=False)
        self.parser_generate.add_argument("-stage", dest="stage", required=True)
        self.parser_generate.add_argument(
            "-radio", dest="radiobin", default="", required=False
        )
        self.parser_generate.add_argument(
            "-radioaddr", dest="radioaddr", required=False
        )
        self.parser_generate.add_argument(
            "-radiover", dest="radioversion", required=False
        )

        self.parser_generate.set_defaults(func=self.generate)

    def generate(self):
        stage_basename = basename(self.args.stage)
        dfu_basename = basename(self.args.dfu)
        radiobin_basename = basename(self.args.radiobin)
        assets_basename = ""

        if not exists(self.args.directory):
            os.makedirs(self.args.directory)

        shutil.copyfile(self.args.stage, join(self.args.directory, stage_basename))
        shutil.copyfile(self.args.dfu, join(self.args.directory, dfu_basename))
        if radiobin_basename:
            shutil.copyfile(
                self.args.radiobin, join(self.args.directory, radiobin_basename)
            )
        if self.args.assets:
            assets_basename = "assets.tar"
            self.package_assets(
                self.args.assets, join(self.args.directory, assets_basename)
            )

        file = FlipperFormatFile()
        file.setHeader("Flipper firmware upgrade configuration", 1)
        file.writeKey("Info", self.args.version)
        file.writeKey("Target", self.args.target[1:])  # dirty 'f' strip
        file.writeKey("Loader", stage_basename)
        file.writeComment("little-endian hex!")
        file.writeKey("Loader CRC", self.int2ffhex(self.crc(self.args.stage)))
        file.writeKey("Firmware", dfu_basename)
        file.writeKey("Radio", radiobin_basename or "")
        file.writeKey("Radio address", self.int2ffhex(self.args.radioaddr or 0))
        file.writeKey("Radio version", self.int2ffhex(self.args.radioversion or 0))
        if radiobin_basename:
            file.writeKey("Radio CRC", self.int2ffhex(self.crc(self.args.radiobin)))
        else:
            file.writeKey("Radio CRC", self.int2ffhex(0))
        file.writeKey("Assets", assets_basename)
        file.save(join(self.args.directory, "update.fuf"))

        return 0

    def package_assets(self, srcdir: str, dst_name: str):
        with tarfile.open(
            dst_name, self.ASSET_TAR_MODE, format=tarfile.USTAR_FORMAT
        ) as tarball:
            tarball.add(srcdir, arcname="")

    @staticmethod
    def int2ffhex(value: int):
        hexstr = "%08X" % value
        return " ".join(list(Main.batch(hexstr, 2))[::-1])

    @staticmethod
    def crc(fileName):
        prev = 0
        with open(fileName, "rb") as file:
            for eachLine in file:
                prev = zlib.crc32(eachLine, prev)
        return prev & 0xFFFFFFFF

    @staticmethod
    def batch(iterable, n=1):
        l = len(iterable)
        for ndx in range(0, l, n):
            yield iterable[ndx : min(ndx + n, l)]


if __name__ == "__main__":
    Main()()
