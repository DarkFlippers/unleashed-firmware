import os
import re
from dataclasses import dataclass, field
from enum import Enum
from typing import Callable, ClassVar, List, Optional, Tuple, Union

try:
    from fbt.util import resolve_real_dir_node
except ImportError:
    # When running outside of SCons, we don't have access to SCons.Node
    def resolve_real_dir_node(node):
        return node


class FlipperManifestException(Exception):
    pass


class FlipperAppType(Enum):
    SERVICE = "Service"
    SYSTEM = "System"
    APP = "App"
    DEBUG = "Debug"
    ARCHIVE = "Archive"
    SETTINGS = "Settings"
    STARTUP = "StartupHook"
    EXTERNAL = "External"
    MENUEXTERNAL = "MenuExternal"
    METAPACKAGE = "Package"
    PLUGIN = "Plugin"


@dataclass
class FlipperApplication:
    APP_ID_REGEX: ClassVar[re.Pattern] = re.compile(r"^[a-z0-9_]+$")

    @dataclass
    class ExternallyBuiltFile:
        path: str
        command: str

    @dataclass
    class Library:
        name: str
        fap_include_paths: List[str] = field(default_factory=lambda: ["."])
        sources: List[str] = field(default_factory=lambda: ["*.c*"])
        cflags: List[str] = field(default_factory=list)
        cdefines: List[str] = field(default_factory=list)
        cincludes: List[str] = field(default_factory=list)

    PRIVATE_FIELD_PREFIX = "_"

    appid: str
    apptype: FlipperAppType
    name: Optional[str] = ""
    entry_point: Optional[str] = None
    flags: List[str] = field(default_factory=lambda: ["Default"])
    cdefines: List[str] = field(default_factory=list)
    requires: List[str] = field(default_factory=list)
    conflicts: List[str] = field(default_factory=list)
    provides: List[str] = field(default_factory=list)
    stack_size: int = 2048
    icon: Optional[str] = None
    order: int = 0
    sdk_headers: List[str] = field(default_factory=list)
    targets: List[str] = field(default_factory=lambda: ["all"])

    # .fap-specific
    sources: List[str] = field(default_factory=lambda: ["*.c*"])
    fap_version: Union[str, Tuple[int]] = "0.1"
    fap_icon: Optional[str] = None
    fap_libs: List[str] = field(default_factory=list)
    fap_category: str = ""
    fap_description: str = ""
    fap_author: str = ""
    fap_weburl: str = ""
    fap_icon_assets: Optional[str] = None
    fap_icon_assets_symbol: Optional[str] = None
    fap_extbuild: List[ExternallyBuiltFile] = field(default_factory=list)
    fap_private_libs: List[Library] = field(default_factory=list)
    fap_file_assets: Optional[str] = None
    fal_embedded: bool = False
    # Internally used by fbt
    _appmanager: Optional["AppManager"] = None
    _appdir: Optional[object] = None
    _apppath: Optional[str] = None
    _plugins: List["FlipperApplication"] = field(default_factory=list)
    _assets_dirs: List[object] = field(default_factory=list)
    _section_fapmeta: Optional[object] = None
    _section_fapfileassets: Optional[object] = None

    @property
    def embeds_plugins(self):
        return any(plugin.fal_embedded for plugin in self._plugins)

    def supports_hardware_target(self, target: str):
        return target in self.targets or "all" in self.targets

    @property
    def is_default_deployable(self):
        return self.apptype != FlipperAppType.DEBUG and self.fap_category != "Examples"

    def __post_init__(self):
        if self.apptype == FlipperAppType.PLUGIN:
            self.stack_size = 0
        if not self.APP_ID_REGEX.match(self.appid):
            raise FlipperManifestException(
                f"Invalid appid '{self.appid}'. Must match regex '{self.APP_ID_REGEX}'"
            )
        if isinstance(self.fap_version, str):
            try:
                self.fap_version = tuple(int(v) for v in self.fap_version.split("."))
            except ValueError:
                raise FlipperManifestException(
                    f"Invalid version string '{self.fap_version}'. Must be in the form 'major.minor'"
                )


