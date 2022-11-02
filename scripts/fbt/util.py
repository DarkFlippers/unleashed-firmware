import SCons
from SCons.Subst import quote_spaces
from SCons.Errors import StopError
from SCons.Node.FS import _my_normcase

import re
import os


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
            raise StopError(f"Source directory {source_path} is not a directory")

        if not os.path.exists(target_path):
            _winapi.CreateJunction(source_path, target_path)
    else:
        os.symlink(source_path, target_path)


def single_quote(arg_list):
    return " ".join(f"'{arg}'" if " " in arg else str(arg) for arg in arg_list)


def extract_abs_dir_path(node):
    if isinstance(node, SCons.Node.FS.EntryProxy):
        node = node.get()

    for repo_dir in node.get_all_rdirs():
        if os.path.exists(repo_dir.abspath):
            return repo_dir.abspath

    raise StopError(f"Can't find absolute path for {node.name}")
