import itertools
import os
import pathlib
import shutil
from dataclasses import dataclass, field
from typing import Optional, TypedDict

from ansi.color import fg

import SCons.Warnings
from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Errors import UserError
from SCons.Node import NodeList
from SCons.Node.FS import File, Entry

from fbt.appmanifest import FlipperApplication, FlipperAppType, FlipperManifestException
from fbt.elfmanifest import assemble_manifest_data
from fbt.fapassets import FileBundler
from fbt.sdk.cache import SdkCache
from fbt.util import extract_abs_dir_path


@dataclass
class FlipperExternalAppInfo:
    app: FlipperApplication
    compact: Optional[File] = None
    debug: Optional[File] = None
    validator: Optional[Entry] = None
    # List of tuples (dist_to_sd, path)
    dist_entries: list[tuple[bool, str]] = field(default_factory=list)


class AppBuilder:
    def __init__(self, env, app):
        self.fw_env = env
        self.app = app
        self.ext_apps_work_dir = env.subst("$EXT_APPS_WORK_DIR")
        self.app_work_dir = os.path.join(self.ext_apps_work_dir, self.app.appid)
        self.app_alias = f"fap_{self.app.appid}"
        self.externally_built_files = []
        self.private_libs = []

    def build(self):
        self._setup_app_env()
        self._build_external_files()
        self._compile_assets()
        self._build_private_libs()
        return self._build_app()

    def _setup_app_env(self):
        self.app_env = self.fw_env.Clone(
            FAP_SRC_DIR=self.app._appdir, FAP_WORK_DIR=self.app_work_dir
        )
        self.app_env.VariantDir(self.app_work_dir, self.app._appdir, duplicate=False)

    def _build_external_files(self):
        if not self.app.fap_extbuild:
            return

        for external_file_def in self.app.fap_extbuild:
            self.externally_built_files.append(external_file_def.path)
            self.app_env.Alias(self.app_alias, external_file_def.path)
            self.app_env.AlwaysBuild(
                self.app_env.Command(
                    external_file_def.path,
                    None,
                    Action(
                        external_file_def.command,
                        "" if self.app_env["VERBOSE"] else "\tEXTCMD\t${TARGET}",
                    ),
                )
            )

    def _compile_assets(self):
        if not self.app.fap_icon_assets:
            return

        fap_icons = self.app_env.CompileIcons(
            self.app_env.Dir(self.app_work_dir),
            self.app._appdir.Dir(self.app.fap_icon_assets),
            icon_bundle_name=f"{self.app.fap_icon_assets_symbol if self.app.fap_icon_assets_symbol else self.app.appid }_icons",
        )
        self.app_env.Alias("_fap_icons", fap_icons)
        self.fw_env.Append(_APP_ICONS=[fap_icons])

    def _build_private_libs(self):
        for lib_def in self.app.fap_private_libs:
            self.private_libs.append(self._build_private_lib(lib_def))

    def _build_private_lib(self, lib_def):
        lib_src_root_path = os.path.join(self.app_work_dir, "lib", lib_def.name)
        self.app_env.AppendUnique(
            CPPPATH=list(
                self.app_env.Dir(lib_src_root_path)
                .Dir(incpath)
                .srcnode()
                .rfile()
                .abspath
                for incpath in lib_def.fap_include_paths
            ),
        )

        lib_sources = list(
            itertools.chain.from_iterable(
                self.app_env.GlobRecursive(source_type, lib_src_root_path)
                for source_type in lib_def.sources
            )
        )

        if len(lib_sources) == 0:
            raise UserError(f"No sources gathered for private library {lib_def}")

        private_lib_env = self.app_env.Clone()
        private_lib_env.AppendUnique(
            CCFLAGS=[
                *lib_def.cflags,
            ],
            CPPDEFINES=lib_def.cdefines,
            CPPPATH=list(
                map(
                    lambda cpath: extract_abs_dir_path(self.app._appdir.Dir(cpath)),
                    lib_def.cincludes,
                )
            ),
        )

        return private_lib_env.StaticLibrary(
            os.path.join(self.app_work_dir, lib_def.name),
            lib_sources,
        )

    def _build_app(self):
        self.app_env.Append(
            LIBS=[*self.app.fap_libs, *self.private_libs],
            CPPPATH=self.app_env.Dir(self.app_work_dir),
        )

        app_sources = list(
            itertools.chain.from_iterable(
                self.app_env.GlobRecursive(
                    source_type,
                    self.app_work_dir,
                    exclude="lib",
                )
                for source_type in self.app.sources
            )
        )

        app_artifacts = FlipperExternalAppInfo(self.app)
        app_artifacts.debug = self.app_env.Program(
            os.path.join(self.ext_apps_work_dir, f"{self.app.appid}_d"),
            app_sources,
            APP_ENTRY=self.app.entry_point,
        )[0]

        app_artifacts.compact = self.app_env.EmbedAppMetadata(
            os.path.join(self.ext_apps_work_dir, self.app.appid),
            app_artifacts.debug,
            APP=self.app,
        )[0]

        app_artifacts.validator = self.app_env.ValidateAppImports(
            app_artifacts.compact
        )[0]

        if self.app.apptype == FlipperAppType.PLUGIN:
            for parent_app_id in self.app.requires:
                fal_path = (
                    f"apps_data/{parent_app_id}/plugins/{app_artifacts.compact.name}"
                )
                deployable = True
                # If it's a plugin for a non-deployable app, don't include it in the resources
                if parent_app := self.app._appmanager.get(parent_app_id):
                    if not parent_app.is_default_deployable:
                        deployable = False
                app_artifacts.dist_entries.append((deployable, fal_path))
        else:
            fap_path = f"apps/{self.app.fap_category}/{app_artifacts.compact.name}"
            app_artifacts.dist_entries.append(
                (self.app.is_default_deployable, fap_path)
            )

        self._configure_deps_and_aliases(app_artifacts)
        return app_artifacts

    def _configure_deps_and_aliases(self, app_artifacts: FlipperExternalAppInfo):
        # Extra things to clean up along with the app
        self.app_env.Clean(
            app_artifacts.debug,
            [*self.externally_built_files, self.app_env.Dir(self.app_work_dir)],
        )

        # Create listing of the app
        app_elf_dump = self.app_env.ObjDump(app_artifacts.debug)
        self.app_env.Alias(f"{self.app_alias}_list", app_elf_dump)

        # Extra dependencies for the app - manifest values, icon file
        manifest_vals = {
            k: v
            for k, v in vars(self.app).items()
            if not k.startswith(FlipperApplication.PRIVATE_FIELD_PREFIX)
        }

        self.app_env.Depends(
            app_artifacts.compact,
            [self.app_env["SDK_DEFINITION"], self.app_env.Value(manifest_vals)],
        )
        if self.app.fap_icon:
            self.app_env.Depends(
                app_artifacts.compact,
                self.app_env.File(f"{self.app._apppath}/{self.app.fap_icon}"),
            )

        # Add dependencies on file assets
        if self.app.fap_file_assets:
            self.app_env.Depends(
                app_artifacts.compact,
                self.app_env.GlobRecursive(
                    "*",
                    self.app._appdir.Dir(self.app.fap_file_assets),
                ),
            )

        # Always run the validator for the app's binary when building the app
        self.app_env.AlwaysBuild(app_artifacts.validator)
        self.app_env.Alias(self.app_alias, app_artifacts.validator)


