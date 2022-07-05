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


def single_quote(arg_list):
    return " ".join(f"'{arg}'" if " " in arg else str(arg) for arg in arg_list)


def link_elf_dir_as_latest(env, elf_node):
    elf_dir = elf_node.Dir(".")
    latest_dir = env.Dir("#build/latest")
    print(f"Setting {elf_dir} as latest built dir (./build/latest/)")
    return link_dir(latest_dir.abspath, elf_dir.abspath, env["PLATFORM"] == "win32")


def should_gen_cdb_and_link_dir(env, requested_targets):
    explicitly_building_updater = False
    # Hacky way to check if updater-related targets were requested
    for build_target in requested_targets:
        if "updater" in str(build_target):
            explicitly_building_updater = True

    is_updater = not env["IS_BASE_FIRMWARE"]
    # If updater is explicitly requested, link to the latest updater
    # Otherwise, link to firmware
    return (is_updater and explicitly_building_updater) or (
        not is_updater and not explicitly_building_updater
    )
