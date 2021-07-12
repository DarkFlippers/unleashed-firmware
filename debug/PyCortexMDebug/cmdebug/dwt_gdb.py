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
import struct

DWT_CTRL = 0xE0001000
DWT_CYCCNT = 0xE0001004
DWT_CPICNT = 0xE0001008
DWT_EXTCNT = 0xE000100C
DWT_SLEEPCNT = 0xE0001010
DWT_LSUCNT = 0xE0001014
DWT_FOLDCNT = 0xE0001018
DWT_PCSR = 0xE000101C

prefix = "dwt : "


class DWT(gdb.Command):
    clk = None
    is_init = False

    def __init__(self):
        gdb.Command.__init__(self, "dwt", gdb.COMMAND_DATA)

    @staticmethod
    def read(address, bits=32):
        """Read from memory (using print) and return an integer"""
        value = gdb.selected_inferior().read_memory(address, bits / 8)
        return struct.unpack_from("<i", value)[0]

    @staticmethod
    def write(address, value, bits=32):
        """Set a value in memory"""
        gdb.selected_inferior().write_memory(address, bytes(value), bits / 8)

    def invoke(self, args, from_tty):
        if not self.is_init:
            self.write(0xE000EDFC, self.read(0xE000EDFC) | (1 << 24))
            self.write(DWT_CTRL, 0)
            self.is_init = True

        s = list(map(lambda x: x.lower(), str(args).split(" ")))
        # Check for empty command
        if s[0] in ["", "help"]:
            self.print_help()
            return ()

        if s[0] == "cyccnt":
            if len(s) > 1:
                if s[1][:2] == "en":
                    self.cyccnt_en()
                elif s[1][0] == "r":
                    self.cyccnt_reset()
                elif s[1][0] == "d":
                    self.cyccnt_dis()
            gdb.write(
                prefix
                + "CYCCNT ({}): ".format("ON" if (self.read(DWT_CTRL) & 1) else "OFF")
                + self.cycles_str(self.read(DWT_CYCCNT))
            )
        elif s[0] == "reset":
            if len(s) > 1:
                if s[1] == "cyccnt":
                    self.cyccnt_reset()
                    gdb.write(prefix + "CYCCNT reset\n")
                if s[1] == "counters":
                    self.cyccnt_reset()
                    gdb.write(prefix + "CYCCNT reset\n")
                else:
                    self.cyccnt_reset()
                    gdb.write(prefix + "CYCCNT reset\n")
            else:
                # Reset everything
                self.cyccnt_reset()
                gdb.write(prefix + "CYCCNT reset\n")
        elif s[0] == "configclk":
            if len(s) == 2:
                try:
                    self.clk = float(s[1])
                except:
                    self.print_help()
            else:
                self.print_help()
        else:
            # Try to figure out what stupid went on here
            gdb.write(args)
            self.print_help()

    @staticmethod
    def complete(text, word):
        text = str(text).lower()
        s = text.split(" ")

        commands = ["configclk", "reset", "cyccnt"]
        reset_commands = ["counters", "cyccnt"]
        cyccnt_commands = ["enable", "reset", "disable"]

        if len(s) == 1:
            return filter(lambda x: x.startswith(s[0]), commands)

        if len(s) == 2:
            if s[0] == "reset":
                return filter(lambda x: x.startswith(s[1]), reset_commands)
            if s[0] == "cyccnt":
                return filter(lambda x: x.startswith(s[1]), cyccnt_commands)

    def cycles_str(self, cycles):
        if self.clk:
            return "%d cycles, %.3es\n" % (cycles, cycles * 1.0 / self.clk)
        else:
            return "%d cycles"

    def cyccnt_en(self):
        self.write(DWT_CTRL, self.read(DWT_CTRL) | 1)

    def cyccnt_dis(self):
        self.write(DWT_CTRL, self.read(DWT_CTRL) & 0xFFFFFFFE)

    def cyccnt_reset(self, value=0):
        self.write(DWT_CYCCNT, value)

    def cpicnt_reset(self, value=0):
        self.write(DWT_CPICNT, value & 0xFF)

    @staticmethod
    def print_help():
        gdb.write("Usage:\n")
        gdb.write("=========\n")
        gdb.write("dwt:\n")
        gdb.write("\tList available peripherals\n")
        gdb.write("dwt configclk [Hz]:\n")
        gdb.write("\tSet clock for rendering time values in seconds\n")
        gdb.write("dwt reset:\n")
        gdb.write("\tReset everything in DWT\n")
        gdb.write("dwt reset counters:\n")
        gdb.write("\tReset all DWT counters\n")
        gdb.write("dwt cyccnt\n")
        gdb.write("\tDisplay the cycle count\n")
        gdb.write("\td(default):decimal, x: hex, o: octal, b: binary\n")
        return


# Registers our class to GDB when sourced:
DWT()