class AppManager:
    def __init__(self):
        self.known_apps = {}

    def get(self, appname: str):
        try:
            return self.known_apps[appname]
        except KeyError:
            raise FlipperManifestException(
                f"Missing application manifest for '{appname}'"
            )

    def find_by_appdir(self, appdir: str):
        for app in self.known_apps.values():
            if app._appdir.name == appdir:
                return app
        return None

    def _validate_app_params(self, *args, **kw):
        apptype = kw.get("apptype")
        if apptype == FlipperAppType.PLUGIN:
            if kw.get("stack_size"):
                raise FlipperManifestException(
                    f"Plugin {kw.get('appid')} cannot have stack (did you mean FlipperAppType.EXTERNAL?)"
                )
            if not kw.get("requires"):
                raise FlipperManifestException(
                    f"Plugin {kw.get('appid')} must have 'requires' in manifest"
                )
        else:
            if kw.get("fal_embedded"):
                raise FlipperManifestException(
                    f"App {kw.get('appid')} cannot have fal_embedded set"
                )
        # Harmless - cdefines for external apps are meaningless
        # if apptype == FlipperAppType.EXTERNAL and kw.get("cdefines"):
        #     raise FlipperManifestException(
        #         f"External app {kw.get('appid')} must not have 'cdefines' in manifest"
        #     )

    def load_manifest(self, app_manifest_path: str, app_dir_node: object):
        if not os.path.exists(app_manifest_path):
            raise FlipperManifestException(
                f"App manifest not found at path {app_manifest_path}"
            )
        # print("Loading", app_manifest_path)

        app_manifests = []

        def App(*args, **kw):
            nonlocal app_manifests
            self._validate_app_params(*args, **kw)
            app_manifests.append(
                FlipperApplication(
                    *args,
                    **kw,
                    _appdir=resolve_real_dir_node(app_dir_node),
                    _apppath=os.path.dirname(app_manifest_path),
                    _appmanager=self,
                ),
            )

        def ExtFile(*args, **kw):
            return FlipperApplication.ExternallyBuiltFile(*args, **kw)

        def Lib(*args, **kw):
            return FlipperApplication.Library(*args, **kw)

        try:
            with open(app_manifest_path, "rt") as manifest_file:
                exec(manifest_file.read())
        except Exception as e:
            raise FlipperManifestException(
                f"Failed parsing manifest '{app_manifest_path}' : {e}"
            )

        if len(app_manifests) == 0:
            raise FlipperManifestException(
                f"App manifest '{app_manifest_path}' is malformed"
            )

        # print("Built", app_manifests)
        for app in app_manifests:
            self._add_known_app(app)

    def _add_known_app(self, app: FlipperApplication):
        if self.known_apps.get(app.appid, None):
            raise FlipperManifestException(f"Duplicate app declaration: {app.appid}")
        self.known_apps[app.appid] = app

    def filter_apps(
        self,
        *,
        applist: List[str],
        ext_applist: List[str],
        hw_target: str,
    ):
        return AppBuildset(
            self,
            hw_target=hw_target,
            appnames=applist,
            extra_ext_appnames=ext_applist,
        )


class AppBuilderException(Exception):
    pass


