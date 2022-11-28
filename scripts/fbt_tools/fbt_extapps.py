from dataclasses import dataclass, field
from typing import Optional
from SCons.Builder import Builder
from SCons.Action import Action
from SCons.Errors import UserError
from SCons.Node import NodeList
import SCons.Warnings

from fbt.elfmanifest import assemble_manifest_data
from fbt.appmanifest import FlipperApplication, FlipperManifestException
from fbt.sdk.cache import SdkCache
from fbt.util import extract_abs_dir_path

import os
import pathlib
import itertools
import shutil

from ansi.color import fg


@dataclass
class FlipperExternalAppInfo:
    app: FlipperApplication
    compact: NodeList = field(default_factory=NodeList)
    debug: NodeList = field(default_factory=NodeList)
    validator: NodeList = field(default_factory=NodeList)
    installer: NodeList = field(default_factory=NodeList)


def BuildAppElf(env, app):
    ext_apps_work_dir = env.subst("$EXT_APPS_WORK_DIR")
    app_work_dir = os.path.join(ext_apps_work_dir, app.appid)

    env.VariantDir(app_work_dir, app._appdir, duplicate=False)

    app_env = env.Clone(FAP_SRC_DIR=app._appdir, FAP_WORK_DIR=app_work_dir)

    app_alias = f"fap_{app.appid}"

    app_artifacts = FlipperExternalAppInfo(app)

    externally_built_files = []
    if app.fap_extbuild:
        for external_file_def in app.fap_extbuild:
            externally_built_files.append(external_file_def.path)
            app_env.Alias(app_alias, external_file_def.path)
            app_env.AlwaysBuild(
                app_env.Command(
                    external_file_def.path,
                    None,
                    Action(
                        external_file_def.command,
                        "" if app_env["VERBOSE"] else "\tEXTCMD\t${TARGET}",
                    ),
                )
            )

    if app.fap_icon_assets:
        fap_icons = app_env.CompileIcons(
            app_env.Dir(app_work_dir),
            app._appdir.Dir(app.fap_icon_assets),
            icon_bundle_name=f"{app.fap_icon_assets_symbol if app.fap_icon_assets_symbol else app.appid }_icons",
        )
        app_env.Alias("_fap_icons", fap_icons)

    private_libs = []

    for lib_def in app.fap_private_libs:
        lib_src_root_path = os.path.join(app_work_dir, "lib", lib_def.name)
        app_env.AppendUnique(
            CPPPATH=list(
                app_env.Dir(lib_src_root_path).Dir(incpath).srcnode().rfile().abspath
                for incpath in lib_def.fap_include_paths
            ),
        )

        lib_sources = list(
            itertools.chain.from_iterable(
                app_env.GlobRecursive(source_type, lib_src_root_path)
                for source_type in lib_def.sources
            )
        )
        if len(lib_sources) == 0:
            raise UserError(f"No sources gathered for private library {lib_def}")

        private_lib_env = app_env.Clone()
        private_lib_env.AppendUnique(
            CCFLAGS=[
                *lib_def.cflags,
            ],
            CPPDEFINES=lib_def.cdefines,
            CPPPATH=list(
                map(
                    lambda cpath: extract_abs_dir_path(app._appdir.Dir(cpath)),
                    lib_def.cincludes,
                )
            ),
        )

        lib = private_lib_env.StaticLibrary(
            os.path.join(app_work_dir, lib_def.name),
            lib_sources,
        )
        private_libs.append(lib)

    app_sources = list(
        itertools.chain.from_iterable(
            app_env.GlobRecursive(
                source_type,
                app_work_dir,
                exclude="lib",
            )
            for source_type in app.sources
        )
    )

    app_env.Append(
        LIBS=[*app.fap_libs, *private_libs],
        CPPPATH=env.Dir(app_work_dir),
    )

    app_artifacts.debug = app_env.Program(
        os.path.join(ext_apps_work_dir, f"{app.appid}_d"),
        app_sources,
        APP_ENTRY=app.entry_point,
    )

    app_env.Clean(
        app_artifacts.debug, [*externally_built_files, app_env.Dir(app_work_dir)]
    )

    app_elf_dump = app_env.ObjDump(app_artifacts.debug)
    app_env.Alias(f"{app_alias}_list", app_elf_dump)

    app_artifacts.compact = app_env.EmbedAppMetadata(
        os.path.join(ext_apps_work_dir, app.appid),
        app_artifacts.debug,
        APP=app,
    )

    manifest_vals = {
        k: v
        for k, v in vars(app).items()
        if not k.startswith(FlipperApplication.PRIVATE_FIELD_PREFIX)
    }

    app_env.Depends(
        app_artifacts.compact,
        [app_env["SDK_DEFINITION"], app_env.Value(manifest_vals)],
    )
    if app.fap_icon:
        app_env.Depends(
            app_artifacts.compact,
            app_env.File(f"{app._apppath}/{app.fap_icon}"),
        )

    app_artifacts.validator = app_env.ValidateAppImports(app_artifacts.compact)
    app_env.AlwaysBuild(app_artifacts.validator)
    app_env.Alias(app_alias, app_artifacts.validator)

    env["EXT_APPS"][app.appid] = app_artifacts
    return app_artifacts


