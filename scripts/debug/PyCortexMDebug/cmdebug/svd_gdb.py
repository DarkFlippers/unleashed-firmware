#!/usr/bin/env python
"""
This file is part of PyCortexMDebug

PyCortexMDebug is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PyCortexMDebug is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PyCortexMDebug.  If not, see <http://www.gnu.org/licenses/>.
"""

import gdb
import re
import math
import sys
import struct
import pkg_resources
import fnmatch
import traceback

from .svd import SVDFile

BITS_TO_UNPACK_FORMAT = {
    8: "B",
    16: "H",
    32: "I",
}


class LoadSVD(gdb.Command):
    """A command to load an SVD file and to create the command for inspecting
    that object
    """

    def __init__(self):
        self.vendors = {}
        try:
            vendor_names = pkg_resources.resource_listdir("cmsis_svd", "data")
            for vendor in vendor_names:
                fnames = pkg_resources.resource_listdir(
                    "cmsis_svd", "data/{}".format(vendor)
                )
                self.vendors[vendor] = [
                    fname for fname in fnames if fname.lower().endswith(".svd")
                ]
        except:
            pass

        if len(self.vendors) > 0:
            gdb.Command.__init__(self, "svd_load", gdb.COMMAND_USER)
        else:
            gdb.Command.__init__(
                self, "svd_load", gdb.COMMAND_DATA, gdb.COMPLETE_FILENAME
            )

    def complete(self, text, word):
        args = gdb.string_to_argv(text)
        num_args = len(args)
        if text.endswith(" "):
            num_args += 1
        if not text:
            num_args = 1

        # "svd_load <tab>" or "svd_load ST<tab>"
        if num_args == 1:
            prefix = word.lower()
            return [
                vendor for vendor in self.vendors if vendor.lower().startswith(prefix)
            ]
        # "svd_load STMicro<tab>" or "svd_load STMicro STM32F1<tab>"
        elif num_args == 2 and args[0] in self.vendors:
            prefix = word.lower()
            filenames = self.vendors[args[0]]
            return [fname for fname in filenames if fname.lower().startswith(prefix)]
        return gdb.COMPLETE_NONE

    @staticmethod
    def invoke(args, from_tty):
        args = gdb.string_to_argv(args)
        argc = len(args)
        if argc == 1:
            gdb.write("Loading SVD file {}...\n".format(args[0]))
            f = args[0]
        elif argc == 2:
            gdb.write("Loading SVD file {}/{}...\n".format(args[0], args[1]))
            f = pkg_resources.resource_filename(
                "cmsis_svd", "data/{}/{}".format(args[0], args[1])
            )
        else:
            raise gdb.GdbError(
                "Usage: svd_load <vendor> <device.svd> or svd_load <path/to/filename.svd>\n"
            )
        try:
            SVD(SVDFile(f))
        except Exception as e:
            traceback.print_exc()
            raise gdb.GdbError("Could not load SVD file {} : {}...\n".format(f, e))


