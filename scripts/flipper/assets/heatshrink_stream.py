import struct


class HeatshrinkDataStreamHeader:
    MAGIC = 0x53445348
    VERSION = 1

    def __init__(self, window_size, lookahead_size):
        self.window_size = window_size
        self.lookahead_size = lookahead_size

    def pack(self):
        return struct.pack(
            "<IBBB", self.MAGIC, self.VERSION, self.window_size, self.lookahead_size
        )

    @staticmethod
    def unpack(data):
        if len(data) != 7:
            raise ValueError("Invalid header length")
        magic, version, window_size, lookahead_size = struct.unpack("<IBBB", data)
        if magic != HeatshrinkDataStreamHeader.MAGIC:
            raise ValueError("Invalid magic number")
        if version != HeatshrinkDataStreamHeader.VERSION:
            raise ValueError("Invalid version")
        return HeatshrinkDataStreamHeader(window_size, lookahead_size)