def prepare_app_metadata(target, source, env):
    sdk_cache = SdkCache(env["SDK_DEFINITION"].path, load_version_only=True)

    if not sdk_cache.is_buildable():
        raise UserError(
            "SDK version is not finalized, please review changes and re-run operation"
        )

    app = env["APP"]
    meta_file_name = source[0].path + ".meta"
    with open(meta_file_name, "wb") as f:
        f.write(
            assemble_manifest_data(
                app_manifest=app,
                hardware_target=int(env.subst("$TARGET_HW")),
                sdk_version=sdk_cache.version.as_int(),
            )
        )


def validate_app_imports(target, source, env):
    sdk_cache = SdkCache(env["SDK_DEFINITION"].path, load_version_only=False)
    app_syms = set()
    with open(target[0].path, "rt") as f:
        for line in f:
            app_syms.add(line.split()[0])
    unresolved_syms = app_syms - sdk_cache.get_valid_names()
    if unresolved_syms:
        warning_msg = fg.brightyellow(
            f"{source[0].path}: app won't run. Unresolved symbols: "
        ) + fg.brightmagenta(f"{unresolved_syms}")
        disabled_api_syms = unresolved_syms.intersection(sdk_cache.get_disabled_names())
        if disabled_api_syms:
            warning_msg += (
                fg.brightyellow(" (in API, but disabled: ")
                + fg.brightmagenta(f"{disabled_api_syms}")
                + fg.brightyellow(")")
            )
        SCons.Warnings.warn(SCons.Warnings.LinkWarning, warning_msg),


def GetExtAppFromPath(env, app_dir):
    if not app_dir:
        raise UserError("APPSRC= not set")

    appmgr = env["APPMGR"]

    app = None
    try:
        # Maybe used passed an appid?
        app = appmgr.get(app_dir)
    except FlipperManifestException as _:
        # Look up path components in known app dits
        for dir_part in reversed(pathlib.Path(app_dir).parts):
            if app := appmgr.find_by_appdir(dir_part):
                break

    if not app:
        raise UserError(f"Failed to resolve application for given APPSRC={app_dir}")

    app_artifacts = env["EXT_APPS"].get(app.appid, None)
    if not app_artifacts:
        raise UserError(
            f"Application {app.appid} is not configured for building as external"
        )

    return app_artifacts


def fap_dist_emitter(target, source, env):
    target_dir = target[0]

    target = []
    for _, app_artifacts in env["EXT_APPS"].items():
        source.extend(app_artifacts.compact)
        target.append(
            target_dir.Dir(app_artifacts.app.fap_category).File(
                app_artifacts.compact[0].name
            )
        )

    return (target, source)


def fap_dist_action(target, source, env):
    # FIXME
    target_dir = env.Dir("#/assets/resources/apps")

    shutil.rmtree(target_dir.path, ignore_errors=True)
    for src, target in zip(source, target):
        os.makedirs(os.path.dirname(target.path), exist_ok=True)
        shutil.copy(src.path, target.path)


def generate(env, **kw):
    env.SetDefault(
        EXT_APPS_WORK_DIR="${FBT_FAP_DEBUG_ELF_ROOT}",
        APP_RUN_SCRIPT="${FBT_SCRIPT_DIR}/runfap.py",
    )
    if not env["VERBOSE"]:
        env.SetDefault(
            FAPDISTCOMSTR="\tFAPDIST\t${TARGET}",
            APPMETA_COMSTR="\tAPPMETA\t${TARGET}",
            APPMETAEMBED_COMSTR="\tFAP\t${TARGET}",
            APPCHECK_COMSTR="\tAPPCHK\t${SOURCE}",
        )

    env.SetDefault(
        EXT_APPS={},  # appid -> FlipperExternalAppInfo
    )

    env.AddMethod(BuildAppElf)
    env.AddMethod(GetExtAppFromPath)
    env.Append(
        BUILDERS={
            "FapDist": Builder(
                action=Action(
                    fap_dist_action,
                    "$FAPDISTCOMSTR",
                ),
                emitter=fap_dist_emitter,
            ),
            "EmbedAppMetadata": Builder(
                action=[
                    Action(prepare_app_metadata, "$APPMETA_COMSTR"),
                    Action(
                        "${OBJCOPY} "
                        "--remove-section .ARM.attributes "
                        "--add-section .fapmeta=${SOURCE}.meta "
                        "--set-section-flags .fapmeta=contents,noload,readonly,data "
                        "--strip-debug --strip-unneeded "
                        "--add-gnu-debuglink=${SOURCE} "
                        "${SOURCES} ${TARGET}",
                        "$APPMETAEMBED_COMSTR",
                    ),
                ],
                suffix=".fap",
                src_suffix=".elf",
            ),
            "ValidateAppImports": Builder(
                action=[
                    Action(
                        "@${NM} -P -u ${SOURCE} > ${TARGET}",
                        None,  # "$APPDUMP_COMSTR",
                    ),
                    Action(
                        validate_app_imports,
                        "$APPCHECK_COMSTR",
                    ),
                ],
                suffix=".impsyms",
                src_suffix=".fap",
            ),
        }
    )


def exists(env):
    return True
