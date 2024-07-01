#!/usr/bin/env python3

import heatshrink2 as hs
from flipper.app import App
from flipper.assets.heatshrink_stream import HeatshrinkDataStreamHeader
from flipper.assets.tarball import compress_tree_tarball


class HSWrapper(App):
    DEFAULT_WINDOW = 13
    DEFAULT_LOOKAHEAD = 6

    def init(self):
        self.subparsers = self.parser.add_subparsers(
            title="subcommands", dest="subcommand"
        )

        self.parser_compress = self.subparsers.add_parser(
            "compress", help="compress file using heatshrink"
        )
        self.parser_compress.add_argument(
            "-w", "--window", help="window size", type=int, default=self.DEFAULT_WINDOW
        )
        self.parser_compress.add_argument(
            "-l",
            "--lookahead",
            help="lookahead size",
            type=int,
            default=self.DEFAULT_LOOKAHEAD,
        )
        self.parser_compress.add_argument("file", help="file to compress")
        self.parser_compress.add_argument(
            "-o", "--output", help="output file", required=True
        )
        self.parser_compress.set_defaults(func=self.compress)

        self.parser_decompress = self.subparsers.add_parser(
            "decompress", help="decompress file using heatshrink"
        )
        self.parser_decompress.add_argument("file", help="file to decompress")
        self.parser_decompress.add_argument(
            "-o", "--output", help="output file", required=True
        )
        self.parser_decompress.set_defaults(func=self.decompress)

        self.parser_info = self.subparsers.add_parser("info", help="show file info")
        self.parser_info.add_argument("file", help="file to show info for")
        self.parser_info.set_defaults(func=self.info)

        self.parser_tar = self.subparsers.add_parser(
            "tar", help="create a tarball and compress it"
        )
        self.parser_tar.add_argument("dir", help="directory to tar")
        self.parser_tar.add_argument(
            "-o", "--output", help="output file", required=True
        )
        self.parser_tar.add_argument(
            "-w", "--window", help="window size", type=int, default=self.DEFAULT_WINDOW
        )
        self.parser_tar.add_argument(
            "-l",
            "--lookahead",
            help="lookahead size",
            type=int,
            default=self.DEFAULT_LOOKAHEAD,
        )
        self.parser_tar.set_defaults(func=self.tar)

    def compress(self):
        args = self.args

        with open(args.file, "rb") as f:
            data = f.read()

        compressed = hs.compress(
            data, window_sz2=args.window, lookahead_sz2=args.lookahead
        )

        with open(args.output, "wb") as f:
            header = HeatshrinkDataStreamHeader(args.window, args.lookahead)
            f.write(header.pack())
            f.write(compressed)

        self.logger.info(
            f"Compressed {len(data)} bytes to {len(compressed)} bytes, "
            f"compression ratio: {len(compressed) * 100 / len(data):.2f}%"
        )
        return 0

    def decompress(self):
        args = self.args

        with open(args.file, "rb") as f:
            header = HeatshrinkDataStreamHeader.unpack(f.read(7))
            compressed = f.read()

        self.logger.info(
            f"Decompressing with window size {header.window_size} and lookahead size {header.lookahead_size}"
        )

        data = hs.decompress(
            compressed,
            window_sz2=header.window_size,
            lookahead_sz2=header.lookahead_size,
        )

        with open(args.output, "wb") as f:
            f.write(data)

        self.logger.info(f"Decompressed {len(compressed)} bytes to {len(data)} bytes")
        return 0

    def info(self):
        args = self.args

        try:
            with open(args.file, "rb") as f:
                header = HeatshrinkDataStreamHeader.unpack(f.read(7))
        except Exception as e:
            self.logger.error(f"Error: {e}")
            return 1

        self.logger.info(
            f"Window size: {header.window_size}, lookahead size: {header.lookahead_size}"
        )

        return 0

    def tar(self):
        args = self.args

        orig_size, compressed_size = compress_tree_tarball(
            args.dir, args.output, hs_window=args.window, hs_lookahead=args.lookahead
        )

        self.logger.info(
            f"Tarred and compressed {orig_size} bytes to {compressed_size} bytes, "
            f"compression ratio: {compressed_size * 100 / orig_size:.2f}%"
        )

        return 0


if __name__ == "__main__":
    HSWrapper()()
