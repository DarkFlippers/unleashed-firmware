import SCons
from SCons.Subst import quote_spaces

import re
import os
import random
import string

WINPATHSEP_RE = re.compile(r"\\([^\"'\\]|$)")


def tempfile_arg_esc_func(arg):
    arg = quote_spaces(arg)
    if SCons.Platform.platform_default() != "win32":
        return arg
    # GCC requires double Windows slashes, let's use UNIX separator
    return WINPATHSEP_RE.sub(r"/\1", arg)


def wrap_tempfile(env, command):
    env[command] = '${TEMPFILE("' + env[command] + '","$' + command + 'STR")}'


def link_dir(target_path, source_path, is_windows):
    # print(f"link_dir: {target_path} -> {source_path}")
    if os.path.lexists(target_path) or os.path.exists(target_path):
        os.unlink(target_path)
    if is_windows:
        # Crete junction
        import _winapi

        if not os.path.isdir(source_path):
            raise Exception(f"Source directory {source_path} is not a directory")

        if not os.path.exists(target_path):
            _winapi.CreateJunction(source_path, target_path)
    else:
        os.symlink(source_path, target_path)


def random_alnum(length):
    return "".join(
        random.choice(string.ascii_letters + string.digits) for _ in range(length)
    )
