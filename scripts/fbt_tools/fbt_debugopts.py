def generate(env, **kw):
    env.SetDefault(
        OPENOCD_GDB_PIPE=[
            "|openocd -c 'gdb_port pipe; log_output debug/openocd.log' ${[SINGLEQUOTEFUNC(OPENOCD_OPTS)]}"
        ],
        GDBOPTS_BASE=[
            "-ex",
            "target extended-remote ${GDBREMOTE}",
            "-ex",
            "set confirm off",
            "-ex",
            "set pagination off",
        ],
        GDBOPTS_BLACKMAGIC=[
            "-ex",
            "monitor swdp_scan",
            "-ex",
            "monitor debug_bmp enable",
            "-ex",
            "attach 1",
            "-ex",
            "set mem inaccessible-by-default off",
        ],
        GDBPYOPTS=[
            "-ex",
            "source debug/FreeRTOS/FreeRTOS.py",
            "-ex",
            "source debug/flipperapps.py",
            "-ex",
            "source debug/PyCortexMDebug/PyCortexMDebug.py",
            "-ex",
            "svd_load ${SVD_FILE}",
            "-ex",
            "compare-sections",
        ],
        JFLASHPROJECT="${ROOT_DIR.abspath}/debug/fw.jflash",
    )


def exists(env):
    return True
