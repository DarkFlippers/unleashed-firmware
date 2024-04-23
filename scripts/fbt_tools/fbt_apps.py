from ansi.color import fg
from fbt.appmanifest import (
    AppManager,
    AppBuildset,
    FlipperApplication,
    FlipperAppType,
    FlipperManifestException,
)
from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Errors import StopError
from SCons.Script import GetOption
from SCons.Warnings import WarningOnByDefault, warn

# Adding objects for application management to env
#  AppManager env["APPMGR"] - loads all manifests; manages list of known apps
#  AppBuildset env["APPBUILD"] - contains subset of apps, filtered for current config


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
            return f"extern void {app.entry_point}(void);"
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


def LoadAppManifest(env, entry):
    try:
        manifest_glob = entry.glob(FlipperApplication.APP_MANIFEST_DEFAULT_NAME)
        if len(manifest_glob) == 0:
            try:
                disk_node = next(filter(lambda d: d.exists(), entry.get_all_rdirs()))
            except Exception:
                disk_node = entry

            raise FlipperManifestException(
                f"App folder '{disk_node.abspath}': missing manifest ({FlipperApplication.APP_MANIFEST_DEFAULT_NAME})"
            )

        app_manifest_file_path = manifest_glob[0].rfile().abspath
        env["APPMGR"].load_manifest(app_manifest_file_path, entry)
    except FlipperManifestException as e:
        if not GetOption("silent"):
            warn(WarningOnByDefault, str(e))


def PrepareApplicationsBuild(env):
    try:
        appbuild = env["APPBUILD"] = env["APPMGR"].filter_apps(
            applist=env["APPS"],
            ext_applist=env["EXTRA_EXT_APPS"],
            hw_target=env.subst("f${TARGET_HW}"),
        )
    except Exception as e:
        raise StopError(e)

    env.Append(
        SDK_HEADERS=appbuild.get_sdk_headers(),
    )


def DumpApplicationConfig(target, source, env):
    print(f"Loaded {len(env['APPMGR'].known_apps)} app definitions.")
    print(fg.boldgreen("Firmware modules configuration:"))
    for apptype in FlipperAppType:
        app_sublist = env["APPBUILD"].get_apps_of_type(apptype)
        if app_sublist:
            print(
                fg.green(f"{apptype.value}:\n\t"),
                ", ".join(app.appid for app in app_sublist),
            )
    if incompatible_ext_apps := env["APPBUILD"].get_incompatible_ext_apps():
        print(
            fg.blue("Incompatible apps (skipped):\n\t"),
            ", ".join(app.appid for app in incompatible_ext_apps),
        )


def build_apps_c(target, source, env):
    target_file_name = target[0].path

    gen = ApplicationsCGenerator(env["APPBUILD"], env.subst("$LOADER_AUTOSTART"))
    with open(target_file_name, "w") as file:
        file.write(gen.generate())


def generate(env):
    env.AddMethod(LoadAppManifest)
    env.AddMethod(PrepareApplicationsBuild)
    env.SetDefault(
        APPMGR=AppManager(),
        APPBUILD_DUMP=env.Action(
            DumpApplicationConfig,
            "\tINFO\t",
        ),
    )

    env.Append(
        BUILDERS={
            "ApplicationsC": Builder(
                action=Action(
                    build_apps_c,
                    "${APPSCOMSTR}",
                ),
                suffix=".c",
            ),
        }
    )


def exists(env):
    return True
