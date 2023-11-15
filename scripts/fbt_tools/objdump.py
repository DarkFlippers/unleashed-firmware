from SCons.Action import Action
from SCons.Builder import Builder


def generate(env):
    env.SetDefault(
        OBJDUMP="objdump",
        OBJDUMPFLAGS=[],
    )
    env.Append(
        BUILDERS={
            "ObjDump": Builder(
                action=Action(
                    [["$OBJDUMP", "$OBJDUMPFLAGS", "-S", "$SOURCES", ">", "$TARGET"]],
                    "${OBJDUMPCOMSTR}",
                ),
                suffix=".lst",
                src_suffix=".elf",
            ),
        }
    )


def exists(env):
    return True
