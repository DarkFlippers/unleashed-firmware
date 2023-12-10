from SCons.Builder import Builder
from SCons.Defaults import Touch
from SCons.Action import Action


def generate(env):
    env.SetDefault(
        JFLASH="JFlash" if env.subst("$PLATFORM") == "win32" else "JFlashExe",
        JFLASHFLAGS=[
            "-auto",
            "-exit",
        ],
    )
    env.Append(
        BUILDERS={
            "JFlash": Builder(
                action=[
                    Action(
                        [
                            [
                                "${JFLASH}",
                                "-openprj${JFLASHPROJECT}",
                                "-open${SOURCE},${JFLASHADDR}",
                                "${JFLASHFLAGS}",
                            ]
                        ]
                    ),
                    Touch("${TARGET}"),
                ],
            ),
        }
    )


def exists(env):
    return True
