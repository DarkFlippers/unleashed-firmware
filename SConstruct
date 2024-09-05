#
# Main Flipper Build System entry point
#
# This file is evaluated by scons (the build system) every time fbt is invoked.
# Scons constructs all referenced environments & their targets' dependency
# trees on startup. So, to keep startup time as low as possible, we're hiding
# construction of certain targets behind command-line options.

import os
from fbt.util import open_browser_action

DefaultEnvironment(tools=[])

EnsurePythonVersion(3, 8)

# Progress(["OwO\r", "owo\r", "uwu\r", "owo\r"], interval=15)

# This environment is created only for loading options & validating file/dir existence
fbt_variables = SConscript("site_scons/commandline.scons")
cmd_environment = Environment(
    toolpath=["#/scripts/fbt_tools"],
    tools=[
        ("fbt_help", {"vars": fbt_variables}),
    ],
    variables=fbt_variables,
)

# Building basic environment - tools, utility methods, cross-compilation
# settings, gcc flags for Cortex-M4, basic builders and more
coreenv = SConscript(
    "site_scons/environ.scons",
    exports={"VAR_ENV": cmd_environment},
    toolpath=["#/scripts/fbt_tools"],
)
SConscript("site_scons/cc.scons", exports={"ENV": coreenv})

# Create a separate "dist" environment and add construction envs to it
distenv = coreenv.Clone(
    tools=[
        "fbt_dist",
        "fbt_debugopts",
        "openocd",
        "blackmagic",
        "jflash",
        "doxygen",
        "textfile",
    ],
    ENV=os.environ,
    UPDATE_BUNDLE_DIR="dist/${DIST_DIR}/f${TARGET_HW}-update-${DIST_SUFFIX}",
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
        "${ARGS}",
        "--bundlever",
        "${UPDATE_VERSION_STRING}",
    ]
    dist_radio_arguments = [
        "--radio",
        "${ROOT_DIR.abspath}/${COPRO_STACK_BIN_DIR}/${COPRO_STACK_BIN}",
        "--radiotype",
        "${COPRO_STACK_TYPE}",
        "${COPRO_DISCLAIMER}",
        "--obdata",
        "${ROOT_DIR.abspath}/${COPRO_OB_DATA}",
        "--stackversion",
        "${COPRO_CUBE_VERSION}",
    ]
    dist_resource_arguments = [
        "-r",
        firmware_env.subst("${RESOURCES_ROOT}"),
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
        (distenv["DIST_DEPENDS"], firmware_env["FW_RESOURCES_MANIFEST"]),
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
        "#build/usbinstall.flag",
        (firmware_env["FW_RESOURCES_MANIFEST"], selfupdate_dist),
    )
    distenv.Alias("flash_usb_full", usb_update_package)

    usb_minupdate_package = distenv.AddUsbFlashTarget(
        "#build/minusbinstall.flag", (selfupdate_min_dist,)
    )
    distenv.Alias("flash_usb", usb_minupdate_package)


# Target for copying & renaming binaries to dist folder
basic_dist = distenv.DistCommand("fw_dist", distenv["DIST_DEPENDS"])
distenv.Default(basic_dist)

dist_dir_name = distenv.GetProjetDirName()
dist_dir = distenv.Dir(f"#/dist/{dist_dir_name}")
external_apps_artifacts = firmware_env["FW_EXTAPPS"]
external_app_list = external_apps_artifacts.application_map.values()

fap_dist = [
    distenv.Install(
        dist_dir.Dir("debug_elf"),
        list(app_artifact.debug for app_artifact in external_app_list),
    ),
    *(
        distenv.Install(
            dist_dir.File(dist_entry[1]).dir,
            app_artifact.compact,
        )
        for app_artifact in external_app_list
        for dist_entry in app_artifact.dist_entries
    ),
]
Depends(
    fap_dist,
    list(app_artifact.validator for app_artifact in external_app_list),
)
Alias("fap_dist", fap_dist)

# Copy all faps to device