def BuildAppElf(env, app):
    app_builder = AppBuilder(env, app)
    env["EXT_APPS"][app.appid] = app_artifacts = app_builder.build()
    return app_artifacts


def prepare_app_metadata(target, source, env):
    sdk_cache = SdkCache(env["SDK_DEFINITION"].path, load_version_only=True)

    if not sdk_cache.is_buildable():
        raise UserError(
            "SDK version is not finalized, please review changes and re-run operation. See AppsOnSDCard.md for more details."
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
            f"{source[0].path}: app may not be runnable. Symbols not resolved using firmware's API: "
        ) + fg.brightmagenta(f"{unresolved_syms}")
        disabled_api_syms = unresolved_syms.intersection(sdk_cache.get_disabled_names())
        if disabled_api_syms:
            warning_msg += (
                fg.brightyellow(" (in API, but disabled: ")
                + fg.brightmagenta(f"{disabled_api_syms}")
                + fg.brightyellow(")")
            )
        SCons.Warnings.warn(SCons.Warnings.LinkWarning, warning_msg),


def GetExtAppByIdOrPath(env, app_dir):
    if not app_dir:
        raise UserError("APPSRC= not set")

    appmgr = env["APPMGR"]

    app = None
    try:
        # Maybe user passed an appid?
        app = appmgr.get(app_dir)
    except FlipperManifestException as _:
        # Look up path components in known app dirs
        for dir_part in reversed(pathlib.Path(app_dir).parts):
            if app := appmgr.find_by_appdir(dir_part):
                break

    if not app:
        raise UserError(f"Failed to resolve application for given APPSRC={app_dir}")

    app_artifacts = env["EXT_APPS"].get(app.appid, None)
    if not app_artifacts:
        raise UserError(
            f"Application {app.appid} is not configured to be built as external"
        )

    return app_artifacts


