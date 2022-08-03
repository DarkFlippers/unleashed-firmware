Import("env")

env.Append(LINT_SOURCES=["furi"])


libenv = env.Clone(FW_LIB_NAME="furi")
libenv.ApplyLibFlags()

sources = libenv.GlobRecursive("*.c")

lib = libenv.StaticLibrary("${FW_LIB_NAME}", sources)
libenv.Install("${LIB_DIST_DIR}", lib)
Return("lib")
