#!/usr/bin/env python3
import hashlib
import os
import struct
import subprocess
import tempfile
from collections import defaultdict
from dataclasses import dataclass

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
from elftools.elf.sections import SymbolTableSection
from fbt.sdk.hashes import gnu_sym_hash
from flipper.app import App

VERSION = 1


@dataclass
class RelData:
    section: int
    section_value: int
    type: int
    offset: int
    name: str


@dataclass(frozen=True)
class UniqueRelData:
    section: int
    section_value: int
    type: int
    name: str


@dataclass
class RelSection:
    name: str
    oringinal_name: str
    data: dict[UniqueRelData, list[int]]


def serialize_relsection_data(data: dict[UniqueRelData, list[int]]) -> bytes:
    result = struct.pack("<B", VERSION)
    result += struct.pack("<I", len(data))
    for unique, values in data.items():
        if unique.section > 0:
            result += struct.pack("<B", (1 << 7) | unique.type & 0x7F)
            result += struct.pack("<I", unique.section)
            result += struct.pack("<I", unique.section_value)
        else:
            result += struct.pack("<B", (0 << 7) | unique.type & 0x7F)
            result += struct.pack("<I", gnu_sym_hash(unique.name))

        result += struct.pack("<I", len(values))
        for offset in values:
            result += struct.pack(
                "<BBB", offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF
            )

    return result


class Main(App):
    def init(self):
        self.parser.add_argument("fap_src_path", help="App file to upload")
        self.parser.add_argument("objcopy_path", help="Objcopy path")
        self.parser.set_defaults(func=self.process)

    def process(self):
        fap_path = self.args.fap_src_path
        objcopy_path = self.args.objcopy_path

        sections: list[RelSection] = []

        with open(fap_path, "rb") as f:
            elf_file = ELFFile(f)

            relocation_sections: list[RelocationSection] = []
            symtab_section: SymbolTableSection | None = None

            for section in elf_file.iter_sections():
                if isinstance(section, RelocationSection):
                    relocation_sections.append(section)

                if isinstance(section, SymbolTableSection):
                    symtab_section = section

            if not symtab_section:
                self.logger.error("No symbol table found")
                return 1

            if not relocation_sections:
                self.logger.info("No relocation sections found")
                return 0

            for section in relocation_sections:
                section_relocations: list[RelData] = []

                for relocation in section.iter_relocations():
                    symbol_id: int = relocation.entry["r_info_sym"]
                    offset: int = relocation.entry["r_offset"]
                    type: int = relocation.entry["r_info_type"]
                    symbol = symtab_section.get_symbol(symbol_id)
                    section_index: int = symbol["st_shndx"]
                    section_value: int = symbol["st_value"]
                    if section_index == "SHN_UNDEF":
                        section_index = 0

                    section_relocations.append(
                        RelData(section_index, section_value, type, offset, symbol.name)
                    )

                unique_relocations: dict[UniqueRelData, list[int]] = defaultdict(list)
                for relocation in section_relocations:
                    unique = UniqueRelData(
                        relocation.section,
                        relocation.section_value,
                        relocation.type,
                        relocation.name,
                    )

                    unique_relocations[unique].append(relocation.offset)

                section_name = section.name
                if section_name.startswith(".rel"):
                    section_name = ".fast.rel" + section_name[4:]
                else:
                    self.logger.error(
                        "Unknown relocation section name: %s", section_name
                    )
                    return 1

                sections.append(
                    RelSection(section_name, section.name, unique_relocations)
                )

        with tempfile.TemporaryDirectory() as temp_dir:
            for section in sections:
                data = serialize_relsection_data(section.data)
                hash_name = hashlib.md5(section.name.encode()).hexdigest()
                filename = f"{temp_dir}/{hash_name}.bin"

                if os.path.isfile(filename):
                    self.logger.error(f"File {filename} already exists")
                    return 1

                with open(filename, "wb") as f:
                    f.write(data)

                exit_code = subprocess.run(
                    [
                        objcopy_path,
                        "--add-section",
                        f"{section.name}={filename}",
                        fap_path,
                    ],
                    check=True,
                )

                if exit_code.returncode != 0:
                    self.logger.error("objcopy failed")
                    return 1

        return 0


if __name__ == "__main__":
    Main()()
