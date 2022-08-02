#
# Main Fipper Build System entry point
#
# This file is evaluated by scons (the build system) every time fbt is invoked.
# Scons constructs all referenced environments & their targets' dependency
# trees on startup. So, to keep startup time as low as possible, we're hiding
# construction of certain targets behind command-line options.

import os
import subprocess

EnsurePythonVersion(3, 8)

DefaultEnvironment(tools=[])
# Progress(["OwO\r", "owo\r", "uwu\r", "owo\r"], interval=15)


# This environment is created only for loading options & validating file/dir existance
fbt_variables = SConscript("site_scons/commandline.scons")
cmd_environment = Environment(tools=[], variables=fbt_variables)
Help(fbt_variables.GenerateHelpText(cmd_environment))

# Building basic environment - tools, utility methods, cross-compilation
# settings, gcc flags for Cortex-M4, basic builders and more
coreenv = SConscript(
    "site_scons/environ.scons",
    exports={"VAR_ENV": cmd_environment},
)
SConscript("site_scons/cc.scons", exports={"ENV": coreenv})

# Store root dir in environment for certain tools
coreenv["ROOT_DIR"] = Dir(".")


# Create a separate "dist" environment and add construction envs to it
distenv = coreenv.Clone(
    tools=["fbt_dist", "openocd", "blackmagic", "jflash"],
    OPENOCD_GDB_PIPE=[
        "|openocd -c 'gdb_port pipe; log_output debug/openocd.log' ${[SINGLEQUOTEFUNC(OPENOCD_OPTS)]}"
    ],
    GDBOPTS_BASE=[
        "-ex",
        "target extended-remote ${GDBREMOTE}",
        "-ex",
        "set confirm off",
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
        "source debug/PyCortexMDebug/PyCortexMDebug.py",
        "-ex",
        "svd_load ${SVD_FILE}",
        "-ex",
        "compare-sections",
    ],
    JFLASHPROJECT="${ROOT_DIR.abspath}/debug/fw.jflash",
    ENV=os.environ,
)

firmware_env = distenv.AddFwProject(
    base_env=coreenv,
    fw_type="firmware",
    fw_env_key="FW_ENV",
)

# If enabled, initialize updater-related targets
if GetOption("fullenv") or any(
    filter(lambda target: "updater" in target or "flash_usb" in target, BUILD_TARGETS)
):
    updater_env = distenv.AddFwProject(
        base_env=coreenv,
        fw_type="updater",
        fw_env_key="UPD_ENV",
    )

    # Target for self-update package
    dist_basic_arguments = [
        "--bundlever",
        '"${UPDATE_VERSION_STRING}"',
    ]
    dist_radio_arguments = [
        "--radio",
        '"${ROOT_DIR.abspath}/${COPRO_STACK_BIN_DIR}/${COPRO_STACK_BIN}"',
        "--radiotype",
        "${COPRO_STACK_TYPE}",
        "${COPRO_DISCLAIMER}",
        "--obdata",
        '"${ROOT_DIR.abspath}/${COPRO_OB_DATA}"',
    ]
    dist_resource_arguments = [
        "-r",
        '"${ROOT_DIR.abspath}/assets/resources"',
    ]
    dist_splash_arguments = (
        [
            "--splash",
            distenv.subst("assets/slideshow/$UPDATE_SPLASH"),
        ]
        if distenv["UPDATE_SPLASH"]
        else []
    )

    selfupdate_dist = distenv.DistCommand(
        "updater_package",
        (distenv["DIST_DEPENDS"], firmware_env["FW_RESOURCES"]),
        DIST_EXTRA=[
            *dist_basic_arguments,
            *dist_radio_arguments,
            *dist_resource_arguments,
            *dist_splash_arguments,
        ],
    )

    selfupdate_min_dist = distenv.DistCommand(
        "updater_minpackage",
        distenv["DIST_DEPENDS"],
        DIST_EXTRA=dist_basic_arguments,
    )

    # Updater debug
    distenv.PhonyTarget(
        "updater_debug",
        "${GDBPYCOM}",
        source=updater_env["FW_ELF"],
        GDBREMOTE="${OPENOCD_GDB_PIPE}",
    )

    distenv.PhonyTarget(
        "updater_blackmagic",
        "${GDBPYCOM}",
        source=updater_env["FW_ELF"],
        GDBOPTS=distenv.subst("$GDBOPTS_BLACKMAGIC"),
        GDBREMOTE="${BLACKMAGIC_ADDR}",
    )

    # Installation over USB & CLI
    usb_update_package = distenv.AddUsbFlashTarget(
        "#build/usbinstall.flag", (firmware_env["FW_RESOURCES"], selfupdate_dist)
    )
    distenv.Alias("flash_usb_full", usb_update_package)

    usb_minupdate_package = distenv.AddUsbFlashTarget(
        "#build/minusbinstall.flag", (selfupdate_min_dist,)
    )
    distenv.Alias("flash_usb", usb_minupdate_package)


