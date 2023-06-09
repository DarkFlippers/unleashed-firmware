#!/usr/bin/env python3

# Requiremets:
#   cxxfilt==0.3.0

# Most part of this code written by Lars-Dominik Braun <lars@6xq.net> https://github.com/PromyLOPh/linkermapviz
# and distributes under MIT licence

# Copyright (c) 2017 Lars-Dominik Braun <lars@6xq.net>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import sys
import re
import os
from typing import TextIO
from cxxfilt import demangle


class Objectfile:
    def __init__(self, section: str, offset: int, size: int, comment: str):
        self.section = section.strip()
        self.offset = offset
        self.size = size
        self.path = (None, None)
        self.basepath = None

        if comment:
            self.path = re.match(r"^(.+?)(?:\(([^\)]+)\))?$", comment).groups()
            self.basepath = os.path.basename(self.path[0])

        self.children = []

    def __repr__(self) -> str:
        return f"<Objectfile {self.section} {self.offset:x} {self.size:x} {self.path} {repr(self.children)}>"


def update_children_size(children: list[list], subsection_size: int) -> list:
    # set subsection size to an only child
    if len(children) == 1:
        children[0][1] = subsection_size
        return children

    rest_size = subsection_size

    for index in range(1, len(children)):
        if rest_size > 0:
            # current size = current address - previous child address
            child_size = children[index][0] - children[index - 1][0]
            rest_size -= child_size
            children[index - 1][1] = child_size

    # if there is rest size, set it to the last child element
    if rest_size > 0:
        children[-1][1] = rest_size

    return children


def parse_sections(file_name: str) -> list:
    """
    Quick&Dirty parsing for GNU ld’s linker map output, needs LANG=C, because
    some messages are localized.
    """

    sections = []
    with open(file_name, "r") as file:
        # skip until memory map is found
        found = False

        while True:
            line = file.readline()
            if not line:
                break
            if line.strip() == "Memory Configuration":
                found = True
                break

        if not found:
            raise Exception(f"Memory configuration is not found in the{input_file}")

        # long section names result in a linebreak afterwards
        sectionre = re.compile(
            "(?P<section>.+?|.{14,}\n)[ ]+0x(?P<offset>[0-9a-f]+)[ ]+0x(?P<size>[0-9a-f]+)(?:[ ]+(?P<comment>.+))?\n+",
            re.I,
        )
        subsectionre = re.compile(
            "[ ]{16}0x(?P<offset>[0-9a-f]+)[ ]+(?P<function>.+)\n+", re.I
        )
        s = file.read()
        pos = 0

        while True:
            m = sectionre.match(s, pos)
            if not m:
                # skip that line
                try:
                    nextpos = s.index("\n", pos) + 1
                    pos = nextpos
                    continue
                except ValueError:
                    break

            pos = m.end()
            section = m.group("section")
            v = m.group("offset")
            offset = int(v, 16) if v is not None else None
            v = m.group("size")
            size = int(v, 16) if v is not None else None
            comment = m.group("comment")

            if section != "*default*" and size > 0:
                of = Objectfile(section, offset, size, comment)

                if section.startswith(" "):
                    children = []
                    sections[-1].children.append(of)

                    while True:
                        m = subsectionre.match(s, pos)
                        if not m:
                            break
                        pos = m.end()
                        offset, function = m.groups()
                        offset = int(offset, 16)
                        if sections and sections[-1].children:
                            children.append([offset, 0, function])

                    if children:
                        children = update_children_size(
                            children=children, subsection_size=of.size
                        )

                    sections[-1].children[-1].children.extend(children)

                else:
                    sections.append(of)

        return sections


def get_subsection_name(section_name: str, subsection: Objectfile) -> str:
    subsection_split_names = subsection.section.split(".")
    if subsection.section.startswith("."):
        subsection_split_names = subsection_split_names[1:]

    return (
        f".{subsection_split_names[1]}"
        if len(subsection_split_names) > 2
        else section_name
    )


def write_subsection(
    section_name: str,
    subsection_name: str,
    address: str,
    size: int,
    demangled_name: str,
    module_name: str,
    file_name: str,
    mangled_name: str,
    write_file_object: TextIO,
) -> None:
    write_file_object.write(
        f"{section_name}\t"
        f"{subsection_name}\t"
        f"{address}\t"
        f"{size}\t"
        f"{demangled_name}\t"
        f"{module_name}\t"
        f"{file_name}\t"
        f"{mangled_name}\n"
    )


def save_subsection(
    section_name: str, subsection: Objectfile, write_file_object: TextIO
) -> None:
    subsection_name = get_subsection_name(section_name, subsection)
    module_name = subsection.path[0]
    file_name = subsection.path[1]

    if not file_name:
        file_name, module_name = module_name, ""

    if not subsection.children:
        address = f"{subsection.offset:x}"
        size = subsection.size
        mangled_name = (
            ""
            if subsection.section == section_name
            else subsection.section.split(".")[-1]
        )
        demangled_name = demangle(mangled_name) if mangled_name else mangled_name

        write_subsection(
            section_name=section_name,
            subsection_name=subsection_name,
            address=address,
            size=size,
            demangled_name=demangled_name,
            module_name=module_name,
            file_name=file_name,
            mangled_name=mangled_name,
            write_file_object=write_file_object,
        )
        return

    for subsection_child in subsection.children:
        address = f"{subsection_child[0]:x}"
        size = subsection_child[1]
        mangled_name = subsection_child[2]
        demangled_name = demangle(mangled_name)

        write_subsection(
            section_name=section_name,
            subsection_name=subsection_name,
            address=address,
            size=size,
            demangled_name=demangled_name,
            module_name=module_name,
            file_name=file_name,
            mangled_name=mangled_name,
            write_file_object=write_file_object,
        )


def save_section(section: Objectfile, write_file_object: TextIO) -> None:
    section_name = section.section
    for subsection in section.children:
        save_subsection(
            section_name=section_name,
            subsection=subsection,
            write_file_object=write_file_object,
        )


def save_parsed_data(parsed_data: list[Objectfile], output_file_name: str) -> None:
    with open(output_file_name, "w") as write_file_object:
        for section in parsed_data:
            if section.children:
                save_section(section=section, write_file_object=write_file_object)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        raise Exception(f"Usage: {sys.argv[0]} <input file> <output file>")

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    parsed_sections = parse_sections(input_file)

    if parsed_sections is None:
        raise Exception(f"Memory configuration is not {input_file}")

    save_parsed_data(parsed_sections, output_file)
