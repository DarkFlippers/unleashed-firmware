import SCons

from SCons.Builder import Builder
from SCons.Action import Action
from SCons.Node.FS import File

import os
import subprocess


def icons_emitter(target, source, env):
    target = [
        "compiled/assets_icons.c",
        "compiled/assets_icons.h",
    ]
    source = env.GlobRecursive("*.*", env["ICON_SRC_DIR"])
    return target, source


def proto_emitter(target, source, env):
    target = []
    for src in source:
        basename = os.path.splitext(src.name)[0]
        target.append(env.File(f"compiled/{basename}.pb.c"))
        target.append(env.File(f"compiled/{basename}.pb.h"))
    return target, source


def dolphin_emitter(target, source, env):
    res_root_dir = source[0].Dir(env["DOLPHIN_RES_TYPE"])
    source = [res_root_dir]
    source.extend(
        env.GlobRecursive("*.*", res_root_dir),
    )

    target_base_dir = target[0]
    env.Replace(_DOLPHIN_OUT_DIR=target[0])

    if env["DOLPHIN_RES_TYPE"] == "external":
        target = []
        target.extend(
            map(
                lambda node: target_base_dir.File(
                    res_root_dir.rel_path(node).replace(".png", ".bm")
                ),
                filter(lambda node: isinstance(node, SCons.Node.FS.File), source),
            )
        )
    else:
        asset_basename = f"assets_dolphin_{env['DOLPHIN_RES_TYPE']}"
        target = [
            target_base_dir.File(asset_basename + ".c"),
            target_base_dir.File(asset_basename + ".h"),
        ]

    return target, source


def _invoke_git(args, source_dir):
    cmd = ["git"]
    cmd.extend(args)
    return (
        subprocess.check_output(cmd, cwd=source_dir, stderr=subprocess.STDOUT)
        .strip()
        .decode()
    )


def proto_ver_generator(target, source, env):
    target_file = target[0]
    src_dir = source[0].dir.abspath
    try:
        git_fetch = _invoke_git(
            ["fetch", "--tags"],
            source_dir=src_dir,
        )
    except (subprocess.CalledProcessError, EnvironmentError) as e:
        # Not great, not terrible
        print("Git: fetch failed")

    try:
        git_describe = _invoke_git(
            ["describe", "--tags", "--abbrev=0"],
            source_dir=src_dir,
        )
    except (subprocess.CalledProcessError, EnvironmentError) as e:
        print("Git: describe failed")
        Exit("git error")

    # print("describe=", git_describe)
    git_major, git_minor = git_describe.split(".")
    version_file_data = (
        "#pragma once",
        f"#define PROTOBUF_MAJOR_VERSION {git_major}",
        f"#define PROTOBUF_MINOR_VERSION {git_minor}",
        "",
    )
    with open(str(target_file), "wt") as file:
        file.write("\n".join(version_file_data))


def generate(env):
    env.SetDefault(
        ASSETS_COMPILER="${ROOT_DIR.abspath}/scripts/assets.py",
        NANOPB_COMPILER="${ROOT_DIR.abspath}/lib/nanopb/generator/nanopb_generator.py",
    )

    env.Append(
        BUILDERS={
            "IconBuilder": Builder(
                action=Action(
                    '${PYTHON3} "${ASSETS_COMPILER}" icons ${ICON_SRC_DIR} ${TARGET.dir}',
                    "${ICONSCOMSTR}",
                ),
                emitter=icons_emitter,
            ),
            "ProtoBuilder": Builder(
                action=Action(
                    '${PYTHON3} "${NANOPB_COMPILER}" -q -I${SOURCE.dir.posix} -D${TARGET.dir.posix} ${SOURCES.posix}',
                    "${PROTOCOMSTR}",
                ),
                emitter=proto_emitter,
                suffix=".pb.c",
                src_suffix=".proto",
            ),
            "DolphinSymBuilder": Builder(
                action=Action(
                    '${PYTHON3} "${ASSETS_COMPILER}" dolphin -s dolphin_${DOLPHIN_RES_TYPE} "${SOURCE}" "${_DOLPHIN_OUT_DIR}"',
                    "${DOLPHINCOMSTR}",
                ),
                emitter=dolphin_emitter,
            ),
            "DolphinExtBuilder": Builder(
                action=Action(
                    '${PYTHON3} "${ASSETS_COMPILER}" dolphin "${SOURCE}" "${_DOLPHIN_OUT_DIR}"',
                    "${DOLPHINCOMSTR}",
                ),
                emitter=dolphin_emitter,
            ),
            "ProtoVerBuilder": Builder(
                action=Action(
                    proto_ver_generator,
                    "${PBVERCOMSTR}",
                ),
            ),
        }
    )


def exists(env):
    return True
