def generate(env):
    env.SetDefault(
        GDB="gdb",
        GDBPY="gdb-py",
        GDBCOM="$GDB $GDBOPTS $SOURCES",  # no $TARGET
        GDBPYCOM="$GDBPY $GDBOPTS $GDBPYOPTS $SOURCES",  # no $TARGET
    )


def exists(env):
    return True
