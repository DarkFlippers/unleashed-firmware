import logging
import json
from io import BytesIO
import tarfile
import xml.etree.ElementTree as ET
import posixpath
import os

from flipper.utils import file_sha256, timestamp
from flipper.assets.coprobin import CoproBinary, get_stack_type


CUBE_COPRO_PATH = "Projects/STM32WB_Copro_Wireless_Binaries"

MANIFEST_TEMPLATE = {
    "manifest": {"version": 0, "timestamp": 0},
    "copro": {
        "fus": {"version": {"major": 1, "minor": 2, "sub": 0}, "files": []},
        "radio": {
            "version": {},
            "files": [],
        },
    },
}


class Copro:
    COPRO_TAR_DIR = "core2_firmware"

    def __init__(self, mcu):
        self.mcu = mcu
        self.version = None
        self.cube_dir = None
        self.mcu_copro = None
        self.logger = logging.getLogger(self.__class__.__name__)

    def loadCubeInfo(self, cube_dir, reference_cube_version):
        if not os.path.isdir(cube_dir):
            raise Exception(f'"{cube_dir}" doesn\'t exists')
        self.cube_dir = cube_dir
        self.mcu_copro = os.path.join(self.cube_dir, CUBE_COPRO_PATH, self.mcu)
        if not os.path.isdir(self.mcu_copro):
            raise Exception(f'"{self.mcu_copro}" doesn\'t exists')
        cube_manifest_file = os.path.join(self.cube_dir, "package.xml")
        cube_manifest = ET.parse(cube_manifest_file)
        cube_package = cube_manifest.find("PackDescription")
        if not cube_package:
            raise Exception("Unknown Cube manifest format")
        cube_version = cube_package.get("Patch") or cube_package.get("Release")
        if not cube_version or not cube_version.startswith("FW.WB"):
            raise Exception("Incorrect Cube package or version info")
        cube_version = cube_version.replace("FW.WB.", "", 1)
        if cube_version != reference_cube_version:
            raise Exception("Unsupported cube version")
        self.version = cube_version

    def _getFileName(self, name):
        return posixpath.join(self.COPRO_TAR_DIR, name)

    def addFile(self, array, filename, **kwargs):
        source_file = os.path.join(self.mcu_copro, filename)
        self.output_tar.add(source_file, arcname=self._getFileName(filename))
        array.append({"name": filename, "sha256": file_sha256(source_file), **kwargs})

    def bundle(self, output_file, stack_file_name, stack_type, stack_addr=None):
        self.output_tar = tarfile.open(output_file, "w:gz", format=tarfile.USTAR_FORMAT)
        fw_directory = tarfile.TarInfo(self.COPRO_TAR_DIR)
        fw_directory.type = tarfile.DIRTYPE
        self.output_tar.addfile(fw_directory)

        stack_file = os.path.join(self.mcu_copro, stack_file_name)
        # Form Manifest
        manifest = dict(MANIFEST_TEMPLATE)
        manifest["manifest"]["timestamp"] = timestamp()
        copro_bin = CoproBinary(stack_file)
        self.logger.info(f"Bundling {copro_bin.img_sig.get_version()}")
        stack_type_code = get_stack_type(stack_type)
        manifest["copro"]["radio"]["version"].update(
            {
                "type": stack_type_code,
                "major": copro_bin.img_sig.version_major,
                "minor": copro_bin.img_sig.version_minor,
                "sub": copro_bin.img_sig.version_sub,
                "branch": copro_bin.img_sig.version_branch,
                "release": copro_bin.img_sig.version_build,
            }
        )
        if not stack_addr:
            stack_addr = copro_bin.get_flash_load_addr()
            self.logger.info(f"Using guessed flash address 0x{stack_addr:x}")

        # Old FUS Update
        self.addFile(
            manifest["copro"]["fus"]["files"],
            "stm32wb5x_FUS_fw_for_fus_0_5_3.bin",
            condition="==0.5.3",
            address="0x080EC000",
        )
        # New FUS Update
        self.addFile(
            manifest["copro"]["fus"]["files"],
            "stm32wb5x_FUS_fw.bin",
            condition=">0.5.3",
            address="0x080EC000",
        )
        # BLE Full Stack
        self.addFile(
            manifest["copro"]["radio"]["files"],
            stack_file_name,
            address=f"0x{stack_addr:X}",
        )

        # Save manifest
        manifest_data = json.dumps(manifest, indent=4).encode("utf-8")
        info = tarfile.TarInfo(self._getFileName("Manifest.json"))
        info.size = len(manifest_data)
        self.output_tar.addfile(info, BytesIO(manifest_data))
        self.output_tar.close()
