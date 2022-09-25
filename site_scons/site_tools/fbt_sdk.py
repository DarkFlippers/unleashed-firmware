from SCons.Builder import Builder
from SCons.Action import Action
from SCons.Errors import UserError

# from SCons.Scanner import C
from SCons.Script import Mkdir, Copy, Delete, Entry
from SCons.Util import LogicalLines

import os.path
import posixpath
import pathlib

from fbt.sdk import SdkCollector, SdkCache


def prebuild_sdk_emitter(target, source, env):
    target.append(env.ChangeFileExtension(target[0], ".d"))
    target.append(env.ChangeFileExtension(target[0], ".i.c"))
    return target, source


def prebuild_sdk_create_origin_file(target, source, env):
    mega_file = env.subst("${TARGET}.c", target=target[0])
    with open(mega_file, "wt") as sdk_c:
        sdk_c.write("\n".join(f"#include <{h.path}>" for h in env["SDK_HEADERS"]))


class SdkTreeBuilder:
    def __init__(self, env, target, source) -> None:
        self.env = env
        self.target = target
        self.source = source

        self.header_depends = []
        self.header_dirs = []

        self.target_sdk_dir = env.subst("f${TARGET_HW}_sdk")
        self.sdk_deploy_dir = target[0].Dir(self.target_sdk_dir)

    def _parse_sdk_depends(self):
        deps_file = self.source[0]
        with open(deps_file.path, "rt") as deps_f:
            lines = LogicalLines(deps_f).readlines()
            _, depends = lines[0].split(":", 1)
            self.header_depends = list(
                filter(lambda fname: fname.endswith(".h"), depends.split()),
            )
            self.header_dirs = sorted(
                set(map(os.path.normpath, map(os.path.dirname, self.header_depends)))
            )

    def _generate_sdk_meta(self):
        filtered_paths = [self.target_sdk_dir]
        full_fw_paths = list(
            map(
                os.path.normpath,
                (self.env.Dir(inc_dir).relpath for inc_dir in self.env["CPPPATH"]),
            )
        )

        sdk_dirs = ", ".join(f"'{dir}'" for dir in self.header_dirs)
        for dir in full_fw_paths:
            if dir in sdk_dirs:
                filtered_paths.append(
                    posixpath.normpath(posixpath.join(self.target_sdk_dir, dir))
                )

        sdk_env = self.env.Clone()
        sdk_env.Replace(CPPPATH=filtered_paths)
        with open(self.target[0].path, "wt") as f:
            cmdline_options = sdk_env.subst(
                "$CCFLAGS $_CCCOMCOM", target=Entry("dummy")
            )
            f.write(cmdline_options.replace("\\", "/"))
            f.write("\n")

    def _create_deploy_commands(self):
        dirs_to_create = set(
            self.sdk_deploy_dir.Dir(dirpath) for dirpath in self.header_dirs
        )
        actions = [
            Delete(self.sdk_deploy_dir),
            Mkdir(self.sdk_deploy_dir),
        ]
        actions += [Mkdir(d) for d in dirs_to_create]

        actions += [
            Copy(
                self.sdk_deploy_dir.File(h).path,
                h,
            )
            for h in self.header_depends
        ]
        return actions

    def generate_actions(self):
        self._parse_sdk_depends()
        self._generate_sdk_meta()

        return self._create_deploy_commands()


def deploy_sdk_tree(target, source, env, for_signature):
    if for_signature:
        return []

    sdk_tree = SdkTreeBuilder(env, target, source)
    return sdk_tree.generate_actions()


def gen_sdk_data(sdk_cache: SdkCache):
    api_def = []
    api_def.extend(
        (f"#include <{h.name}>" for h in sdk_cache.get_headers()),
    )

    api_def.append(f"const int elf_api_version = {sdk_cache.version.as_int()};")

    api_def.append(
        "static constexpr auto elf_api_table = sort(create_array_t<sym_entry>("
    )

    api_lines = []
    for fun_def in sdk_cache.get_functions():
        api_lines.append(
            f"API_METHOD({fun_def.name}, {fun_def.returns}, ({fun_def.params}))"
        )

    for var_def in sdk_cache.get_variables():
        api_lines.append(f"API_VARIABLE({var_def.name}, {var_def.var_type })")

    api_def.append(",\n".join(api_lines))

    api_def.append("));")
    return api_def


def _check_sdk_is_up2date(sdk_cache: SdkCache):
    if not sdk_cache.is_buildable():
        raise UserError(
            "SDK version is not finalized, please review changes and re-run operation"
        )


def validate_sdk_cache(source, target, env):
    # print(f"Generating SDK for {source[0]} to {target[0]}")
    current_sdk = SdkCollector()
    current_sdk.process_source_file_for_sdk(source[0].path)
    for h in env["SDK_HEADERS"]:
        current_sdk.add_header_to_sdk(pathlib.Path(h.path).as_posix())

    sdk_cache = SdkCache(target[0].path)
    sdk_cache.validate_api(current_sdk.get_api())
    sdk_cache.save()
    _check_sdk_is_up2date(sdk_cache)


def generate_sdk_symbols(source, target, env):
    sdk_cache = SdkCache(source[0].path)
    _check_sdk_is_up2date(sdk_cache)

    api_def = gen_sdk_data(sdk_cache)
    with open(target[0].path, "wt") as f:
        f.write("\n".join(api_def))


def generate(env, **kw):
    env.Append(
        BUILDERS={
            "SDKPrebuilder": Builder(
                emitter=prebuild_sdk_emitter,
                action=[
                    Action(
                        prebuild_sdk_create_origin_file,
                        "$SDK_PREGEN_COMSTR",
                    ),
                    Action(
                        "$CC -o $TARGET -E -P $CCFLAGS $_CCCOMCOM $SDK_PP_FLAGS -MMD ${TARGET}.c",
                        "$SDK_COMSTR",
                    ),
                ],
                suffix=".i",
            ),
            "SDKTree": Builder(
                generator=deploy_sdk_tree,
                src_suffix=".d",
            ),
            "SDKSymUpdater": Builder(
                action=Action(
                    validate_sdk_cache,
                    "$SDKSYM_UPDATER_COMSTR",
                ),
                suffix=".csv",
                src_suffix=".i",
            ),
            "SDKSymGenerator": Builder(
                action=Action(
                    generate_sdk_symbols,
                    "$SDKSYM_GENERATOR_COMSTR",
                ),
                suffix=".h",
                src_suffix=".csv",
            ),
        }
    )


def exists(env):
    return True