class AppBuildset:
    BUILTIN_APP_TYPES = (
        FlipperAppType.SERVICE,
        FlipperAppType.SYSTEM,
        FlipperAppType.APP,
        FlipperAppType.DEBUG,
        FlipperAppType.ARCHIVE,
        FlipperAppType.SETTINGS,
        FlipperAppType.STARTUP,
    )
    EXTERNAL_APP_TYPES = (
        FlipperAppType.EXTERNAL,
        FlipperAppType.MENUEXTERNAL,
        FlipperAppType.PLUGIN,
        FlipperAppType.DEBUG,
    )

    @staticmethod
    def print_writer(message):
        print(message)

    def __init__(
        self,
        appmgr: AppManager,
        hw_target: str,
        appnames: List[str],
        *,
        extra_ext_appnames: List[str],
        message_writer: Callable | None = None,
    ):
        self.appmgr = appmgr
        self.appnames = set(appnames)
        self.incompatible_extapps, self.extapps = [], []
        self._extra_ext_appnames = extra_ext_appnames
        self._orig_appnames = appnames
        self.hw_target = hw_target
        self._writer = message_writer if message_writer else self.print_writer
        self._process_deps()
        self._process_ext_apps()
        self._check_conflicts()
        self._check_unsatisfied()  # unneeded?
        self._check_target_match()
        self._group_plugins()
        self.apps = sorted(
            list(map(self.appmgr.get, self.appnames)),
            key=lambda app: app.appid,
        )

    def _is_missing_dep(self, dep_name: str):
        return dep_name not in self.appnames

    def _check_if_app_target_supported(self, app_name: str):
        return self.appmgr.get(app_name).supports_hardware_target(self.hw_target)

    def _get_app_depends(self, app_name: str) -> List[str]:
        app_def = self.appmgr.get(app_name)
        # Skip app if its target is not supported by the target we are building for
        if not self._check_if_app_target_supported(app_name):
            self._writer(
                f"Skipping {app_name} due to target mismatch (building for {self.hw_target}, app supports {app_def.targets}"
            )
            return []

        return list(
            filter(
                self._check_if_app_target_supported,
                filter(self._is_missing_dep, app_def.provides + app_def.requires),
            )
        )

    def _process_deps(self):
        while True:
            provided = []
            for app_name in self.appnames:
                provided.extend(self._get_app_depends(app_name))

            # print("provides round: ", provided)
            if len(provided) == 0:
                break
            self.appnames.update(provided)

    def _process_ext_apps(self):
        extapps = [
            app
            for apptype in self.EXTERNAL_APP_TYPES
            for app in self.get_apps_of_type(apptype, True)
        ]
        extapps.extend(map(self.appmgr.get, self._extra_ext_appnames))

        for app in extapps:
            (
                self.extapps
                if app.supports_hardware_target(self.hw_target)
                else self.incompatible_extapps
            ).append(app)

    def get_ext_apps(self):
        return self.extapps

    def get_incompatible_ext_apps(self):
        return self.incompatible_extapps

    def _check_conflicts(self):
        conflicts = []
        for app in self.appnames:
            if conflict_app_name := list(
                filter(
                    lambda dep_name: dep_name in self.appnames,
                    self.appmgr.get(app).conflicts,
                )
            ):
                conflicts.append((app, conflict_app_name))

        if len(conflicts):
            raise AppBuilderException(
                f"App conflicts for {', '.join(f'{conflict_dep[0]}: {conflict_dep[1]}' for conflict_dep in conflicts)}"
            )

    def _check_unsatisfied(self):
        unsatisfied = []
        for app in self.appnames:
            if missing_dep := list(
                filter(self._is_missing_dep, self.appmgr.get(app).requires)
            ):
                unsatisfied.append((app, missing_dep))

        if len(unsatisfied):
            raise AppBuilderException(
                f"Unsatisfied dependencies for {', '.join(f'{missing_dep[0]}: {missing_dep[1]}' for missing_dep in unsatisfied)}"
            )

    def _check_target_match(self):
        incompatible = []
        for app in self.appnames:
            if not self.appmgr.get(app).supports_hardware_target(self.hw_target):
                incompatible.append(app)

        if len(incompatible):
            raise AppBuilderException(
                f"Apps incompatible with target {self.hw_target}: {', '.join(incompatible)}"
            )

    def _group_plugins(self):
        known_extensions = self.get_apps_of_type(FlipperAppType.PLUGIN, all_known=True)
        for extension_app in known_extensions:
            for parent_app_id in extension_app.requires:
                try:
                    parent_app = self.appmgr.get(parent_app_id)
                    parent_app._plugins.append(extension_app)
                except FlipperManifestException:
                    self._writer(
                        f"Module {extension_app.appid} has unknown parent {parent_app_id}"
                    )

    def get_apps_cdefs(self):
        cdefs = set()
        for app in self.apps:
            cdefs.update(app.cdefines)
        return sorted(list(cdefs))

    def get_sdk_headers(self):
        sdk_headers = []
        for app in self.apps:
            sdk_headers.extend(
                [
                    src._appdir.File(header)
                    for src in [app, *app._plugins]
                    for header in src.sdk_headers
                ]
            )
        return sdk_headers

    def get_apps_of_type(self, apptype: FlipperAppType, all_known: bool = False):
        return sorted(
            filter(
                lambda app: app.apptype == apptype,
                self.appmgr.known_apps.values() if all_known else self.apps,
            ),
            key=lambda app: app.order,
        )

    def get_builtin_apps(self):
        return list(
            filter(lambda app: app.apptype in self.BUILTIN_APP_TYPES, self.apps)
        )

    def get_builtin_app_folders(self):
        return sorted(
            set(
                (app._appdir, source_type)
                for app in self.get_builtin_apps()
                for source_type in app.sources
            )
        )