fap_deploy = distenv.PhonyTarget(
    "fap_deploy",
    Action(
        [
            [
                "${PYTHON3}",
                "${FBT_SCRIPT_DIR}/storage.py",
                "-p",
                "${FLIP_PORT}",
                "send",
                "${SOURCE}",
                "/ext/apps",
                "${ARGS}",
            ]
        ]
    ),
    source=firmware_env.Dir(("${RESOURCES_ROOT}/apps")),
)
Depends(fap_deploy, firmware_env["FW_RESOURCES_MANIFEST"])


# Target for bundling core2 package for qFlipper
copro_dist = distenv.CoproBuilder(
    "#/build/core2_firmware.tgz",
    [],
)
distenv.AlwaysBuild(copro_dist)
distenv.Alias("copro_dist", copro_dist)


firmware_flash = distenv.AddFwFlashTarget(firmware_env)
distenv.Alias("flash", firmware_flash)

# To be implemented in fwflash.py
firmware_jflash = distenv.AddJFlashTarget(firmware_env)
distenv.Alias("jflash", firmware_jflash)

distenv.PhonyTarget(
    "gdb_trace_all",
    [["${GDB}", "${GDBOPTS}", "${SOURCES}", "${GDBFLASH}"]],
    source=firmware_env["FW_ELF"],
    GDBOPTS="${GDBOPTS_BASE}",
    GDBREMOTE="${OPENOCD_GDB_PIPE}",
    GDBFLASH=[
        "-ex",
        "thread apply all bt",
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
    FBT_FAP_DEBUG_ELF_ROOT=firmware_env["FBT_FAP_DEBUG_ELF_ROOT"],
)
distenv.Depends(firmware_debug, firmware_flash)

firmware_blackmagic = distenv.PhonyTarget(
    "blackmagic",
    "${GDBPYCOM}",
    source=firmware_env["FW_ELF"],
    GDBOPTS="${GDBOPTS_BASE} ${GDBOPTS_BLACKMAGIC}",
    GDBREMOTE="${BLACKMAGIC_ADDR}",
    FBT_FAP_DEBUG_ELF_ROOT=firmware_env["FBT_FAP_DEBUG_ELF_ROOT"],
)
distenv.Depends(firmware_blackmagic, firmware_flash)

# Debug alien elf
debug_other_opts = [
    "-ex",
    "source ${FBT_DEBUG_DIR}/PyCortexMDebug/PyCortexMDebug.py",
    # "-ex",
    # "source ${FBT_DEBUG_DIR}/FreeRTOS/FreeRTOS.py",
    "-ex",
    "source ${FBT_DEBUG_DIR}/flipperversion.py",
    "-ex",
    "fw-version",
]

distenv.PhonyTarget(
    "debug_other",
    "${GDBPYCOM}",
    GDBOPTS="${GDBOPTS_BASE}",
    GDBREMOTE="${OPENOCD_GDB_PIPE}",
    GDBPYOPTS=debug_other_opts,
)

distenv.PhonyTarget(
    "debug_other_blackmagic",
    "${GDBPYCOM}",
    GDBOPTS="${GDBOPTS_BASE} ${GDBOPTS_BLACKMAGIC}",
    GDBREMOTE="${BLACKMAGIC_ADDR}",
    GDBPYOPTS=debug_other_opts,
)


# Just start OpenOCD
distenv.PhonyTarget(
    "openocd",
    [["${OPENOCDCOM}", "${ARGS}"]],
)

# Linter
distenv.PhonyTarget(
    "lint",
    [
        [
            "${PYTHON3}",
            "${FBT_SCRIPT_DIR}/lint.py",
            "check",
            "${LINT_SOURCES}",
            "${ARGS}",
        ]
    ],
    LINT_SOURCES=[n.srcnode() for n in firmware_env["LINT_SOURCES"]],
)

distenv.PhonyTarget(
    "format",
    [
        [
            "${PYTHON3}",
            "${FBT_SCRIPT_DIR}/lint.py",
            "format",
            "${LINT_SOURCES}",
            "${ARGS}",
        ]
    ],
    LINT_SOURCES=[n.srcnode() for n in firmware_env["LINT_SOURCES"]],
)

# PY_LINT_SOURCES contains recursively-built modules' SConscript files
# Here we add additional Python files residing in repo root
firmware_env.Append(
    PY_LINT_SOURCES=[
        # Py code folders
        "site_scons",
        "scripts",
        "applications",
        "applications_user",
        "assets",
        "targets",
        # Extra files
        "SConstruct",
        "firmware.scons",
        "fbt_options.py",
    ],
    IMG_LINT_SOURCES=[
        # Image assets
        "applications",
        "assets",
    ],
)


black_commandline = [
    [
        "@${PYTHON3}",
        "-m",
        "black",
        "${PY_BLACK_ARGS}",
        "${PY_LINT_SOURCES}",
        "${ARGS}",
    ]
]
black_base_args = [
    "--include",
    '"(\\.scons|\\.py|SConscript|SConstruct|\\.fam)$"',
]

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

# Image assets linting
distenv.PhonyTarget(
    "lint_img",
    [
        [
            "${PYTHON3}",
            "${FBT_SCRIPT_DIR}/imglint.py",
            "check",
            "${IMG_LINT_SOURCES}",
            "${ARGS}",
        ]
    ],
    IMG_LINT_SOURCES=firmware_env["IMG_LINT_SOURCES"],
)

distenv.PhonyTarget(
    "format_img",
    [
        [
            "${PYTHON3}",
            "${FBT_SCRIPT_DIR}/imglint.py",
            "format",
            "${IMG_LINT_SOURCES}",
            "${ARGS}",
        ]
    ],
    IMG_LINT_SOURCES=firmware_env["IMG_LINT_SOURCES"],
)

distenv.Alias("lint_all", ["lint", "lint_py", "lint_img"])
distenv.Alias("format_all", ["format", "format_py", "format_img"])


# Start Flipper CLI via PySerial's miniterm
distenv.PhonyTarget(
    "cli",
    [
        [
            "${PYTHON3}",
            "${FBT_SCRIPT_DIR}/serial_cli.py",
            "-p",
            "${FLIP_PORT}",
            "${ARGS}",
        ]
    ],
)

# Update WiFi devboard firmware with release channel
distenv.PhonyTarget(
    "devboard_flash",
    [
        [
            "${PYTHON3}",
            "${FBT_SCRIPT_DIR}/wifi_board.py",
            "${ARGS}",
        ]
    ],
)


# Find blackmagic probe
distenv.PhonyTarget(
    "get_blackmagic",
    "@echo $( ${BLACKMAGIC_ADDR} $)",
)


# Find STLink probe ids
distenv.PhonyTarget(
    "get_stlink",
    distenv.Action(
        lambda **_: distenv.GetDevices(),
        None,
    ),
)

# Prepare vscode environment
vscode_dist = distenv.Install(
    "#.vscode",
    [
        distenv.Glob("#.vscode/example/*.json", exclude="*.tmpl"),
        distenv.Glob("#.vscode/example/${LANG_SERVER}/*.json"),
    ],
)
for template_file in distenv.Glob("#.vscode/example/*.tmpl"):
    vscode_dist.append(
        distenv.Substfile(
            distenv.Dir("#.vscode").File(template_file.name.replace(".tmpl", "")),
            template_file,
            SUBST_DICT={
                "@FBT_PLATFORM_EXECUTABLE_EXT@": ".exe" if os.name == "nt" else ""
            },
        )
    )
distenv.Precious(vscode_dist)
distenv.NoClean(vscode_dist)
distenv.Alias("vscode_dist", (vscode_dist, firmware_env["FW_CDB"]))

# Configure shell with build tools
distenv.PhonyTarget(
    "env",
    "@echo $( ${FBT_SCRIPT_DIR.abspath}/toolchain/fbtenv.sh $)",
)

doxy_build = distenv.DoxyBuild(
    "documentation/doxygen/build/html/index.html",
    "documentation/doxygen/Doxyfile-awesome.cfg",
    doxy_env_variables={
        "DOXY_SRC_ROOT": Dir(".").abspath,
        "DOXY_BUILD_DIR": Dir("documentation/doxygen/build").abspath,
        "DOXY_CONFIG_DIR": "documentation/doxygen",
    },
)
distenv.Alias("doxygen", doxy_build)
distenv.AlwaysBuild(doxy_build)

# Open generated documentation in browser
distenv.PhonyTarget("doxy", open_browser_action, source=doxy_build)
