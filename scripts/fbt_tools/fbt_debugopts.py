from re import search

from SCons.Errors import UserError
from fbt_options import OPENOCD_OPTS


def _get_device_serials(search_str="STLink"):
    import serial.tools.list_ports as list_ports

    return set([device.serial_number for device in list_ports.grep(search_str)])


def GetDevices(env):
    serials = _get_device_serials()
    if len(serials) == 0:
        raise UserError("No devices found")

    print("\n".join(serials))


def generate(env, **kw):
    env.AddMethod(GetDevices)

    if (adapter_serial := env.subst("$OPENOCD_ADAPTER_SERIAL")) != "auto":
        env.Append(
            OPENOCD_OPTS=[
                "-c",
                f"adapter serial {adapter_serial}",
            ]
        )

    # Final command is "init", always explicitly added
    env.Append(
        OPENOCD_OPTS=["-c", "init"],
    )

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
