import os
import subprocess

from ansi.color import fg
from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Errors import StopError


def icons_emitter(target, source, env):
    target = [
        target[0].File(env.subst("${ICON_FILE_NAME}.c")),
        target[0].File(env.subst("${ICON_FILE_NAME}.h")),
    ]
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
    source.extend(env.GlobRecursive("*.*", res_root_dir.srcnode()))

    target_base_dir = target[0]
    env.Replace(_DOLPHIN_OUT_DIR=target[0])

    if env["DOLPHIN_RES_TYPE"] == "external":
        target = [target_base_dir.File("manifest.txt")]
        ## A detailed list of files to be generated
        ## works better if we just leave target the folder
        # target = []
        # target.extend(
        #     map(
        #         lambda node: target_base_dir.File(
        #             res_root_dir.rel_path(node).replace(".png", ".bm")
        #         ),
        #         filter(lambda node: isinstance(node, SCons.Node.FS.File), source),
        #     )
        # )
    else:
        asset_basename = f"assets_dolphin_{env['DOLPHIN_RES_TYPE']}"
        target = [
            target_base_dir.File(asset_basename + ".c"),
            target_base_dir.File(asset_basename + ".h"),
        ]

    # Debug output
    # print(
    #     f"Dolphin res type: {env['DOLPHIN_RES_TYPE']},\ntarget files:",
    #     list(f.path for f in target),
    #     f"\nsource files:",
    #     list(f.path for f in source),
    # )
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
        _invoke_git(
            ["fetch", "--tags"],
            source_dir=src_dir,
        )
    except (subprocess.CalledProcessError, EnvironmentError):
        # Not great, not terrible
        print(fg.boldred("Git: fetch failed"))

    try:
        git_describe = _invoke_git(
            ["describe", "--tags", "--abbrev=0"],
            source_dir=src_dir,
        )
    except (subprocess.CalledProcessError, EnvironmentError):
        raise StopError("Git: describe failed")

    git_major, git_minor = git_describe.split(".")
    version_file_data = (
        "#pragma once",
        f"#define PROTOBUF_MAJOR_VERSION {git_major}",
        f"#define PROTOBUF_MINOR_VERSION {git_minor}",
        "",
    )
    with open(str(target_file), "wt") as file:
        file.write("\n".join(version_file_data))


def CompileIcons(env, target_dir, source_dir, *, icon_bundle_name="assets_icons"):
    # Gathering icons sources
    icons_src = env.GlobRecursive("*.png", source_dir)
    icons_src += env.GlobRecursive("frame_rate", source_dir)

    icons = env.IconBuilder(
        target_dir,
        source_dir,
        ICON_FILE_NAME=icon_bundle_name,
    )
    env.Depends(icons, icons_src)
    return icons


def generate(env):
    env.SetDefault(
        ASSETS_COMPILER="${FBT_SCRIPT_DIR}/assets.py",
        NANOPB_COMPILER="${ROOT_DIR}/lib/nanopb/generator/nanopb_generator.py",
    )
    env.AddMethod(CompileIcons)

    if not env["VERBOSE"]:
        env.SetDefault(
            ICONSCOMSTR="\tICONS\t${TARGET}",
            PROTOCOMSTR="\tPROTO\t${SOURCE}",
            DOLPHINCOMSTR="\tDOLPHIN\t${DOLPHIN_RES_TYPE}",
            RESMANIFESTCOMSTR="\tMANIFEST\t${TARGET}",
            PBVERCOMSTR="\tPBVER\t${TARGET}",
        )

    env.Append(
        BUILDERS={
            "IconBuilder": Builder(
                action=Action(
                    '${PYTHON3} ${ASSETS_COMPILER} icons ${ABSPATHGETTERFUNC(SOURCE)} ${TARGET.dir} --filename "${ICON_FILE_NAME}"',
                    "${ICONSCOMSTR}",
                ),
                emitter=icons_emitter,
            ),
            "ProtoBuilder": Builder(
                action=Action(
                    "${PYTHON3} ${NANOPB_COMPILER} -q -I${SOURCE.dir.posix} -D${TARGET.dir.posix} ${SOURCES.posix}",
                    "${PROTOCOMSTR}",
                ),
                emitter=proto_emitter,
                suffix=".pb.c",
                src_suffix=".proto",
            ),
            "DolphinSymBuilder": Builder(
                action=Action(
                    "${PYTHON3} ${ASSETS_COMPILER} dolphin -s dolphin_${DOLPHIN_RES_TYPE} ${SOURCE} ${_DOLPHIN_OUT_DIR}",
                    "${DOLPHINCOMSTR}",
                ),
                emitter=dolphin_emitter,
            ),
            "DolphinExtBuilder": Builder(
                action=Action(
                    "${PYTHON3} ${ASSETS_COMPILER} dolphin ${SOURCE} ${_DOLPHIN_OUT_DIR}",
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
