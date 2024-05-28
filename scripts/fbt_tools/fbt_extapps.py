import itertools
import pathlib
from dataclasses import dataclass, field
from typing import Dict, List, Optional

import SCons.Warnings
from ansi.color import fg
from fbt.appmanifest import FlipperApplication, FlipperAppType, FlipperManifestException
from fbt.elfmanifest import assemble_manifest_data
from fbt.fapassets import FileBundler
from fbt.sdk.cache import SdkCache
from fbt.util import resolve_real_dir_node
from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Errors import UserError
from SCons.Node.FS import Entry, File

_FAP_META_SECTION = ".fapmeta"
_FAP_FILEASSETS_SECTION = ".fapassets"


@dataclass
class FlipperExternalAppInfo:
    app: FlipperApplication
    compact: Optional[File] = None
    debug: Optional[File] = None
    validator: Optional[Entry] = None
    # List of tuples (dist_to_sd, path)
    dist_entries: list[tuple[bool, str]] = field(default_factory=list)


class AppBuilder:
    @staticmethod
    def get_app_work_dir(env, app):
        return env["EXT_APPS_WORK_DIR"].Dir(app.appid)

    def __init__(self, env, app):
        self.fw_env = env
        self.app = app
        self.ext_apps_work_dir = env["EXT_APPS_WORK_DIR"]
        self.app_work_dir = self.get_app_work_dir(env, app)
        self.app_alias = f"fap_{self.app.appid}"
        self.icons_src = None
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
            FAP_SRC_DIR=self.app._appdir,
            FAP_WORK_DIR=self.app_work_dir,
        )
        self.app_env.Append(
            CPPDEFINES=[
                ("FAP_VERSION", f'\\"{".".join(map(str, self.app.fap_version))}\\"'),
                *self.app.cdefines,
            ],
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
            self.app_work_dir,
            self.app._appdir.Dir(self.app.fap_icon_assets),
            icon_bundle_name=f"{self.app.fap_icon_assets_symbol or self.app.appid }_icons",
        )
        self.app_env.Alias("_fap_icons", fap_icons)
        self.fw_env.Append(_APP_ICONS=[fap_icons])
        self.icons_src = next(filter(lambda n: n.path.endswith(".c"), fap_icons))

    def _build_private_libs(self):
        for lib_def in self.app.fap_private_libs:
            self.private_libs.append(self._build_private_lib(lib_def))

    def _build_private_lib(self, lib_def):
        lib_src_root_path = self.app_work_dir.Dir("lib").Dir(lib_def.name)
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
            CCFLAGS=lib_def.cflags,
            CPPDEFINES=lib_def.cdefines,
            CPPPATH=list(
                map(
                    lambda cpath: resolve_real_dir_node(self.app._appdir.Dir(cpath)),
                    lib_def.cincludes,
                )
            ),
        )

        return private_lib_env.StaticLibrary(
            self.app_work_dir.File(lib_def.name),
            lib_sources,
        )

    def _build_app(self):
        if self.app.fap_file_assets:
            self.app._assets_dirs = [self.app._appdir.Dir(self.app.fap_file_assets)]

        self.app_env.Append(
            LIBS=[*self.app.fap_libs, *self.private_libs, *self.app.fap_libs],
            CPPPATH=[self.app_env.Dir(self.app_work_dir), self.app._appdir],
        )

        app_sources = self.app_env.GatherSources(
            [self.app.sources, "!lib"], self.app_work_dir
        )

        if not app_sources:
            raise UserError(f"No source files found for {self.app.appid}")

        # Ensure that icons are included in the build, regardless of user-configured sources
        if self.icons_src and not self.icons_src in app_sources:
            app_sources.append(self.icons_src)

        ## Uncomment for debug
        # print(f"App sources for {self.app.appid}: {list(f.path for f in app_sources)}")

        app_artifacts = FlipperExternalAppInfo(self.app)
        app_artifacts.debug = self.app_env.Program(
            self.ext_apps_work_dir.File(f"{self.app.appid}_d.elf"),
            app_sources,
            APP_ENTRY=self.app.entry_point,
        )[0]

        app_artifacts.compact = self.app_env.EmbedAppMetadata(
            self.ext_apps_work_dir.File(f"{self.app.appid}.fap"),
            app_artifacts.debug,
            APP=self.app,
        )[0]

        if self.app.embeds_plugins:
            self.app._assets_dirs.append(self.app_work_dir.Dir("assets"))

        app_artifacts.validator = self.app_env.ValidateAppImports(
            app_artifacts.compact,
            _CHECK_APP=self.app.do_strict_import_checks
            and self.app_env.get("STRICT_FAP_IMPORT_CHECK"),
        )[0]

        if self.app.apptype == FlipperAppType.PLUGIN:
            for parent_app_id in self.app.requires:
                if self.app.fal_embedded:
                    parent_app = self.app._appmanager.get(parent_app_id)
                    if not parent_app:
                        raise UserError(
                            f"Embedded plugin {self.app.appid} requires unknown app {parent_app_id}"
                        )
                    self.app_env.Install(
                        target=self.get_app_work_dir(self.app_env, parent_app)
                        .Dir("assets")
                        .Dir("plugins"),
                        source=app_artifacts.compact,
                    )
                else:
                    fal_path = f"apps_data/{parent_app_id}/plugins/{app_artifacts.compact.name}"
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
            [*self.externally_built_files, self.app_work_dir],
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
        for assets_dir in self.app._assets_dirs:
            glob_res = self.app_env.GlobRecursive("*", assets_dir)
            self.app_env.Depends(
                app_artifacts.compact,
                (*glob_res, assets_dir),
            )

        # Always run the validator for the app's binary when building the app
        self.app_env.AlwaysBuild(app_artifacts.validator)
        self.app_env.Alias(self.app_alias, app_artifacts.validator)


