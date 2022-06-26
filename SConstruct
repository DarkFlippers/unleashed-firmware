#
# Main Fipper Build System entry point
#
# This file is evaluated by scons (the build system) every time fbt is invoked.
# Scons constructs all referenced environments & their targets' dependency
# trees on startup. So, to keep startup time as low as possible, we're hiding
# construction of certain targets behind command-line options.

import os

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
    tools=["fbt_dist", "openocd"],
    GDBOPTS="-ex 'target extended-remote | ${OPENOCD} -c \"gdb_port pipe\" ${OPENOCD_OPTS}' "
    '-ex "set confirm off" ',
    ENV=os.environ,
)

firmware_out = distenv.AddFwProject(
    base_env=coreenv,
    fw_type="firmware",
    fw_env_key="FW_ENV",
)


# If enabled, initialize updater-related targets
if GetOption("fullenv"):
    updater_out = distenv.AddFwProject(
        base_env=coreenv,
        fw_type="updater",
        fw_env_key="UPD_ENV",
    )

    # Target for self-update package
    dist_arguments = [
        "-r",
        '"${ROOT_DIR.abspath}/assets/resources"',
        "--bundlever",
        '"${UPDATE_VERSION_STRING}"',
        "--radio",
        '"${ROOT_DIR.abspath}/${COPRO_STACK_BIN_DIR}/${COPRO_STACK_BIN}"',
        "--radiotype",
        "${COPRO_STACK_TYPE}",
        "${COPRO_DISCLAIMER}",
        "--obdata",
        '"${ROOT_DIR.abspath}/${COPRO_OB_DATA}"',
    ]
    if distenv["UPDATE_SPLASH"]:
        dist_arguments += [
            "--splash",
            distenv.subst("assets/slideshow/$UPDATE_SPLASH"),
        ]
    selfupdate_dist = distenv.DistBuilder(
        "selfupdate.pseudo",
        (distenv["DIST_DEPENDS"], firmware_out["FW_RESOURCES"]),
        DIST_EXTRA=dist_arguments,
    )
    distenv.Pseudo("selfupdate.pseudo")
    AlwaysBuild(selfupdate_dist)
    Alias("updater_package", selfupdate_dist)

    # Updater debug
    debug_updater_elf = distenv.AddDebugTarget(updater_out, False)
    Alias("updater_debug", debug_updater_elf)


# Target for copying & renaming binaries to dist folder
basic_dist = distenv.DistBuilder("dist.pseudo", distenv["DIST_DEPENDS"])
distenv.Pseudo("dist.pseudo")
AlwaysBuild(basic_dist)
Alias("fw_dist", basic_dist)
Default(basic_dist)

# Target for bundling core2 package for qFlipper
copro_dist = distenv.CoproBuilder(
    Dir("assets/core2_firmware"),
    [],
)
AlwaysBuild(copro_dist)
Alias("copro_dist", copro_dist)


# Debugging firmware

debug_fw_elf = distenv.AddDebugTarget(firmware_out)
Alias("debug", debug_fw_elf)


# Debug alien elf
debug_other = distenv.GDBPy(
    "debugother.pseudo",
    None,
    GDBPYOPTS=
    # '-ex "source ${ROOT_DIR.abspath}/debug/FreeRTOS/FreeRTOS.py" '
    '-ex "source debug/PyCortexMDebug/PyCortexMDebug.py" '
)
distenv.Pseudo("debugother.pseudo")
AlwaysBuild(debug_other)
Alias("debug_other", debug_other)


# Just start OpenOCD
openocd = distenv.OOCDCommand("openocd.pseudo", [])
distenv.Pseudo("openocd.pseudo")
AlwaysBuild(openocd)
Alias("openocd", openocd)


# Linter
lint_check = distenv.Command(
    "lint.check.pseudo",
    [],
    "${PYTHON3} scripts/lint.py check $LINT_SOURCES",
    LINT_SOURCES=firmware_out["LINT_SOURCES"],
)
distenv.Pseudo("lint.check.pseudo")
AlwaysBuild(lint_check)
Alias("lint", lint_check)


lint_format = distenv.Command(
    "lint.format.pseudo",
    [],
    "${PYTHON3} scripts/lint.py format $LINT_SOURCES",
    LINT_SOURCES=firmware_out["LINT_SOURCES"],
)
distenv.Pseudo("lint.format.pseudo")
AlwaysBuild(lint_format)
Alias("format", lint_format)
