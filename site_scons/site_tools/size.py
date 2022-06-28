from SCons.Builder import Builder
from SCons.Action import Action


def generate(env):
    env.SetDefault(
        SIZE="size",
        SIZEFLAGS=[],
        SIZECOM="$SIZE $SIZEFLAGS $TARGETS",
    )
    env.Append(
        BUILDERS={
            "ELFSize": Builder(
                action=Action(
                    "${SIZECOM}",
                    "${SIZECOMSTR}",
                ),
            ),
        }
    )


def exists(env):
    return True
