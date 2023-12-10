from SCons.Action import Action
from SCons.Builder import Builder


def generate(env):
    env.SetDefault(
        STRIP="strip",
        STRIPFLAGS=[],
    )
    env.Append(
        BUILDERS={
            "ELFStripper": Builder(
                action=Action(
                    [["$STRIP", "$STRIPFLAGS", "$SOURCES", "-o", "$TARGET"]],
                    "${STRIPCOMSTR}",
                ),
                suffix=".elf",
                src_suffix=".elf",
            ),
        }
    )


def exists(env):
    return True
