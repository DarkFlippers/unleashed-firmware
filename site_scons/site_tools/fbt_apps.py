from SCons.Builder import Builder
from SCons.Action import Action
from SCons.Warnings import warn, WarningOnByDefault
import SCons
import os.path

from fbt.appmanifest import (
    FlipperAppType,
    AppManager,
    ApplicationsCGenerator,
    FlipperManifestException,
)

# Adding objects for application management to env
#  AppManager env["APPMGR"] - loads all manifests; manages list of known apps
#  AppBuildset env["APPBUILD"] - contains subset of apps, filtered for current config


def LoadApplicationManifests(env):
    appmgr = env["APPMGR"] = AppManager()
    for entry in env.Glob("#/applications/*", ondisk=True, source=True):
        if isinstance(entry, SCons.Node.FS.Dir) and not str(entry).startswith("."):
            try:
                app_manifest_file_path = os.path.join(entry.abspath, "application.fam")
                appmgr.load_manifest(app_manifest_file_path, entry.name)
                env.Append(PY_LINT_SOURCES=[app_manifest_file_path])
            except FlipperManifestException as e:
                warn(WarningOnByDefault, str(e))


def PrepareApplicationsBuild(env):
    env["APPBUILD"] = env["APPMGR"].filter_apps(env["APPS"])
    env["APPBUILD_DUMP"] = env.Action(
        DumpApplicationConfig,
        "\tINFO\t",
    )


def DumpApplicationConfig(target, source, env):
    print(f"Loaded {len(env['APPMGR'].known_apps)} app definitions.")
    print("Firmware modules configuration:")
    for apptype in FlipperAppType:
        app_sublist = env["APPBUILD"].get_apps_of_type(apptype)
        if app_sublist:
            print(
                f"{apptype.value}:\n\t",
                ", ".join(app.appid for app in app_sublist),
            )


def build_apps_c(target, source, env):
    target_file_name = target[0].path

    gen = ApplicationsCGenerator(env["APPBUILD"])
    with open(target_file_name, "w") as file:
        file.write(gen.generate())


def generate(env):
    env.AddMethod(LoadApplicationManifests)
    env.AddMethod(PrepareApplicationsBuild)

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