class SVD(gdb.Command):
    """The CMSIS SVD (System View Description) inspector command

    This allows easy access to all peripheral registers supported by the system
    in the GDB debug environment
    """

    def __init__(self, svd_file):
        gdb.Command.__init__(self, "svd", gdb.COMMAND_DATA)
        self.svd_file = svd_file

    def _print_registers(self, container_name, form, registers):
        if len(registers) == 0:
            return
        try:
            regs_iter = registers.itervalues()
        except AttributeError:
            regs_iter = registers.values()
        gdb.write("Registers in %s:\n" % container_name)
        reg_list = []
        for r in regs_iter:
            if r.readable():
                try:
                    data = self.read(r.address(), r.size)
                    data = self.format(data, form, r.size)
                    if form == "a":
                        data += (
                            " <"
                            + re.sub(
                                r"\s+",
                                " ",
                                gdb.execute(
                                    "info symbol {}".format(data), True, True
                                ).strip(),
                            )
                            + ">"
                        )
                except gdb.MemoryError:
                    data = "(error reading)"
            else:
                data = "(not readable)"
            desc = re.sub(r"\s+", " ", r.description)
            reg_list.append((r.name, data, desc))

        column1_width = max(len(reg[0]) for reg in reg_list) + 2  # padding
        column2_width = max(len(reg[1]) for reg in reg_list)
        for reg in reg_list:
            gdb.write(
                "\t{}:{}{}".format(
                    reg[0],
                    "".ljust(column1_width - len(reg[0])),
                    reg[1].rjust(column2_width),
                )
            )
            if reg[2] != reg[0]:
                gdb.write("  {}".format(reg[2]))
            gdb.write("\n")

    def _print_register_fields(self, container_name, form, register):
        gdb.write("Fields in {}:\n".format(container_name))
        fields = register.fields
        if not register.readable():
            data = 0
        else:
            data = self.read(register.address(), register.size)
        field_list = []
        try:
            fields_iter = fields.itervalues()
        except AttributeError:
            fields_iter = fields.values()
        for f in fields_iter:
            desc = re.sub(r"\s+", " ", f.description)
            if register.readable():
                val = data >> f.offset
                val &= (1 << f.width) - 1
                if f.enum:
                    if val in f.enum:
                        desc = f.enum[val][1] + " - " + desc
                        val = f.enum[val][0]
                    else:
                        val = "Invalid enum value: " + self.format(val, form, f.width)
                else:
                    val = self.format(val, form, f.width)
            else:
                val = "(not readable)"
            field_list.append((f.name, val, desc))

        column1_width = max(len(field[0]) for field in field_list) + 2  # padding
        column2_width = max(len(field[1]) for field in field_list)  # padding
        for field in field_list:
            gdb.write(
                "\t{}:{}{}".format(
                    field[0],
                    "".ljust(column1_width - len(field[0])),
                    field[1].rjust(column2_width),
                )
            )
            if field[2] != field[0]:
                gdb.write("  {}".format(field[2]))
            gdb.write("\n")

    def invoke(self, args, from_tty):
        s = str(args).split(" ")
        form = ""
        if s[0] and s[0][0] == "/":
            if len(s[0]) == 1:
                gdb.write("Incorrect format\n")
                return
            else:
                form = s[0][1:]
                if len(s) == 1:
                    return
                s = s[1:]

        if s[0].lower() == "help":
            gdb.write("Usage:\n")
            gdb.write("=========\n")
            gdb.write("svd:\n")
            gdb.write("\tList available peripherals\n")
            gdb.write("svd [peripheral_name]:\n")
            gdb.write("\tDisplay all registers pertaining to that peripheral\n")
            gdb.write("svd [peripheral_name] [register_name]:\n")
            gdb.write("\tDisplay the fields in that register\n")
            gdb.write("svd/[format_character] ...\n")
            gdb.write("\tFormat values using that character\n")
            gdb.write("\td(default):decimal, x: hex, o: octal, b: binary\n")
            gdb.write("\n")
            gdb.write(
                "Both prefix matching and case-insensitive matching is supported for peripherals, registers, clusters and fields.\n"
            )
            return

        if not len(s[0]):
            gdb.write("Available Peripherals:\n")
            try:
                peripherals = self.svd_file.peripherals.itervalues()
            except AttributeError:
                peripherals = self.svd_file.peripherals.values()
            column_width = max(len(p.name) for p in peripherals) + 2  # padding
            try:
                peripherals = self.svd_file.peripherals.itervalues()
            except AttributeError:
                peripherals = self.svd_file.peripherals.values()
            for p in peripherals:
                desc = re.sub(r"\s+", " ", p.description)
                gdb.write(
                    "\t{}:{}{}\n".format(
                        p.name, "".ljust(column_width - len(p.name)), desc
                    )
                )
            return

        def warn_if_ambiguous(smart_dict, key):
            if smart_dict.is_ambiguous(key):
                gdb.write(
                    "Warning: {} could prefix match any of: {}\n".format(
                        key, ", ".join(smart_dict.prefix_match_iter(key))
                    )
                )

        registers = None
        if len(s) >= 1:
            peripheral_name = s[0]
            if peripheral_name not in self.svd_file.peripherals:
                gdb.write("Peripheral {} does not exist!\n".format(s[0]))
                return

            warn_if_ambiguous(self.svd_file.peripherals, peripheral_name)

            peripheral = self.svd_file.peripherals[peripheral_name]

        if len(s) == 1:
            self._print_registers(peripheral.name, form, peripheral.registers)
            if len(peripheral.clusters) > 0:
                try:
                    clusters_iter = peripheral.clusters.itervalues()
                except AttributeError:
                    clusters_iter = peripheral.clusters.values()
                gdb.write("Clusters in %s:\n" % peripheral.name)
                reg_list = []
                for r in clusters_iter:
                    desc = re.sub(r"\s+", " ", r.description)
                    reg_list.append((r.name, "", desc))

                column1_width = max(len(reg[0]) for reg in reg_list) + 2  # padding
                column2_width = max(len(reg[1]) for reg in reg_list)
                for reg in reg_list:
                    gdb.write(
                        "\t{}:{}{}".format(
                            reg[0],
                            "".ljust(column1_width - len(reg[0])),
                            reg[1].rjust(column2_width),
                        )
                    )
                    if reg[2] != reg[0]:
                        gdb.write("  {}".format(reg[2]))
                    gdb.write("\n")
            return

        cluster = None
        if len(s) == 2:
            if s[1] in peripheral.clusters:
                warn_if_ambiguous(peripheral.clusters, s[1])
                cluster = peripheral.clusters[s[1]]
                container = peripheral.name + " > " + cluster.name
                self._print_registers(container, form, cluster.registers)

            elif s[1] in peripheral.registers:
                warn_if_ambiguous(peripheral.registers, s[1])
                register = peripheral.registers[s[1]]
                container = peripheral.name + " > " + register.name

                self._print_register_fields(container, form, register)
            else:
                found = False
                for key in fnmatch.filter(peripheral.registers.keys(), s[1]):
                    register = peripheral.registers[key]
                    container = peripheral.name + " > " + register.name
                    self._print_register_fields(container, form, register)
                    found = True
                if not found:
                    gdb.write(
                        "Register/cluster {} in peripheral {} does not exist!\n".format(
                            s[1], peripheral.name
                        )
                    )
            return

        if len(s) == 3:
            if s[1] not in peripheral.clusters:
                gdb.write(
                    "Cluster {} in peripheral {} does not exist!\n".format(
                        s[1], peripheral.name
                    )
                )
                return
            warn_if_ambiguous(peripheral.clusters, s[1])

            cluster = peripheral.clusters[s[1]]
            if s[2] not in cluster.registers:
                gdb.write(
                    "Register {} in cluster {} in peripheral {} does not exist!\n".format(
                        s[2], cluster.name, peripheral.name
                    )
                )
                return
            warn_if_ambiguous(cluster.registers, s[2])

            register = cluster.registers[s[2]]
            container = " > ".join([peripheral.name, cluster.name, register.name])
            self._print_register_fields(container, form, register)
            return

        if len(s) == 4:
            if s[1] not in peripheral.registers:
                gdb.write(
                    "Register {} in peripheral {} does not exist!\n".format(
                        s[1], peripheral.name
                    )
                )
                return
            warn_if_ambiguous(peripheral.registers, s[1])

            reg = peripheral.registers[s[1]]

            if s[2] not in reg.fields:
                gdb.write(
                    "Field {} in register {} in peripheral {} does not exist!\n".format(
                        s[2], reg.name, peripheral.name
                    )
                )
                return
            warn_if_ambiguous(reg.fields, s[2])

            field = reg.fields[s[2]]

            if not field.writable() or not reg.writable():
                gdb.write(
                    "Field {} in register {} in peripheral {} is read-only!\n".format(
                        field.name, reg.name, peripheral.name
                    )
                )
                return

            try:
                val = int(s[3], 0)
            except ValueError:
                gdb.write(
                    "{} is not a valid number! You can prefix numbers with 0x for hex, 0b for binary, or any python "
                    "int literal\n".format(s[3])
                )
                return

            if val >= 1 << field.width or val < 0:
                gdb.write(
                    "{} not a valid number for a field with width {}!\n".format(
                        val, field.width
                    )
                )
                return

            if not reg.readable():
                data = 0
            else:
                data = self.read(reg.address(), reg.size)
            data &= ~(((1 << field.width) - 1) << field.offset)
            data |= val << field.offset
            self.write(reg.address(), data, reg.size)
            return

        gdb.write("Unknown input\n")

    def complete(self, text, word):
        """Perform tab-completion for the command"""
        text = str(text)
        s = text.split(" ")

        # Deal with the possibility of a '/' parameter
        if s[0] and s[0][0] == "/":
            if len(s) > 1:
                s = s[1:]
            else:
                return []  # completion after e.g. "svd/x" but before trailing space

        if len(s) == 1:
            return list(self.svd_file.peripherals.prefix_match_iter(s[0]))

        if len(s) == 2:
            reg = s[1].upper()
            if len(reg) and reg[0] == "&":
                reg = reg[1:]

            if s[0] not in self.svd_file.peripherals:
                return []

            per = self.svd_file.peripherals[s[0]]
            return list(per.registers.prefix_match_iter(s[1]))

        return []

    @staticmethod
    def read(address, bits=32):
        """Read from memory and return an integer"""
        value = gdb.selected_inferior().read_memory(address, bits / 8)
        unpack_format = "I"
        if bits in BITS_TO_UNPACK_FORMAT:
            unpack_format = BITS_TO_UNPACK_FORMAT[bits]
        # gdb.write("{:x} {}\n".format(address, binascii.hexlify(value)))
        return struct.unpack_from("<" + unpack_format, value)[0]

    @staticmethod
    def write(address, data, bits=32):
        """Write data to memory"""
        gdb.selected_inferior().write_memory(address, bytes(data), bits / 8)

    @staticmethod
    def format(value, form, length=32):
        """Format a number based on a format character and length"""
        # get current gdb radix setting
        radix = int(
            re.search(r"\d+", gdb.execute("show output-radix", True, True)).group(0)
        )

        # override it if asked to
        if form == "x" or form == "a":
            radix = 16
        elif form == "o":
            radix = 8
        elif form == "b" or form == "t":
            radix = 2

        # format the output
        if radix == 16:
            # For addresses, probably best in hex too
            l = int(math.ceil(length / 4.0))
            return "0x" + "{:X}".format(value).zfill(l)
        if radix == 8:
            l = int(math.ceil(length / 3.0))
            return "0" + "{:o}".format(value).zfill(l)
        if radix == 2:
            return "0b" + "{:b}".format(value).zfill(length)
        # Default: Just return in decimal
        return str(value)

    def peripheral_list(self):
        try:
            keys = self.svd_file.peripherals.iterkeys()
        except AttributeError:
            keys = self.svd_file.peripherals.keys()
        return list(keys)

    def register_list(self, peripheral):
        try:
            try:
                keys = self.svd_file.peripherals[peripheral].registers.iterkeys()
            except AttributeError:
                keys = self.svd_file.peripherals[peripheral].registers.keys()
            return list(keys)
        except:
            gdb.write("Peripheral {} doesn't exist\n".format(peripheral))
            return []

    def field_list(self, peripheral, register):
        try:
            periph = self.svd_file.peripherals[peripheral]
            reg = periph.registers[register]
            try:
                regs = reg.fields.iterkeys()
            except AttributeError:
                regs = reg.fields.keys()
            return list(regs)
        except:
            gdb.write("Register {} doesn't exist on {}\n".format(register, peripheral))
            return []
