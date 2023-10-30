import os
import shutil

from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Errors import StopError
from SCons.Node.FS import Dir, File


def _resources_dist_emitter(target, source, env):
    resources_root = env.Dir(env["RESOURCES_ROOT"])

    target = []
    for app_artifacts in env["FW_EXTAPPS"].application_map.values():
        for _, dist_path in filter(
            lambda dist_entry: dist_entry[0], app_artifacts.dist_entries
        ):
            source.append(app_artifacts.compact)
            target.append(resources_root.File(dist_path))

    # Deploy apps' resources too
    for app in env["APPBUILD"].apps:
        if not app.resources:
            continue
        apps_resource_dir = app._appdir.Dir(app.resources)
        for res_file in env.GlobRecursive("*", apps_resource_dir):
            if not isinstance(res_file, File):
                continue
            source.append(res_file)
            target.append(resources_root.File(res_file.get_path(apps_resource_dir)))

    # Deploy other stuff from _EXTRA_DIST
    for extra_dist in env["_EXTRA_DIST"]:
        if isinstance(extra_dist, Dir):
            for extra_file in env.GlobRecursive("*", extra_dist):
                if not isinstance(extra_file, File):
                    continue
                source.append(extra_file)
                target.append(
                    # Preserve dir name from original node
                    resources_root.Dir(extra_dist.name).File(
                        extra_file.get_path(extra_dist)
                    )
                )
        else:
            raise StopError(f"Unsupported extra dist type: {type(extra_dist)}")

    assert len(target) == len(source)
    return (target, source)


def _resources_dist_action(target, source, env):
    shutil.rmtree(env.Dir(env["RESOURCES_ROOT"]).abspath, ignore_errors=True)
    for src, target in zip(source, target):
        os.makedirs(os.path.dirname(target.path), exist_ok=True)
        shutil.copy(src.path, target.path)


def generate(env, **kw):
    env.SetDefault(
        ASSETS_COMPILER="${FBT_SCRIPT_DIR}/assets.py",
    )

    if not env["VERBOSE"]:
        env.SetDefault(
            RESOURCEDISTCOMSTR="\tRESDIST\t${RESOURCES_ROOT}",
            RESMANIFESTCOMSTR="\tMANIFST\t${TARGET}",
        )

    env.Append(
        BUILDERS={
            "ResourcesDist": Builder(
                action=Action(
                    _resources_dist_action,
                    "${RESOURCEDISTCOMSTR}",
                ),
                emitter=_resources_dist_emitter,
            ),
            "ManifestBuilder": Builder(
                action=Action(
                    [
                        [
                            "${PYTHON3}",
                            "${ASSETS_COMPILER}",
                            "manifest",
                            "${TARGET.dir.posix}",
                            "--timestamp=${GIT_UNIX_TIMESTAMP}",
                        ]
                    ],
                    "${RESMANIFESTCOMSTR}",
                )
            ),
        }
    )


def exists(env):
    return True