class ApplicationsCGenerator:
    APP_TYPE_MAP = {
        FlipperAppType.SERVICE: ("FlipperInternalApplication", "FLIPPER_SERVICES"),
        FlipperAppType.SYSTEM: ("FlipperInternalApplication", "FLIPPER_SYSTEM_APPS"),
        FlipperAppType.APP: ("FlipperInternalApplication", "FLIPPER_APPS"),
        FlipperAppType.DEBUG: ("FlipperInternalApplication", "FLIPPER_DEBUG_APPS"),
        FlipperAppType.SETTINGS: (
            "FlipperInternalApplication",
            "FLIPPER_SETTINGS_APPS",
        ),
        FlipperAppType.STARTUP: (
            "FlipperInternalOnStartHook",
            "FLIPPER_ON_SYSTEM_START",
        ),
    }

    APP_EXTERNAL_TYPE = (
        "FlipperExternalApplication",
        "FLIPPER_EXTERNAL_APPS",
    )

    def __init__(self, buildset: AppBuildset, autorun_app: str = ""):
        self.buildset = buildset
        self.autorun = autorun_app

    def get_app_ep_forward(self, app: FlipperApplication):
        if app.apptype == FlipperAppType.STARTUP:
            return f"extern void {app.entry_point}();"
        return f"extern int32_t {app.entry_point}(void* p);"

    def get_app_descr(self, app: FlipperApplication):
        if app.apptype == FlipperAppType.STARTUP:
            return app.entry_point
        return f"""
    {{.app = {app.entry_point},
     .name = "{app.name}",
     .appid = "{app.appid}", 
     .stack_size = {app.stack_size},
     .icon = {f"&{app.icon}" if app.icon else "NULL"},
     .flags = {'|'.join(f"FlipperInternalApplicationFlag{flag}" for flag in app.flags)} }}"""

    def get_external_app_descr(self, app: FlipperApplication):
        app_path = "/ext/apps"
        if app.fap_category:
            app_path += f"/{app.fap_category}"
        app_path += f"/{app.appid}.fap"
        return f"""
    {{
     .name = "{app.name}",
     .icon = {f"&{app.icon}" if app.icon else "NULL"},
     .path = "{app_path}" }}"""

    def generate(self):
        contents = [
            '#include "applications.h"',
            "#include <assets_icons.h>",
            f'const char* FLIPPER_AUTORUN_APP_NAME = "{self.autorun}";',
        ]
        for apptype in self.APP_TYPE_MAP:
            contents.extend(
                map(self.get_app_ep_forward, self.buildset.get_apps_of_type(apptype))
            )
            entry_type, entry_block = self.APP_TYPE_MAP[apptype]
            contents.append(f"const {entry_type} {entry_block}[] = {{")
            contents.append(
                ",\n".join(
                    map(self.get_app_descr, self.buildset.get_apps_of_type(apptype))
                )
            )
            contents.append("};")
            contents.append(
                f"const size_t {entry_block}_COUNT = COUNT_OF({entry_block});"
            )

        archive_app = self.buildset.get_apps_of_type(FlipperAppType.ARCHIVE)
        if archive_app:
            contents.extend(
                [
                    self.get_app_ep_forward(archive_app[0]),
                    f"const FlipperInternalApplication FLIPPER_ARCHIVE = {self.get_app_descr(archive_app[0])};",
                ]
            )

        entry_type, entry_block = self.APP_EXTERNAL_TYPE
        external_apps = self.buildset.get_apps_of_type(FlipperAppType.MENUEXTERNAL)
        contents.append(f"const {entry_type} {entry_block}[] = {{")
        contents.append(",\n".join(map(self.get_external_app_descr, external_apps)))
        contents.append("};")
        contents.append(f"const size_t {entry_block}_COUNT = COUNT_OF({entry_block});")

        return "\n".join(contents)
