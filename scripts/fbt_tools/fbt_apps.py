from ansi.color import fg
from fbt.appmanifest import (
    ApplicationsCGenerator,
    AppManager,
    FlipperAppType,
    FlipperManifestException,
)
from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Errors import StopError
from SCons.Warnings import WarningOnByDefault, warn
from SCons.Script import GetOption

# Adding objects for application management to env
#  AppManager env["APPMGR"] - loads all manifests; manages list of known apps
#  AppBuildset env["APPBUILD"] - contains subset of apps, filtered for current config


def LoadAppManifest(env, entry):
    try:
        APP_MANIFEST_NAME = "application.fam"
        manifest_glob = entry.glob(APP_MANIFEST_NAME)
        if len(manifest_glob) == 0:
            raise FlipperManifestException(
                f"Folder {entry}: manifest {APP_MANIFEST_NAME} is missing"
            )

        app_manifest_file_path = manifest_glob[0].rfile().abspath
        env["APPMGR"].load_manifest(app_manifest_file_path, entry)
        env.Append(PY_LINT_SOURCES=[app_manifest_file_path])
    except FlipperManifestException as e:
        if not GetOption("silent"):
            warn(WarningOnByDefault, str(e))


def PrepareApplicationsBuild(env):
    try:
        appbuild = env["APPBUILD"] = env["APPMGR"].filter_apps(
            env["APPS"], env.subst("f${TARGET_HW}")
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