def BuildAppElf(env, app):
    app_builder = AppBuilder(env, app)
    env["EXT_APPS"][app.appid] = app_artifacts = app_builder.build()
    return app_artifacts


def prepare_app_metadata(target, source, env):
    metadata_node = next(filter(lambda t: t.name.endswith(_FAP_META_SECTION), target))

    sdk_cache = SdkCache(env["SDK_DEFINITION"].path, load_version_only=True)

    if not sdk_cache.is_buildable():
        raise UserError(
            "SDK version is not finalized, please review changes and re-run operation. See AppsOnSDCard.md for more details."
        )

    app = env["APP"]
    with open(metadata_node.abspath, "wb") as f:
        f.write(
            assemble_manifest_data(
                app_manifest=app,
                hardware_target=int(env.subst("$TARGET_HW")),
                sdk_version=sdk_cache.version.as_int(),
            )
        )


def _validate_app_imports(target, source, env):
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
        if env.get("_CHECK_APP"):
            raise UserError(warning_msg)
        else:
            SCons.Warnings.warn(SCons.Warnings.LinkWarning, warning_msg),


def GetExtAppByIdOrPath(env, app_dir):
    if not app_dir:
        raise UserError("APPSRC= not set")

    appmgr = env["APPMGR"]

    app = None
    try:
        # Maybe user passed an appid?
        app = appmgr.get(app_dir)
    except FlipperManifestException:
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


def _embed_app_metadata_emitter(target, source, env):
    app = env["APP"]

    # Hack: change extension for fap libs
    if app.apptype == FlipperAppType.PLUGIN:
        target[0].name = target[0].name.replace(".fap", ".fal")

    app_work_dir = AppBuilder.get_app_work_dir(env, app)
    app._section_fapmeta = app_work_dir.File(_FAP_META_SECTION)
    target.append(app._section_fapmeta)

    # At this point, we haven't added dir with embedded plugins to _assets_dirs yet
    if app._assets_dirs or app.embeds_plugins:
        app._section_fapfileassets = app_work_dir.File(_FAP_FILEASSETS_SECTION)
        target.append(app._section_fapfileassets)

    return (target, source)


def prepare_app_file_assets(target, source, env):
    files_section_node = next(
        filter(lambda t: t.name.endswith(_FAP_FILEASSETS_SECTION), target)
    )

    bundler = FileBundler(
        list(env.Dir(asset_dir).abspath for asset_dir in env["APP"]._assets_dirs)
    )
    bundler.export(files_section_node.abspath)


def generate_embed_app_metadata_actions(source, target, env, for_signature):
    app = env["APP"]

    actions = [
        Action(prepare_app_metadata, "$APPMETA_COMSTR"),
    ]

    objcopy_args = [
        "${OBJCOPY}",
        "--remove-section",
        ".ARM.attributes",
        "--add-section",
        "${_FAP_META_SECTION}=${APP._section_fapmeta}",
        "--set-section-flags",
        "${_FAP_META_SECTION}=contents,noload,readonly,data",
    ]

    if app._section_fapfileassets:
        actions.append(Action(prepare_app_file_assets, "$APPFILE_COMSTR"))
        objcopy_args.extend(
            (
                "--add-section",
                "${_FAP_FILEASSETS_SECTION}=${APP._section_fapfileassets}",
                "--set-section-flags",
                "${_FAP_FILEASSETS_SECTION}=contents,noload,readonly,data",
            )
        )

    objcopy_args.extend(
        (
            "--strip-debug",
            "--strip-unneeded",
            "--add-gnu-debuglink=${SOURCE}",
            "${SOURCES}",
            "${TARGET}",
        )
    )

    actions.extend(
        (
            Action(
                [objcopy_args],
                "$APPMETAEMBED_COMSTR",
            ),
            Action(
                [
                    [
                        "${PYTHON3}",
                        "${FBT_SCRIPT_DIR}/fastfap.py",
                        "${TARGET}",
                        "${OBJCOPY}",
                    ]
                ],
                "$FASTFAP_COMSTR",
            ),
        )
    )

    return Action(actions)


