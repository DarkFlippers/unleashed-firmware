from dataclasses import dataclass, field
from typing import Dict, Optional

import gdb


# Must match FuriHalRtcRegisterVersion index in FuriHalRtcRegister enum
RTC_BACKUP_VERSION_REGISTER_IDX = 0x2

RTC_BASE = 0x40002800
RTC_BACKUP_BASE = RTC_BASE + 0x50

VERSION_REGISTER_ADDRESS = RTC_BACKUP_BASE + RTC_BACKUP_VERSION_REGISTER_IDX * 4

VERSION_STRUCT_MAGIC = 0xBE40


@dataclass
class VersionData:
    git_hash: str
    git_branch: str
    build_date: str
    version: str
    target: int
    build_is_dirty: bool
    # Since version 1.1
    firmware_origin: str = ""
    git_origin: str = ""
    # More fields may be added in the future
    extra: Optional[Dict[str, str]] = field(default_factory=dict)


class VersionLoader:
    def __init__(self, version_ptr):
        self.version_ptr = version_ptr
        self._cstr_type = gdb.lookup_type("char").pointer()
        self._uint_type = gdb.lookup_type("unsigned int")

        version_signature = version_ptr.dereference().cast(self._uint_type)
        is_versioned = (version_signature & (0xFFFF)) == VERSION_STRUCT_MAGIC
        if is_versioned:
            self._version_data = self.load_versioned(
                major=version_signature >> 16 & 0xFF,
                minor=version_signature >> 24 & 0xFF,
            )
        else:
            self._version_data = self.load_unversioned()

    @property
    def version(self) -> VersionData:
        return self._version_data

    def load_versioned(self, major, minor):
        if major != 1:
            raise ValueError("Unsupported version struct major version")

        # Struct version 1.0
        extra_data = int(self.version_ptr[5].cast(self._uint_type))
        version_data = VersionData(
            git_hash=self.version_ptr[1].cast(self._cstr_type).string(),
            git_branch=self.version_ptr[2].cast(self._cstr_type).string(),
            build_date=self.version_ptr[3].cast(self._cstr_type).string(),
            version=self.version_ptr[4].cast(self._cstr_type).string(),
            target=extra_data & 0xF,
            build_is_dirty=bool((extra_data >> 8) & 0xF),
        )
        if minor >= 1:
            version_data.firmware_origin = (
                self.version_ptr[6].cast(self._cstr_type).string()
            )
            version_data.git_origin = self.version_ptr[7].cast(self._cstr_type).string()
        return version_data

    def load_unversioned(self):
        """Parse an early version of the version struct."""
        extra_data = int(self.version_ptr[5].cast(self._uint_type))
        return VersionData(
            git_hash=self.version_ptr[0].cast(self._cstr_type).string(),
            git_branch=self.version_ptr[1].cast(self._cstr_type).string(),
            # branch number is #2, but we don't care about it
            build_date=self.version_ptr[3].cast(self._cstr_type).string(),
            version=self.version_ptr[4].cast(self._cstr_type).string(),
            target=extra_data & 0xF,
            build_is_dirty=bool((extra_data >> 8) & 0xF),
        )


class FlipperFwVersion(gdb.Command):
    """Print the version of Flipper's firmware."""

    def __init__(self):
        super(FlipperFwVersion, self).__init__("fw-version", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        void_ptr_type = gdb.lookup_type("void").pointer().pointer()
        version_ptr_ptr = gdb.Value(VERSION_REGISTER_ADDRESS).cast(void_ptr_type)

        if not version_ptr_ptr:
            print("RTC version register is NULL")
            return

        version_ptr = version_ptr_ptr.dereference()
        if not version_ptr:
            print("Pointer to version struct is NULL")
            return

        version_struct = version_ptr.cast(void_ptr_type)

        v = VersionLoader(version_struct)
        print("Firmware version on attached Flipper:")
        print(f"\tVersion:     {v.version.version}")
        print(f"\tBuilt on:    {v.version.build_date}")
        print(f"\tGit branch:  {v.version.git_branch}")
        print(f"\tGit commit:  {v.version.git_hash}")
        print(f"\tDirty:       {v.version.build_is_dirty}")
        print(f"\tHW Target:   {v.version.target}")
        if v.version.firmware_origin:
            print(f"\tOrigin:      {v.version.firmware_origin}")
        if v.version.git_origin:
            print(f"\tGit origin:  {v.version.git_origin}")


FlipperFwVersion()