def resources_fap_dist_emitter(target, source, env):
    # Initially we have a single target - target dir
    # Here we inject pairs of (target, source) for each file
    resources_root = target[0]

    target = []
    for app_artifacts in env["EXT_APPS"].values():
        for _, dist_path in filter(
            lambda dist_entry: dist_entry[0], app_artifacts.dist_entries
        ):
            source.append(app_artifacts.compact)
            target.append(resources_root.File(dist_path))

    assert len(target) == len(source)
    return (target, source)


def resources_fap_dist_action(target, source, env):
    # FIXME: find a proper way to remove stale files
    target_dir = env.Dir("${RESOURCES_ROOT}/apps")
    shutil.rmtree(target_dir.path, ignore_errors=True)

    # Iterate over pairs generated in emitter
    for src, target in zip(source, target):
        os.makedirs(os.path.dirname(target.path), exist_ok=True)
        shutil.copy(src.path, target.path)


def embed_app_metadata_emitter(target, source, env):
    app = env["APP"]

    # Hack: change extension for fap libs
    if app.apptype == FlipperAppType.PLUGIN:
        target[0].name = target[0].name.replace(".fap", ".fal")

    meta_file_name = source[0].path + ".meta"
    target.append("#" + meta_file_name)

    if app.fap_file_assets:
        files_section = source[0].path + ".files.section"
        target.append("#" + files_section)

    return (target, source)


def prepare_app_files(target, source, env):
    app = env["APP"]
    directory = app._appdir.Dir(app.fap_file_assets)
    if not directory.exists():
        raise UserError(f"File asset directory {directory} does not exist")

    bundler = FileBundler(directory.abspath)
    bundler.export(source[0].path + ".files.section")


def generate_embed_app_metadata_actions(source, target, env, for_signature):
    app = env["APP"]

    actions = [
        Action(prepare_app_metadata, "$APPMETA_COMSTR"),
    ]

    objcopy_str = (
        "${OBJCOPY} "
        "--remove-section .ARM.attributes "
        "--add-section .fapmeta=${SOURCE}.meta "
    )

    if app.fap_file_assets:
        actions.append(Action(prepare_app_files, "$APPFILE_COMSTR"))
        objcopy_str += "--add-section .fapassets=${SOURCE}.files.section "

    objcopy_str += (
        "--set-section-flags .fapmeta=contents,noload,readonly,data "
        "--strip-debug --strip-unneeded "
        "--add-gnu-debuglink=${SOURCE} "
        "${SOURCES} ${TARGET}"
    )

    actions.append(
        Action(
            objcopy_str,
            "$APPMETAEMBED_COMSTR",
        )
    )

    return Action(actions)


def generate(env, **kw):
    env.SetDefault(
        EXT_APPS_WORK_DIR="${FBT_FAP_DEBUG_ELF_ROOT}",
        APP_RUN_SCRIPT="${FBT_SCRIPT_DIR}/runfap.py",
        STORAGE_SCRIPT="${FBT_SCRIPT_DIR}/storage.py",
    )
    if not env["VERBOSE"]:
        env.SetDefault(
            FAPDISTCOMSTR="\tFAPDIST\t${TARGET}",
            APPMETA_COMSTR="\tAPPMETA\t${TARGET}",
            APPFILE_COMSTR="\tAPPFILE\t${TARGET}",
            APPMETAEMBED_COMSTR="\tFAP\t${TARGET}",
            APPCHECK_COMSTR="\tAPPCHK\t${SOURCE}",
        )

    env.SetDefault(
        EXT_APPS={},  # appid -> FlipperExternalAppInfo
        EXT_LIBS={},
        _APP_ICONS=[],
    )

    env.AddMethod(BuildAppElf)
    env.AddMethod(GetExtAppByIdOrPath)
    env.Append(
        BUILDERS={
            "FapDist": Builder(
                action=Action(
                    resources_fap_dist_action,
                    "$FAPDISTCOMSTR",
                ),
                emitter=resources_fap_dist_emitter,
            ),
            "EmbedAppMetadata": Builder(
                generator=generate_embed_app_metadata_actions,
                suffix=".fap",
                src_suffix=".elf",
                emitter=embed_app_metadata_emitter,
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
