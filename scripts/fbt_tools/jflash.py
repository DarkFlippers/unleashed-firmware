from SCons.Builder import Builder
from SCons.Defaults import Touch


def generate(env):
    env.SetDefault(
        JFLASH="JFlash" if env.subst("$PLATFORM") == "win32" else "JFlashExe",
        JFLASHFLAGS=[
            "-auto",
            "-exit",
        ],
        JFLASHCOM="${JFLASH} -openprj${JFLASHPROJECT} -open${SOURCE},${JFLASHADDR} ${JFLASHFLAGS}",
    )
    env.Append(
        BUILDERS={
            "JFlash": Builder(
                action=[
                    "${JFLASHCOM}",
                    Touch("${TARGET}"),
                ],
            ),
        }
    )


def exists(env):
    return True
