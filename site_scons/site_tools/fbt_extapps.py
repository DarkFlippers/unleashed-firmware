from SCons.Builder import Builder
from SCons.Action import Action
from SCons.Errors import UserError
import SCons.Warnings

import os
import pathlib
from fbt.elfmanifest import assemble_manifest_data
from fbt.sdk import SdkCache
import itertools


def BuildAppElf(env, app):
    work_dir = env.subst("$EXT_APPS_WORK_DIR")

    app_alias = f"{env['FIRMWARE_BUILD_CFG']}_{app.appid}"
    app_original_elf = os.path.join(work_dir, f"{app.appid}_d")
    app_sources = list(
        itertools.chain.from_iterable(
            env.GlobRecursive(source_type, os.path.join(work_dir, app._appdir.relpath))
            for source_type in app.sources
        )
    )
    app_elf_raw = env.Program(
        app_original_elf,
        app_sources,
        APP_ENTRY=app.entry_point,
        LIBS=env["LIBS"] + app.fap_libs,
    )

    app_elf_dump = env.ObjDump(app_elf_raw)
    env.Alias(f"{app_alias}_list", app_elf_dump)

    app_elf_augmented = env.EmbedAppMetadata(
        os.path.join(env.subst("$PLUGIN_ELF_DIR"), app.appid),
        app_elf_raw,
        APP=app,
    )

    env.Depends(app_elf_augmented, [env["SDK_DEFINITION"], env.Value(app)])
    if app.fap_icon:
        env.Depends(
            app_elf_augmented,
            env.File(f"{app._apppath}/{app.fap_icon}"),
        )
    env.Alias(app_alias, app_elf_augmented)

    app_elf_import_validator = env.ValidateAppImports(app_elf_augmented)
    env.AlwaysBuild(app_elf_import_validator)
    return (app_elf_augmented, app_elf_raw, app_elf_import_validator)


def prepare_app_metadata(target, source, env):
    sdk_cache = SdkCache(env.subst("$SDK_DEFINITION"), load_version_only=True)

    if not sdk_cache.is_buildable():
        raise UserError(
            "SDK version is not finalized, please review changes and re-run operation"
        )

    app = env["APP"]
    meta_file_name = source[0].path + ".meta"
    with open(meta_file_name, "wb") as f:
        # f.write(f"hello this is {app}")
        f.write(
            assemble_manifest_data(
                app_manifest=app,
                hardware_target=int(env.subst("$TARGET_HW")),
                sdk_version=sdk_cache.version.as_int(),
            )
        )


def validate_app_imports(target, source, env):
    sdk_cache = SdkCache(env.subst("$SDK_DEFINITION"), load_version_only=False)
    app_syms = set()
    with open(target[0].path, "rt") as f:
        for line in f:
            app_syms.add(line.split()[0])
    unresolved_syms = app_syms - sdk_cache.get_valid_names()
    if unresolved_syms:
        SCons.Warnings.warn(
            SCons.Warnings.LinkWarning,
            f"{source[0].path}: app won't run. Unresolved symbols: {unresolved_syms}",
        )


def GetExtAppFromPath(env, app_dir):
    if not app_dir:
        raise UserError("APPSRC= not set")

    appmgr = env["APPMGR"]

    app = None
    for dir_part in reversed(pathlib.Path(app_dir).parts):
        if app := appmgr.find_by_appdir(dir_part):
            break
    if not app:
        raise UserError(f"Failed to resolve application for given APPSRC={app_dir}")

    app_elf = env["_extapps"]["compact"].get(app.appid, None)
    if not app_elf:
        raise UserError(f"No external app found for {app.appid}")

    return (app, app_elf[0])


def generate(env, **kw):
    env.SetDefault(EXT_APPS_WORK_DIR=kw.get("EXT_APPS_WORK_DIR"))
    env.VariantDir(env.subst("$EXT_APPS_WORK_DIR"), env.Dir("#"), duplicate=False)

    env.AddMethod(BuildAppElf)
    env.AddMethod(GetExtAppFromPath)
    env.Append(
        BUILDERS={
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
                        None,  # "$APPCHECK_COMSTR",
                    ),
                ],
                suffix=".impsyms",
                src_suffix=".fap",
            ),
        }
    )


def exists(env):
    return True
