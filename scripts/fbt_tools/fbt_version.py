from SCons.Action import Action
from SCons.Builder import Builder


def _version_emitter(target, source, env):
    target_dir = target[0]
    target = [
        target_dir.File("version.inc.h"),
        target_dir.File("version.json"),
    ]
    return target, source


def generate(env):
    env.SetDefault(
        VERSION_SCRIPT="${FBT_SCRIPT_DIR}/version.py",
    )
    env.Append(
        BUILDERS={
            "VersionBuilder": Builder(
                action=Action(
                    [
                        [
                            "${PYTHON3}",
                            "${VERSION_SCRIPT}",
                            "generate",
                            "-t",
                            "${TARGET_HW}",
                            "--fw-origin",
                            "${FIRMWARE_ORIGIN}",
                            "-o",
                            "${TARGET.dir.posix}",
                            "--dir",
                            "${ROOT_DIR}",
                        ]
                    ],
                    "${VERSIONCOMSTR}",
                ),
                emitter=_version_emitter,
            ),
        }
    )


def exists(env):
    return True
