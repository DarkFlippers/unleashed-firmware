#!/usr/bin/env python3

import math
import os
import shutil
import tarfile
import zlib
from os.path import exists, join

from flipper.app import App
from flipper.assets.coprobin import CoproBinary, get_stack_type
from flipper.assets.heatshrink_stream import HeatshrinkDataStreamHeader
from flipper.assets.obdata import ObReferenceValues, OptionBytesData
from flipper.assets.tarball import compress_tree_tarball, tar_sanitizer_filter
from flipper.utils.fff import FlipperFormatFile
from slideshow import Main as SlideshowMain


class Main(App):
    UPDATE_MANIFEST_VERSION = 2
    UPDATE_MANIFEST_NAME = "update.fuf"

    #  No compression, plain tar
    RESOURCE_TAR_MODE = "w:"
    RESOURCE_FILE_NAME = "resources.ths"  # .Tar.HeatShrink
    RESOURCE_ENTRY_NAME_MAX_LENGTH = 100

    WHITELISTED_STACK_TYPES = set(
        map(
            get_stack_type,
            ["BLE_FULL", "BLE_LIGHT", "BLE_BASIC"],
        )
    )

    FLASH_BASE = 0x8000000
    FLASH_PAGE_SIZE = 4 * 1024
    MIN_GAP_PAGES = 2

    # Update stage file larger than that is not loadable without fix
    # https://github.com/flipperdevices/flipperzero-firmware/pull/3676
    UPDATER_SIZE_THRESHOLD = 128 * 1024

    HEATSHRINK_WINDOW_SIZE = 13
    HEATSHRINK_LOOKAHEAD_SIZE = 6

    # Post-update slideshow
    SPLASH_BIN_NAME = "splash.bin"

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
        self.parser_generate.add_argument("--splash", dest="splash", required=False)
        self.parser_generate.add_argument(
            "--I-understand-what-I-am-doing", dest="disclaimer", required=False
        )
        self.parser_generate.add_argument(
            "--stackversion", dest="stack_version", required=False, default=""
        )

        self.parser_generate.set_defaults(func=self.generate)

    def generate(self):
        stage_basename = "updater.bin"  # used to be basename(self.args.stage)
        dfu_basename = (
            "firmware.dfu" if self.args.dfu else ""
        )  # used to be basename(self.args.dfu)
        radiobin_basename = (
            "radio.bin" if self.args.radiobin else ""
        )  # used to be basename(self.args.radiobin)
        resources_basename = ""

        radio_version = 0
        radio_meta = None
        radio_addr = self.args.radioaddr
        if self.args.radiobin:
            if not self.args.radiotype:
                raise ValueError("Missing --radiotype")
            radio_meta = CoproBinary(self.args.radiobin)
            if self.args.stack_version:
                actual_stack_version_str = f"{radio_meta.img_sig.version_major}.{radio_meta.img_sig.version_minor}.{radio_meta.img_sig.version_sub}"
                if actual_stack_version_str != self.args.stack_version:
                    self.logger.error(
                        f"Stack version mismatch: expected {self.args.stack_version}, actual {actual_stack_version_str}"
                    )
                    return 1
            radio_version = self.copro_version_as_int(radio_meta, self.args.radiotype)
            if (
                get_stack_type(self.args.radiotype) not in self.WHITELISTED_STACK_TYPES
                and self.args.disclaimer != "yes"
            ):
                self.logger.error(
                    f"You are trying to bundle a non-standard stack type '{self.args.radiotype}'."
                )
                self.show_disclaimer()
                return 1

            if radio_addr == 0:
                radio_addr = radio_meta.get_flash_load_addr()
                self.logger.info(
                    f"Using guessed radio address 0x{radio_addr:08X}, verify with Release_Notes"
                    " or specify --radioaddr"
                )

        if not exists(self.args.directory):
            os.makedirs(self.args.directory)

        updater_stage_size = os.stat(self.args.stage).st_size
        shutil.copyfile(self.args.stage, join(self.args.directory, stage_basename))

        dfu_size = 0
        if self.args.dfu:
            dfu_size = os.stat(self.args.dfu).st_size
            shutil.copyfile(self.args.dfu, join(self.args.directory, dfu_basename))
        if radiobin_basename:
            shutil.copyfile(
                self.args.radiobin, join(self.args.directory, radiobin_basename)
            )
        if self.args.resources:
            resources_basename = self.RESOURCE_FILE_NAME
            if not self.package_resources(
                self.args.resources, join(self.args.directory, resources_basename)
            ):
                return 3

        if not self.layout_check(updater_stage_size, dfu_size, radio_addr):
            self.logger.warning("Memory layout looks suspicious")
            if self.args.disclaimer != "yes":
                self.show_disclaimer()
                return 2

        if self.args.splash:
            splash_args = [
                "-i",
                self.args.splash,
                "-o",
                join(self.args.directory, self.SPLASH_BIN_NAME),
            ]
            if splash_code := SlideshowMain(no_exit=True)(splash_args):
                self.logger.error(
                    f"Failed to convert splash screen data: {splash_code}"
                )
                return splash_code

        file = FlipperFormatFile()
        file.setHeader(
            "Flipper firmware upgrade configuration", self.UPDATE_MANIFEST_VERSION
        )
        file.writeKey("Info", self.args.version)
        file.writeKey("Target", self.args.target[1:])  # dirty 'f' strip
        file.writeKey("Loader", stage_basename)
        file.writeComment("little-endian hex!")
        file.writeKey("Loader CRC", self.int2ffhex(self.crc(self.args.stage)))
        file.writeKey("Firmware", dfu_basename)
        file.writeKey("Radio", radiobin_basename or "")
        file.writeKey("Radio address", self.int2ffhex(radio_addr))
        file.writeKey("Radio version", self.int2ffhex(radio_version, 12))
        if radiobin_basename:
            file.writeKey("Radio CRC", self.int2ffhex(self.crc(self.args.radiobin)))
        else:
            file.writeKey("Radio CRC", self.int2ffhex(0))
        file.writeKey("Resources", resources_basename)
        obvalues = ObReferenceValues((), (), ())
        if self.args.obdata:
            obd = OptionBytesData(self.args.obdata)
            obvalues = obd.gen_values().export()
            file.writeComment(
                "NEVER EVER MESS WITH THESE VALUES, YOU WILL BRICK YOUR DEVICE"
            )
        file.writeKey("OB reference", self.bytes2ffhex(obvalues.reference))
        file.writeKey("OB mask", self.bytes2ffhex(obvalues.compare_mask))
        file.writeKey("OB write mask", self.bytes2ffhex(obvalues.write_mask))
        file.writeKey("Splashscreen", self.SPLASH_BIN_NAME if self.args.splash else "")
        file.save(join(self.args.directory, self.UPDATE_MANIFEST_NAME))

        return 0

    def layout_check(self, stage_size, fw_size, radio_addr):
        if stage_size > self.UPDATER_SIZE_THRESHOLD:
            self.logger.warning(
                f"Updater size {stage_size}b > {self.UPDATER_SIZE_THRESHOLD}b and is not loadable on older firmwares!"
            )

        if fw_size == 0 or radio_addr == 0:
            self.logger.info("Cannot validate layout for partial package")
            return True

        fw2stack_gap = radio_addr - self.FLASH_BASE - fw_size
        self.logger.debug(f"Expected reserved space size: {fw2stack_gap}")
        fw2stack_gap_pages = fw2stack_gap / self.FLASH_PAGE_SIZE
        if fw2stack_gap_pages < 0:
            self.logger.warning(
                f"Firmware image overlaps C2 region and is not programmable!"
            )
            return False

        elif fw2stack_gap_pages < self.MIN_GAP_PAGES:
            self.logger.warning(
                f"Expected reserved flash size is too small (~{int(fw2stack_gap_pages)} page(s), need >={self.MIN_GAP_PAGES} page(s))"
            )
            return False
        return True

    def show_disclaimer(self):
        self.logger.error(
            "You might brick your device into a state in which you'd need an SWD programmer to fix it."
        )
        self.logger.error(
            "Please confirm that you REALLY want to do that with --I-understand-what-I-am-doing=yes"
        )

    def _tar_filter(self, tarinfo: tarfile.TarInfo):
        if len(tarinfo.name) > self.RESOURCE_ENTRY_NAME_MAX_LENGTH:
            self.logger.error(
                f"Cannot package resource: name '{tarinfo.name}' too long"
            )
            raise ValueError("Resource name too long")
        return tar_sanitizer_filter(tarinfo)

    def package_resources(self, srcdir: str, dst_name: str):
        try:
            src_size, compressed_size = compress_tree_tarball(
                srcdir, dst_name, filter=self._tar_filter
            )

            self.logger.info(
                f"Resources compression ratio: {compressed_size * 100 / src_size:.2f}%"
            )
            return True
        except Exception as e:
            self.logger.error(f"Cannot package resources: {e}")
            return False

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
    def int2ffhex(value: int, n_hex_syms=8):
        if value:
            n_hex_syms = max(math.ceil(math.ceil(math.log2(value)) / 8) * 2, n_hex_syms)
        fmtstr = f"%0{n_hex_syms}X"
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
        iterable_len = len(iterable)
        for ndx in range(0, iterable_len, n):
            yield iterable[ndx : min(ndx + n, iterable_len)]


if __name__ == "__main__":
    Main()()
