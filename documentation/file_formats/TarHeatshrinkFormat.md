# Heatshrink-compressed Tarball Format {#heatshrink_file_format}

Flipper supports the use of Heatshrink compression library for `.tar` archives. This allows for smaller file sizes and faster OTA updates. 

Heatshrink specification does not define a container format for storing compression parameters. This document describes the format used by Flipper to store Heatshrink-compressed data streams.

## Header

Header begins with a magic value, followed by a version number and compression parameters - window size and lookahead size.

Magic value consists of 4 bytes: `0x48 0x53 0x44 0x53` (ASCII "HSDS", HeatShrink DataStream).

Version number is a single byte, currently set to `0x01`.

Window size is a single byte, representing the size of the sliding window used by the compressor. It corresponds to `-w` parameter in Heatshrink CLI.

Lookahead size is a single byte, representing the size of the lookahead buffer used by the compressor. It corresponds to `-l` parameter in Heatshrink CLI.

Total header size is 7 bytes. Header is followed by compressed data.
