from SCons.Builder import Builder
from SCons.Action import Action
from SCons.Script import Mkdir
from SCons.Defaults import Touch


def GetProjetDirName(env, project=None):
    parts = [f"f{env['TARGET_HW']}"]
    if project:
        parts.append(project)

    suffix = ""
    if env["DEBUG"]:
        suffix += "D"
    if env["COMPACT"]:
        suffix += "C"
    if suffix:
        parts.append(suffix)

    return "-".join(parts)


def create_fw_build_targets(env, configuration_name):
    flavor = GetProjetDirName(env, configuration_name)
    build_dir = env.Dir("build").Dir(flavor).abspath
    return env.SConscript(
        "firmware.scons",
        variant_dir=build_dir,
        duplicate=0,
        exports={
            "ENV": env,
            "fw_build_meta": {
                "type": configuration_name,
                "flavor": flavor,
                "build_dir": build_dir,
            },
        },
    )


def AddFwProject(env, base_env, fw_type, fw_env_key):
    project_env = env[fw_env_key] = create_fw_build_targets(base_env, fw_type)
    env.Append(
        DIST_PROJECTS=[
            project_env["FW_FLAVOR"],
        ],
        DIST_DEPENDS=[
            project_env["FW_ARTIFACTS"],
        ],
    )

    env.Replace(DIST_DIR=env.GetProjetDirName())
    return project_env


def AddOpenOCDFlashTarget(env, targetenv, **kw):
    openocd_target = env.OpenOCDFlash(
        "#build/oocd-${BUILD_CFG}-flash.flag",
        targetenv["FW_BIN"],
        OPENOCD_COMMAND=[
            "-c",
            "program ${SOURCE.posix} reset exit ${BASE_ADDRESS}",
        ],
        BUILD_CFG=targetenv.subst("$FIRMWARE_BUILD_CFG"),
        BASE_ADDRESS=targetenv.subst("$IMAGE_BASE_ADDRESS"),
        **kw,
    )
    env.Alias(targetenv.subst("${FIRMWARE_BUILD_CFG}_flash"), openocd_target)
    if env["FORCE"]:
        env.AlwaysBuild(openocd_target)
    return openocd_target


def AddJFlashTarget(env, targetenv, **kw):
    jflash_target = env.JFlash(
        "#build/jflash-${BUILD_CFG}-flash.flag",
        targetenv["FW_BIN"],
        JFLASHADDR=targetenv.subst("$IMAGE_BASE_ADDRESS"),
        BUILD_CFG=targetenv.subst("${FIRMWARE_BUILD_CFG}"),
        **kw,
    )
    env.Alias(targetenv.subst("${FIRMWARE_BUILD_CFG}_jflash"), jflash_target)
    if env["FORCE"]:
        env.AlwaysBuild(jflash_target)
    return jflash_target


def AddUsbFlashTarget(env, file_flag, extra_deps, **kw):
    usb_update = env.UsbInstall(
        file_flag,
        (
            env["DIST_DEPENDS"],
            *extra_deps,
        ),
    )
    if env["FORCE"]:
        env.AlwaysBuild(usb_update)
    return usb_update


def DistCommand(env, name, source, **kw):
    target = f"dist_{name}"
    command = env.Command(
        target,
        source,
        '@${PYTHON3} "${ROOT_DIR.abspath}/scripts/sconsdist.py" copy -p ${DIST_PROJECTS} -s "${DIST_SUFFIX}" ${DIST_EXTRA}',
        **kw,
    )
    env.Pseudo(target)
    env.Alias(name, command)
    return command


def generate(env):
    env.AddMethod(AddFwProject)
    env.AddMethod(DistCommand)
    env.AddMethod(AddOpenOCDFlashTarget)
    env.AddMethod(GetProjetDirName)
    env.AddMethod(AddJFlashTarget)
    env.AddMethod(AddUsbFlashTarget)

    env.SetDefault(
        COPRO_MCU_FAMILY="STM32WB5x",
    )

    env.Append(
        BUILDERS={
            "UsbInstall": Builder(
                action=[
                    Action(
                        '${PYTHON3} "${ROOT_DIR.abspath}/scripts/selfupdate.py" dist/${DIST_DIR}/f${TARGET_HW}-update-${DIST_SUFFIX}/update.fuf'
                    ),
                    Touch("${TARGET}"),
                ]
            ),
            "CoproBuilder": Builder(
                action=Action(
                    [
                        Mkdir("$TARGET"),
                        '${PYTHON3} "${ROOT_DIR.abspath}/scripts/assets.py" '
                        "copro ${COPRO_CUBE_DIR} "
                        "${TARGET} ${COPRO_MCU_FAMILY} "
                        "--cube_ver=${COPRO_CUBE_VERSION} "
                        "--stack_type=${COPRO_STACK_TYPE} "
                        '--stack_file="${COPRO_STACK_BIN}" '
                        "--stack_addr=${COPRO_STACK_ADDR} ",
                    ],
                    "",
                )
            ),
        }
    )


def exists(env):
    return True
