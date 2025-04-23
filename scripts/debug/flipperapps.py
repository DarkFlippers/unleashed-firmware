from dataclasses import dataclass
from typing import Optional, Tuple, Dict, ClassVar
import struct
import posixpath
import zlib

import gdb


class bcolors:
    HEADER = "\033[95m"
    OKBLUE = "\033[94m"
    OKCYAN = "\033[96m"
    OKGREEN = "\033[92m"
    WARNING = "\033[93m"
    FAIL = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"


LOG_PREFIX = "[FURI]"


def error(line):
    print(f"{bcolors.FAIL}{LOG_PREFIX} {line}{bcolors.ENDC}")


def warning(line):
    print(f"{bcolors.WARNING}{LOG_PREFIX} {line}{bcolors.ENDC}")


def info(line):
    print(f"{bcolors.OKGREEN}{LOG_PREFIX} {line}{bcolors.ENDC}")


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

    DEBUG_ELF_ROOT: ClassVar[Optional[str]] = None

    def __post_init__(self):
        if self.other_sections is None:
            self.other_sections = {}

    def get_original_elf_path(self) -> str:
        if self.DEBUG_ELF_ROOT is None:
            raise ValueError("DEBUG_ELF_ROOT not set; call fap-set-debug-elf-root")
        return (
            posixpath.join(self.DEBUG_ELF_ROOT, self.debug_link_elf)
            if self.DEBUG_ELF_ROOT
            else self.debug_link_elf
        )

    def is_debug_available(self) -> bool:
        have_debug_info = bool(self.debug_link_elf and self.debug_link_crc)
        if not have_debug_info:
            warning("No debug info available for this app")
            return False
        debug_elf_path = self.get_original_elf_path()
        debug_elf_crc32 = get_file_crc32(debug_elf_path)
        if self.debug_link_crc != debug_elf_crc32:
            warning(
                f"Debug info ({debug_elf_path}) CRC mismatch: {self.debug_link_crc:08x} != {debug_elf_crc32:08x}, rebuild app"
            )
            return False
        return True

    def get_gdb_load_command(self) -> str:
        load_path = self.get_original_elf_path()
        info(f"Loading debug information from {load_path}")
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

    @staticmethod
    def get_gdb_app_ep(app) -> int:
        return int(app["state"]["entry"])

    @staticmethod
    def parse_debug_link_data(section_data: bytes) -> Tuple[str, int]:
        # Debug link format: a null-terminated string with debuggable file name
        # Padded with 0's to multiple of 4 bytes
        # Followed by 4 bytes of CRC32 checksum of that file
        elf_name = section_data[:-4].decode("utf-8").split("\x00")[0]
        crc32 = struct.unpack("<I", section_data[-4:])[0]
        return (elf_name, crc32)

    @classmethod
    def from_gdb(cls, gdb_app: "AppState") -> "AppState":
        state = AppState(str(gdb_app["manifest"]["name"].string()))
        state.entry_address = cls.get_gdb_app_ep(gdb_app)

        app_state = gdb_app["state"]
        if debug_link_size := int(app_state["debug_link_info"]["debug_link_size"]):
            debug_link_data = (
                gdb.selected_inferior()
                .read_memory(
                    int(app_state["debug_link_info"]["debug_link"]), debug_link_size
                )
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


class SetFapDebugElfRoot(gdb.Command):
    """Set path to original ELF files for debug info"""

    def __init__(self):
        super().__init__(
            "fap-set-debug-elf-root", gdb.COMMAND_FILES, gdb.COMPLETE_FILENAME
        )
        self.dont_repeat()

    def invoke(self, arg, from_tty):
        AppState.DEBUG_ELF_ROOT = arg
        try:
            global helper
            info(f"Set '{arg}' as debug info lookup path for Flipper external apps")
            helper.attach_to_fw()
            gdb.events.stop.connect(helper.handle_stop)
            gdb.events.gdb_exiting.connect(helper.handle_exit)
        except gdb.error as e:
            error(f"Support for Flipper external apps debug is not available: {e}")


class FlipperAppStateHelper:
    def __init__(self):
        self.app_type_ptr = None
        self.app_list_ptr = None
        self.app_list_entry_type = None
        self._current_apps: list[AppState] = []
        self.set_debug_mode(True)

    def _walk_app_list(self, list_head):
        while list_head:
            if app := list_head["data"]:
                yield app.dereference()
            list_head = list_head["next"]

    def _exec_gdb_command(self, command: str) -> bool:
        try:
            gdb.execute(command)
            return True
        except gdb.error as e:
            error(f"Failed to execute GDB command '{command}': {e}")
            return False

    def _get_crash_message(self):
        message = self.app_check_message.value()
        if message == 1:
            return "furi_assert failed"
        elif message == 2:
            return "furi_check failed"
        else:
            return message

    def _sync_apps(self) -> None:
        crash_message = self._get_crash_message()
        if crash_message:
            crash_message = f"! System crashed: {crash_message} !"
            error("!" * len(crash_message))
            error(crash_message)
            error("!" * len(crash_message))

        self.set_debug_mode(True)
        if not (app_list := self.app_list_ptr.value()):
            info("Reset app loader state")
            for app in self._current_apps:
                self._exec_gdb_command(app.get_gdb_unload_command())
            self._current_apps = []
            return

        loaded_apps: dict[int, gdb.Value] = dict(
            (AppState.get_gdb_app_ep(app), app)
            for app in self._walk_app_list(app_list[0])
        )

        for app in self._current_apps.copy():
            if app.entry_address not in loaded_apps:
                warning(f"Application {app.name} is no longer loaded")
                if not self._exec_gdb_command(app.get_gdb_unload_command()):
                    error(f"Failed to unload debug info for {app.name}")
                self._current_apps.remove(app)

        for entry_point, app in loaded_apps.items():
            if entry_point not in set(app.entry_address for app in self._current_apps):
                new_app_state = AppState.from_gdb(app)
                warning(f"New application loaded. Adding debug info")
                if self._exec_gdb_command(new_app_state.get_gdb_load_command()):
                    self._current_apps.append(new_app_state)
                else:
                    error(f"Failed to load debug info for {new_app_state}")

    def attach_to_fw(self) -> None:
        info("Attaching to Flipper firmware")
        self.app_check_message = gdb.lookup_global_symbol("__furi_check_message")
        self.app_list_ptr = gdb.lookup_global_symbol(
            "flipper_application_loaded_app_list"
        )
        self.app_type_ptr = gdb.lookup_type("FlipperApplication").pointer()
        self.app_list_entry_type = gdb.lookup_type("struct FlipperApplicationList_s")
        self._sync_apps()

    def handle_stop(self, event) -> None:
        self._sync_apps()

    def handle_exit(self, event) -> None:
        self.set_debug_mode(False)

    def set_debug_mode(self, mode: bool) -> None:
        try:
            gdb.execute(f"set variable furi_hal_debug_gdb_session_active = {int(mode)}")
        except gdb.error as e:
            error(f"Failed to set debug mode: {e}")


# Init additional 'fap-set-debug-elf-root' command and set up hooks
SetFapDebugElfRoot()
helper = FlipperAppStateHelper()
info("Support for Flipper external apps debug is loaded")
