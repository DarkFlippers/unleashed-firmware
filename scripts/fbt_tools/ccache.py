def exists():
    return True


def generate(env):
    if env.WhereIs("ccache"):
        env["CCACHE"] = "ccache"
        env["CC_NOCACHE"] = env["CC"]
        env["CC"] = "$CCACHE $CC_NOCACHE"
        # Tricky place: linking is done with CXX
        # Using ccache breaks it
        env["LINK"] = env["CXX"]
        env["CXX_NOCACHE"] = env["CXX"]
        env["CXX"] = "$CCACHE $CXX_NOCACHE"
        env.AppendUnique(COMPILATIONDB_OMIT_BINARIES=["ccache"])
