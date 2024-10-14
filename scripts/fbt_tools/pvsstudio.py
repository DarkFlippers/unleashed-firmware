import atexit
import multiprocessing
import subprocess
import sys
import webbrowser

from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Script import Delete, Flatten, GetBuildFailures, Mkdir

__no_browser = False


def _set_browser_action(target, source, env):
    if env["PVSNOBROWSER"]:
        global __no_browser
        __no_browser = True


def _emit_pvsreport(target, source, env):
    target_dir = env["REPORT_DIR"]
    if env["PLATFORM"] == "win32":
        # Report generator on Windows emits to a subfolder of given output folder
        target_dir = target_dir.Dir("fullhtml")
    return [target_dir.File("index.html")], source


def atexist_handler():
    global __no_browser
    if __no_browser:
        return

    for bf in GetBuildFailures():
        for node in Flatten(bf.node):
            if node.exists and "pvs" in node.path and node.name.endswith(".html"):
                # macOS
                if sys.platform == "darwin":
                    subprocess.run(["open", node.abspath])
                else:
                    webbrowser.open(node.abspath)
                break


def generate(env):
    env.SetDefault(
        PVSNCORES=multiprocessing.cpu_count(),
        PVSOPTIONS=[
            "@.pvsoptions",
            "-j${PVSNCORES}",
            # "--disableLicenseExpirationCheck",
            # "--incremental", # kinda broken on PVS side
        ],
        PVSCONVOPTIONS=[
            "-a",
            "GA:1,2,3",
            "-t",
            "fullhtml",
            "--indicate-warnings",
        ],
    )

    if env["PLATFORM"] == "win32":
        env.SetDefault(
            PVSCHECKBIN="CompilerCommandsAnalyzer.exe",
            PVSCONVBIN="PlogConverter.exe",
        )
    else:
        env.SetDefault(
            PVSCHECKBIN="pvs-studio-analyzer",
            PVSCONVBIN="plog-converter",
        )

    if not env["VERBOSE"]:
        env.SetDefault(
            PVSCHECKCOMSTR="\tPVS\t${TARGET}",
            PVSCONVCOMSTR="\tPVSREP\t${TARGET}",
        )

    env.Append(
        BUILDERS={
            "PVSCheck": Builder(
                action=Action(
                    [
                        [
                            "${PVSCHECKBIN}",
                            "analyze",
                            "${PVSOPTIONS}",
                            "-f",
                            "${SOURCE}",
                            "-o",
                            "${TARGET}",
                        ]
                    ],
                    "${PVSCHECKCOMSTR}",
                ),
                suffix=".log",
                src_suffix=".json",
            ),
            "PVSReport": Builder(
                action=Action(
                    [
                        Delete("${TARGET.dir}"),
                        # PlogConverter.exe and plog-converter have different behavior
                        Mkdir("${TARGET.dir}") if env["PLATFORM"] == "win32" else None,
                        Action(_set_browser_action, None),
                        Action(
                            [
                                [
                                    "${PVSCONVBIN}",
                                    "${PVSCONVOPTIONS}",
                                    "${SOURCE}",
                                    "-o",
                                    "${REPORT_DIR}",
                                ]
                            ]
                        ),
                    ],
                    "${PVSCONVCOMSTR}",
                ),
                emitter=_emit_pvsreport,
                src_suffix=".log",
            ),
        }
    )
    atexit.register(atexist_handler)


def exists(env):
    return True
