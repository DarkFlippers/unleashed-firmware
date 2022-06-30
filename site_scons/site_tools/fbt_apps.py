from SCons.Builder import Builder
from SCons.Action import Action

import SCons
from fbt.appmanifest import FlipperAppType, AppManager, ApplicationsCGenerator

# Adding objects for application management to env
#  AppManager env["APPMGR"] - loads all manifests; manages list of known apps
#  AppBuildset env["APPBUILD"] - contains subset of apps, filtered for current config


def LoadApplicationManifests(env):
    appmgr = env["APPMGR"] = AppManager()
    for entry in env.Glob("#/applications/*"):
        if isinstance(entry, SCons.Node.FS.Dir) and not str(entry).startswith("."):
            appmgr.load_manifest(entry.File("application.fam").abspath, entry.name)


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
            ),
        }
    )


def exists(env):
    return True