@dataclass
class AppDeploymentComponents:
    deploy_sources: Dict[str, object] = field(default_factory=dict)
    validators: List[object] = field(default_factory=list)
    extra_launch_args: str = ""

    def add_app(self, app_artifacts):
        for _, ext_path in app_artifacts.dist_entries:
            self.deploy_sources[f"/ext/{ext_path}"] = app_artifacts.compact
        self.validators.append(app_artifacts.validator)


def _gather_app_components(env, appname) -> AppDeploymentComponents:
    components = AppDeploymentComponents()

    def _add_host_app_to_targets(host_app):
        artifacts_app_to_run = env["EXT_APPS"].get(host_app.appid, None)
        components.add_app(artifacts_app_to_run)
        for plugin in host_app._plugins:
            components.add_app(env["EXT_APPS"].get(plugin.appid, None))

    artifacts_app_to_run = env.GetExtAppByIdOrPath(appname)
    if artifacts_app_to_run.app.apptype == FlipperAppType.PLUGIN:
        # We deploy host app instead
        host_app = env["APPMGR"].get(artifacts_app_to_run.app.requires[0])

        if host_app:
            if host_app.apptype in [
                FlipperAppType.EXTERNAL,
                FlipperAppType.MENUEXTERNAL,
            ]:
                components.add_app(host_app)
            else:
                # host app is a built-in app
                components.add_app(artifacts_app_to_run)
                if host_app.name:
                    components.extra_launch_args = f"-a {host_app.name}"
        else:
            raise UserError("Host app is unknown")
    else:
        _add_host_app_to_targets(artifacts_app_to_run.app)
    return components


def AddAppLaunchTarget(env, appname, launch_target_name):
    components = _gather_app_components(env, appname)
    target = env.PhonyTarget(
        launch_target_name,
        [
            [
                "${PYTHON3}",
                "${APP_RUN_SCRIPT}",
                "-p",
                "${FLIP_PORT}",
                "${EXTRA_ARGS}",
                "-s",
                "${SOURCES}",
                "-t",
                "${FLIPPER_FILE_TARGETS}",
            ]
        ],
        source=components.deploy_sources.values(),
        FLIPPER_FILE_TARGETS=components.deploy_sources.keys(),
        EXTRA_ARGS=components.extra_launch_args,
    )
    env.Alias(launch_target_name, components.validators)
    return target


def AddAppBuildTarget(env, appname, build_target_name):
    components = _gather_app_components(env, appname)
    env.Alias(build_target_name, components.validators)
    env.Alias(build_target_name, components.deploy_sources.values())


def generate(env, **kw):
    env.SetDefault(
        EXT_APPS_WORK_DIR=env.Dir(env["FBT_FAP_DEBUG_ELF_ROOT"]),
        APP_RUN_SCRIPT="${FBT_SCRIPT_DIR}/runfap.py",
    )
    if not env["VERBOSE"]:
        env.SetDefault(
            APPMETA_COMSTR="\tAPPMETA\t${TARGET}",
            APPFILE_COMSTR="\tAPPFILE\t${TARGET}",
            APPMETAEMBED_COMSTR="\tFAP\t${TARGET}",
            FASTFAP_COMSTR="\tFASTFAP\t${TARGET}",
            APPCHECK_COMSTR="\tAPPCHK\t${SOURCE}",
        )

    env.SetDefault(
        EXT_APPS={},  # appid -> FlipperExternalAppInfo
        EXT_LIBS={},
        _APP_ICONS=[],
        _FAP_META_SECTION=_FAP_META_SECTION,
        _FAP_FILEASSETS_SECTION=_FAP_FILEASSETS_SECTION,
    )

    env.AddMethod(BuildAppElf)
    env.AddMethod(GetExtAppByIdOrPath)
    env.AddMethod(AddAppLaunchTarget)
    env.AddMethod(AddAppBuildTarget)

    env.Append(
        BUILDERS={
            "EmbedAppMetadata": Builder(
                generator=generate_embed_app_metadata_actions,
                suffix=".fap",
                src_suffix=".elf",
                emitter=_embed_app_metadata_emitter,
            ),
            "ValidateAppImports": Builder(
                action=[
                    Action(
                        "@${NM} -P -u ${SOURCE} > ${TARGET}",
                        None,  # "$APPDUMP_COMSTR",
                    ),
                    Action(
                        _validate_app_imports,
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
