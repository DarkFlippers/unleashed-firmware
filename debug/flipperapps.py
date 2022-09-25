from dataclasses import dataclass
from typing import Tuple, Dict
import struct
import posixpath
import os
import zlib

import gdb


def get_file_crc32(filename):
    with open(filename, "rb") as f:
        return zlib.crc32(f.read())


@dataclass
class AppState:
    name: str
    text_address: int = 0
    entry_address: int = 0
    other_sections: Dict[str, int] = None
    debug_link_elf: str = ""
    debug_link_crc: int = 0

    def __post_init__(self):
        if self.other_sections is None:
            self.other_sections = {}

    def get_original_elf_path(self, elf_path="build/latest/.extapps") -> str:
        return (
            posixpath.join(elf_path, self.debug_link_elf)
            if elf_path
            else self.debug_link_elf
        )

    def is_debug_available(self) -> bool:
        have_debug_info = bool(self.debug_link_elf and self.debug_link_crc)
        if not have_debug_info:
            print("No debug info available for this app")
            return False
        debug_elf_path = self.get_original_elf_path()
        debug_elf_crc32 = get_file_crc32(debug_elf_path)
        if self.debug_link_crc != debug_elf_crc32:
            print(
                f"Debug info ({debug_elf_path}) CRC mismatch: {self.debug_link_crc:08x} != {debug_elf_crc32:08x}, rebuild app"
            )
            return False
        return True

    def get_gdb_load_command(self) -> str:
        load_path = self.get_original_elf_path()
        print(f"Loading debug information from {load_path}")
        load_command = (
            f"add-symbol-file -readnow {load_path} 0x{self.text_address:08x} "
        )
        load_command += " ".join(
            f"-s {name} 0x{address:08x}"
            for name, address in self.other_sections.items()
        )
        return load_command

    def get_gdb_unload_command(self) -> str:
        return f"remove-symbol-file -a 0x{self.text_address:08x}"

    def is_loaded_in_gdb(self, gdb_app) -> bool:
        # Avoid constructing full app wrapper for comparison
        return self.entry_address == int(gdb_app["state"]["entry"])

    @staticmethod
    def parse_debug_link_data(section_data: bytes) -> Tuple[str, int]:
        # Debug link format: a null-terminated string with debuggable file name
        # Padded with 0's to multiple of 4 bytes
        # Followed by 4 bytes of CRC32 checksum of that file
        elf_name = section_data[:-4].decode("utf-8").split("\x00")[0]
        crc32 = struct.unpack("<I", section_data[-4:])[0]
        return (elf_name, crc32)

    @staticmethod
    def from_gdb(gdb_app: "AppState") -> "AppState":
        state = AppState(str(gdb_app["manifest"]["name"].string()))
        state.entry_address = int(gdb_app["state"]["entry"])

        app_state = gdb_app["state"]
        if debug_link_size := int(app_state["debug_link_info"]["debug_link_size"]):
            debug_link_data = (
                gdb.selected_inferior()
                .read_memory(int(app_state["debug_link_info"]["debug_link"]), debug_link_size)
                .tobytes()
            )
            state.debug_link_elf, state.debug_link_crc = AppState.parse_debug_link_data(
                debug_link_data
            )

        for idx in range(app_state["mmap_entry_count"]):
            mmap_entry = app_state["mmap_entries"][idx]
            section_name = mmap_entry["name"].string()
            section_addr = int(mmap_entry["address"])
            if section_name == ".text":
                state.text_address = section_addr
            else:
                state.other_sections[section_name] = section_addr

        return state


class FlipperAppDebugHelper:
    def __init__(self):
        self.app_ptr = None
        self.app_type_ptr = None
        self.current_app: AppState = None

    def attach_fw(self) -> None:
        self.app_ptr = gdb.lookup_global_symbol("last_loaded_app")
        self.app_type_ptr = gdb.lookup_type("FlipperApplication").pointer()
        self._check_app_state()

    def _check_app_state(self) -> None:
        app_ptr_value = self.app_ptr.value()
        if not app_ptr_value and self.current_app:
            # There is an ELF loaded in GDB, but nothing is running on the device
            self._unload_debug_elf()
        elif app_ptr_value:
            # There is an app running on the device
            loaded_app = app_ptr_value.cast(self.app_type_ptr).dereference()

            if self.current_app and not self.current_app.is_loaded_in_gdb(loaded_app):
                # Currently loaded ELF is not the one running on the device
                self._unload_debug_elf()

            if not self.current_app:
                # Load ELF for the app running on the device
                self._load_debug_elf(loaded_app)

    def _unload_debug_elf(self) -> None:
        try:
            gdb.execute(self.current_app.get_gdb_unload_command())
        except gdb.error as e:
            print(f"Failed to unload debug ELF: {e} (might not be an error)")
        self.current_app = None

    def _load_debug_elf(self, app_object) -> None:
        self.current_app = AppState.from_gdb(app_object)

        if self.current_app.is_debug_available():
            gdb.execute(self.current_app.get_gdb_load_command())

    def handle_stop(self, event) -> None:
        self._check_app_state()


helper = FlipperAppDebugHelper()
try:
    helper.attach_fw()
    print("Support for Flipper external apps debug is enabled")
    gdb.events.stop.connect(helper.handle_stop)
except gdb.error as e:
    print(f"Support for Flipper external apps debug is not available: {e}")
