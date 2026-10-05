import io
import os
import tarfile

import heatshrink2
from flipper.utils import is_macos_junk

from .heatshrink_stream import HeatshrinkDataStreamHeader

FLIPPER_TAR_FORMAT = tarfile.USTAR_FORMAT
TAR_HEATSRINK_EXTENSION = ".ths"


def tar_sanitizer_filter(tarinfo: tarfile.TarInfo):
    if is_macos_junk(os.path.basename(tarinfo.name)):
        return None
    tarinfo.gid = tarinfo.uid = 0
    tarinfo.mtime = 0
    tarinfo.uname = tarinfo.gname = "furippa"
    return tarinfo


def compress_tree_tarball(
    src_dir, output_name, filter=tar_sanitizer_filter, hs_window=13, hs_lookahead=6
):
    plain_tar = io.BytesIO()
    with tarfile.open(
        fileobj=plain_tar,
        mode="w:",
        format=FLIPPER_TAR_FORMAT,
    ) as tarball:
        tarball.add(src_dir, arcname="", filter=filter)
    plain_tar.seek(0)

    src_data = plain_tar.read()
    compressed = heatshrink2.compress(
        src_data, window_sz2=hs_window, lookahead_sz2=hs_lookahead
    )

    header = HeatshrinkDataStreamHeader(hs_window, hs_lookahead)
    with open(output_name, "wb") as f:
        f.write(header.pack())
        f.write(compressed)

    return len(src_data), len(compressed)