# Target for copying & renaming binaries to dist folder
basic_dist = distenv.DistCommand("fw_dist", distenv["DIST_DEPENDS"])
distenv.Default(basic_dist)

# Target for bundling core2 package for qFlipper
copro_dist = distenv.CoproBuilder(
    distenv.Dir("assets/core2_firmware"),
    [],
)
distenv.Alias("copro_dist", copro_dist)

firmware_flash = distenv.AddOpenOCDFlashTarget(firmware_env)
distenv.Alias("flash", firmware_flash)

firmware_jflash = distenv.AddJFlashTarget(firmware_env)
distenv.Alias("jflash", firmware_jflash)

firmware_bm_flash = distenv.PhonyTarget(
    "flash_blackmagic",
    "$GDB $GDBOPTS $SOURCES $GDBFLASH",
    source=firmware_env["FW_ELF"],
    GDBOPTS="${GDBOPTS_BASE} ${GDBOPTS_BLACKMAGIC}",
    GDBREMOTE="${BLACKMAGIC_ADDR}",
    GDBFLASH=[
        "-ex",
        "load",
        "-ex",
        "quit",
    ],
)

# Debugging firmware
firmware_debug = distenv.PhonyTarget(
    "debug",
    "${GDBPYCOM}",
    source=firmware_env["FW_ELF"],
    GDBOPTS="${GDBOPTS_BASE}",
    GDBREMOTE="${OPENOCD_GDB_PIPE}",
)
distenv.Depends(firmware_debug, firmware_flash)

distenv.PhonyTarget(
    "blackmagic",
    "${GDBPYCOM}",
    source=firmware_env["FW_ELF"],
    GDBOPTS="${GDBOPTS_BASE} ${GDBOPTS_BLACKMAGIC}",
    GDBREMOTE="${BLACKMAGIC_ADDR}",
)

# Debug alien elf
distenv.PhonyTarget(
    "debug_other",
    "${GDBPYCOM}",
    GDBPYOPTS='-ex "source debug/PyCortexMDebug/PyCortexMDebug.py" ',
    GDBREMOTE="${OPENOCD_GDB_PIPE}",
)

# Just start OpenOCD
distenv.PhonyTarget(
    "openocd",
    "${OPENOCDCOM}",
)

# Linter
distenv.PhonyTarget(
    "lint",
    "${PYTHON3} scripts/lint.py check ${LINT_SOURCES}",
    LINT_SOURCES=firmware_env["LINT_SOURCES"],
)

distenv.PhonyTarget(
    "format",
    "${PYTHON3} scripts/lint.py format ${LINT_SOURCES}",
    LINT_SOURCES=firmware_env["LINT_SOURCES"],
)

# PY_LINT_SOURCES contains recursively-built modules' SConscript files + application manifests
# Here we add additional Python files residing in repo root
firmware_env.Append(
    PY_LINT_SOURCES=[
        # Py code folders
        "site_scons",
        "scripts",
        # Extra files
        "applications/extapps.scons",
        "SConstruct",
        "firmware.scons",
        "fbt_options.py",
    ]
)


black_commandline = "@${PYTHON3} -m black ${PY_BLACK_ARGS} ${PY_LINT_SOURCES}"
black_base_args = ["--include", '"\\.scons|\\.py|SConscript|SConstruct"']

distenv.PhonyTarget(
    "lint_py",
    black_commandline,
    PY_BLACK_ARGS=[
        "--check",
        "--diff",
        *black_base_args,
    ],
    PY_LINT_SOURCES=firmware_env["PY_LINT_SOURCES"],
)

distenv.PhonyTarget(
    "format_py",
    black_commandline,
    PY_BLACK_ARGS=black_base_args,
    PY_LINT_SOURCES=firmware_env["PY_LINT_SOURCES"],
)

# Start Flipper CLI via PySerial's miniterm
distenv.PhonyTarget("cli", "${PYTHON3} scripts/serial_cli.py")


# Find blackmagic probe

distenv.PhonyTarget(
    "get_blackmagic",
    "@echo $( ${BLACKMAGIC_ADDR} $)",
)
