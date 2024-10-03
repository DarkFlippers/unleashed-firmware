import logging


class FlipperFormatFile:
    def __init__(self):
        # Storage
        self.lines = []
        self.cursor = 0
        # Logger
        self.logger = logging.getLogger("FlipperFormatFile")

    def _resetCursor(self):
        self.cursor = 0

    def nextLine(self):
        line = None
        while self.cursor < len(self.lines):
            temp_line = self.lines[self.cursor].strip()
            self.cursor += 1
            if len(temp_line) > 0 and not temp_line.startswith("#"):
                line = temp_line
                break
        if line is None:
            raise EOFError()
        return line

    def readKeyValue(self):
        line = self.nextLine()
        data = line.split(":", 1)
        if len(data) != 2:
            self.logger.error(f"Incorrectly formated line {self.cursor}: `{line}`")
            raise Exception("Unexpected line: not `key:value`")
        return data[0].strip(), data[1].strip()

    def readComment(self):
        if self.cursor == len(self.lines):
            raise EOFError()
        line = self.lines[self.cursor].strip()
        if line.startswith("#"):
            self.cursor += 1
            return line[1:].strip()
        else:
            return None

    def readKey(self, key: str):
        k, v = self.readKeyValue()
        if k != key:
            raise KeyError(f"Unexpected key {k} != {key}")
        return v

    def readKeyInt(self, key: str):
        value = self.readKey(key)
        return int(value) if value else None

    def readKeyIntArray(self, key: str):
        value = self.readKey(key)
        return [int(i) for i in value.split(" ")] if value else None

    def readKeyFloat(self, key: str):
        value = self.readKey(key)
        return float(value) if value else None

    def writeLine(self, line: str):
        self.lines.insert(self.cursor, line)
        self.cursor += 1

    def writeKey(self, key: str, value):
        if isinstance(value, (str, int, float)):
            pass
        elif isinstance(value, (list, set)):
            value = " ".join(map(str, value))
        else:
            raise Exception("Unknown value type")
        self.writeLine(f"{key}: {value}")

    def writeEmptyLine(self):
        self.writeLine("")

    def writeComment(self, text: str):
        if text and len(text):
            self.writeLine(f"# {text}")
        else:
            self.writeLine("#")

    def getHeader(self):
        if self.cursor != 0 and len(self.lines) == 0:
            raise Exception("Can't read header data: cursor not at 0 or file is empty")

        # Read Filetype
        key, value = self.readKeyValue()
        if key != "Filetype":
            raise Exception("Invalid Header: missing `Filetype`")
        filetype = value

        # Read Version
        key, value = self.readKeyValue()
        if key != "Version":
            raise Exception("Invalid Header: missing `Version`")
        version = int(value)

        return filetype, version

    def setHeader(self, filetype: str, version: int):
        if self.cursor != 0 and len(self.lines) != 0:
            raise Exception("Can't set header data: file is not empty")

        self.writeKey("Filetype", filetype)
        self.writeKey("Version", version)

    def load(self, filename: str):
        with open(filename, "r") as file:
            self.lines = file.readlines()

    def save(self, filename: str):
        with open(filename, "w", newline="\n") as file:
            file.write("\n".join(self.lines))
            file.write("\n")
