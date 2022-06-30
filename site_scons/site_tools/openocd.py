from SCons.Builder import Builder
from SCons.Action import Action
from SCons.Defaults import Touch
import SCons

__OPENOCD_BIN = "openocd"

_oocd_action = Action(
    "${OPENOCD} ${OPENOCD_OPTS} ${OPENOCD_COMMAND}",
    "${OPENOCDCOMSTR}",
)


def generate(env):
    env.SetDefault(
        OPENOCD=__OPENOCD_BIN,
        OPENOCD_OPTS="",
        OPENOCD_COMMAND="",
        OPENOCDCOM="${OPENOCD} ${OPENOCD_OPTS} ${OPENOCD_COMMAND}",
        OPENOCDCOMSTR="",
    )

    env.Append(
        BUILDERS={
            "OpenOCDFlash": Builder(
                action=[
                    _oocd_action,
                    Touch("${TARGET}"),
                ],
                suffix=".flash",
                src_suffix=".bin",
            ),
        }
    )


def exists(env):
    try:
        return env["OPENOCD"]
    except KeyError:
        pass

    if openocd := env.WhereIs(__OPENOCD_BIN):
        return openocd

    raise SCons.Errors.StopError("Could not detect openocd")
