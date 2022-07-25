from SCons.Builder import Builder
from SCons.Action import Action


def version_emitter(target, source, env):
    target_dir = target[0]
    target = [
        target_dir.File("version.inc.h"),
        target_dir.File("version.json"),
    ]
    return target, source


def generate(env):
    env.Append(
        BUILDERS={
            "VersionBuilder": Builder(
                action=Action(
                    '${PYTHON3} "${ROOT_DIR.abspath}/scripts/version.py" generate -t ${TARGET_HW} -o ${TARGET.dir.posix} --dir "${ROOT_DIR}"',
                    "${VERSIONCOMSTR}",
                ),
                emitter=version_emitter,
            ),
        }
    )


def exists(env):
    return True
