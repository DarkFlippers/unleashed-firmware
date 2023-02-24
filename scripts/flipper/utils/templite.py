#  Templite++
#  A light-weight, fully functional, general purpose templating engine
#  Proudly made of shit and sticks. Strictly not for production use.
#  Extremly unsafe and difficult to debug.
#
#  Copyright (c) 2022 Flipper Devices
#  Author: Aleksandr Kutuzov <alletam@gmail.com>
#
#  Copyright (c) 2009 joonis new media
#  Author: Thimo Kraemer <thimo.kraemer@joonis.de>
#
#  Based on Templite by Tomer Filiba
#  http://code.activestate.com/recipes/496702/
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.


from enum import Enum

import sys
import os


class TempliteCompiler:
    class State(Enum):
        TEXT = 1
        CONTROL = 2
        VARIABLE = 3

    def __init__(self, source: str, encoding: str):
        self.blocks = [f"# -*- coding: {encoding} -*-"]
        self.block = ""
        self.source = source
        self.cursor = 0
        self.offset = 0

    def processText(self):
        self.block = self.block.replace("\\", "\\\\").replace('"', '\\"')
        self.block = "\t" * self.offset + f'write("""{self.block}""")'
        self.blocks.append(self.block)
        self.block = ""

    def getLine(self):
        return self.source[: self.cursor].count("\n") + 1

    def controlIsEnding(self):
        block_stripped = self.block.lstrip()
        if block_stripped.startswith(":"):
            if not self.offset:
                raise SyntaxError(
                    f"Line: {self.getLine()}, no statement to terminate: `{block_stripped}`"
                )
            self.offset -= 1
            self.block = block_stripped[1:]
            if not self.block.endswith(":"):
                return True
        return False

    def processControl(self):
        self.block = self.block.rstrip()

        if self.controlIsEnding():
            self.block = ""
            return

        lines = self.block.splitlines()
        margin = min(len(l) - len(l.lstrip()) for l in lines if l.strip())
        self.block = "\n".join("\t" * self.offset + l[margin:] for l in lines)
        self.blocks.append(self.block)
        if self.block.endswith(":"):
            self.offset += 1
        self.block = ""

    def processVariable(self):
        self.block = self.block.strip()
        self.block = "\t" * self.offset + f"write({self.block})"
        self.blocks.append(self.block)
        self.block = ""

    def compile(self):
        state = self.State.TEXT

        # Process template source
        while self.cursor < len(self.source):
            # Process plain text till first token occurance
            if state == self.State.TEXT:
                if self.source[self.cursor :].startswith("{%"):
                    state = self.State.CONTROL
                    self.cursor += 1
                elif self.source[self.cursor :].startswith("{{"):
                    state = self.State.VARIABLE
                    self.cursor += 1
                else:
                    self.block += self.source[self.cursor]
                # Commit self.block if token was found
                if state != self.State.TEXT:
                    self.processText()
            elif state == self.State.CONTROL:
                if self.source[self.cursor :].startswith("%}"):
                    self.cursor += 1
                    state = self.State.TEXT
                    self.processControl()
                else:
                    self.block += self.source[self.cursor]
            elif state == self.State.VARIABLE:
                if self.source[self.cursor :].startswith("}}"):
                    self.cursor += 1
                    state = self.State.TEXT
                    self.processVariable()
                else:
                    self.block += self.source[self.cursor]
            else:
                raise Exception("Unknown State")

            self.cursor += 1

        if state != self.State.TEXT:
            raise Exception("Last self.block was not closed")

        if self.block:
            self.processText()

        return "\n".join(self.blocks)


class Templite:
    cache = {}

    def __init__(self, text=None, filename=None, encoding="utf-8", caching=False):
        """Loads a template from string or file."""
        if filename:
            filename = os.path.abspath(filename)
            mtime = os.path.getmtime(filename)
            self.file = key = filename
        elif text is not None:
            self.file = mtime = None
            key = hash(text)
        else:
            raise ValueError("either text or filename required")
        # set attributes
        self.encoding = encoding
        self.caching = caching
        # check cache
        cache = self.cache
        if caching and key in cache and cache[key][0] == mtime:
            self._code = cache[key][1]
            return
        # read file
        if filename:
            with open(filename) as fh:
                text = fh.read()
        # Compile template to executable
        code = TempliteCompiler(text, self.encoding).compile()
        self._code = compile(code, self.file or "<string>", "exec")
        # Cache for future use
        if caching:
            cache[key] = (mtime, self._code)

    def render(self, **namespace):
        """Renders the template according to the given namespace."""
        stack = []
        namespace["__file__"] = self.file

        # add write method
        def write(*args):
            for value in args:
                stack.append(str(value))

        namespace["write"] = write

        # add include method
        def include(file):
            if not os.path.isabs(file):
                if self.file:
                    base = os.path.dirname(self.file)
                else:
                    base = os.path.dirname(sys.argv[0])
                file = os.path.join(base, file)
            t = Templite(None, file, self.encoding, self.delimiters, self.caching)
            stack.append(t.render(**namespace))

        namespace["include"] = include
        # execute template code
        exec(self._code, namespace)
        return "".join(stack)
