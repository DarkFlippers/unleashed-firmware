from SCons.Builder import Builder
from SCons.Action import Action


def generate(env):
    env.SetDefault(
        GDB="gdb",
        GDBPY="gdb-py",
        GDBOPTS="",
        GDBPYOPTS="",
        GDBCOM="$GDB $GDBOPTS $SOURCES",  # no $TARGET
        GDBPYCOM="$GDBPY $GDBOPTS $GDBPYOPTS $SOURCES",  # no $TARGET
    )


def exists(env):
    return True
