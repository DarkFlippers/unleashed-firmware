#!/usr/bin/env python3

from flipper.app import App
from flipper.utils.fff import FlipperFormatFile
from flipper.assets.coprobin import CoproBinary, get_stack_type
from flipper.assets.obdata import OptionBytesData
from os.path import basename, join, exists
import os
import shutil
import zlib
import tarfile
import math


class Main(App):
    UPDATE_MANIFEST_NAME = "update.fuf"

    #  No compression, plain tar
    RESOURCE_TAR_MODE = "w:"
    RESOURCE_TAR_FORMAT = tarfile.USTAR_FORMAT
    RESOURCE_FILE_NAME = "resources.tar"

    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # generate
        self.parser_generate = self.subparsers.add_parser(
            "generate", help="Generate update description file"
        )

        self.parser_generate.add_argument("-d", dest="directory", required=True)
        self.parser_generate.add_argument("-v", dest="version", required=True)
        self.parser_generate.add_argument("-t", dest="target", required=True)
        self.parser_generate.add_argument(
            "--dfu", dest="dfu", default="", required=False
        )
        self.parser_generate.add_argument("-r", dest="resources", required=False)
        self.parser_generate.add_argument("--stage", dest="stage", required=True)
        self.parser_generate.add_argument(
            "--radio", dest="radiobin", default="", required=False
        )
        self.parser_generate.add_argument(
            "--radioaddr",
            dest="radioaddr",
            type=lambda x: int(x, 16),
            default=0,
            required=False,
        )

        self.parser_generate.add_argument(
            "--radiotype", dest="radiotype", required=False
        )

        self.parser_generate.add_argument("--obdata", dest="obdata", required=False)

        self.parser_generate.set_defaults(func=self.generate)

    def generate(self):
        stage_basename = basename(self.args.stage)
        dfu_basename = basename(self.args.dfu)
        radiobin_basename = basename(self.args.radiobin)
        resources_basename = ""

        radio_version = 0
        radio_meta = None
        radio_addr = self.args.radioaddr
        if self.args.radiobin:
            if not self.args.radiotype:
                raise ValueError("Missing --radiotype")
            radio_meta = CoproBinary(self.args.radiobin)
            radio_version = self.copro_version_as_int(radio_meta, self.args.radiotype)
            if radio_addr == 0:
                radio_addr = radio_meta.get_flash_load_addr()
                self.logger.info(
                    f"Using guessed radio address 0x{radio_addr:X}, verify with Release_Notes"
                    " or specify --radioaddr"
                )

        if not exists(self.args.directory):
            os.makedirs(self.args.directory)

        shutil.copyfile(self.args.stage, join(self.args.directory, stage_basename))
        if self.args.dfu:
            shutil.copyfile(self.args.dfu, join(self.args.directory, dfu_basename))
        if radiobin_basename:
            shutil.copyfile(
                self.args.radiobin, join(self.args.directory, radiobin_basename)
            )
        if self.args.resources:
            resources_basename = self.RESOURCE_FILE_NAME
            self.package_resources(
                self.args.resources, join(self.args.directory, resources_basename)
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
        file.writeKey("Radio address", self.int2ffhex(radio_addr))
        file.writeKey("Radio version", self.int2ffhex(radio_version))
        if radiobin_basename:
            file.writeKey("Radio CRC", self.int2ffhex(self.crc(self.args.radiobin)))
        else:
            file.writeKey("Radio CRC", self.int2ffhex(0))
        file.writeKey("Resources", resources_basename)
        file.writeComment(
            "NEVER EVER MESS WITH THESE VALUES, YOU WILL BRICK YOUR DEVICE"
        )
        if self.args.obdata:
            obd = OptionBytesData(self.args.obdata)
            obvalues = obd.gen_values().export()
            file.writeKey("OB reference", self.bytes2ffhex(obvalues.reference))
            file.writeKey("OB mask", self.bytes2ffhex(obvalues.compare_mask))
            file.writeKey("OB write mask", self.bytes2ffhex(obvalues.write_mask))
        file.save(join(self.args.directory, self.UPDATE_MANIFEST_NAME))

        return 0

    def package_resources(self, srcdir: str, dst_name: str):
        with tarfile.open(
            dst_name, self.RESOURCE_TAR_MODE, format=self.RESOURCE_TAR_FORMAT
        ) as tarball:
            tarball.add(srcdir, arcname="")

    @staticmethod
    def copro_version_as_int(coprometa, stacktype):
        major = coprometa.img_sig.version_major
        minor = coprometa.img_sig.version_minor
        sub = coprometa.img_sig.version_sub
        branch = coprometa.img_sig.version_branch
        release = coprometa.img_sig.version_build
        stype = get_stack_type(stacktype)
        return (
            major
            | (minor << 8)
            | (sub << 16)
            | (branch << 24)
            | (release << 32)
            | (stype << 40)
        )

    @staticmethod
    def bytes2ffhex(value: bytes):
        return " ".join(f"{b:02X}" for b in value)

    @staticmethod
    def int2ffhex(value: int):
        n_hex_bytes = 4
        if value:
            n_hex_bytes = math.ceil(math.ceil(math.log2(value)) / 8) * 2
        fmtstr = f"%0{n_hex_bytes}X"
        hexstr = fmtstr % value
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
