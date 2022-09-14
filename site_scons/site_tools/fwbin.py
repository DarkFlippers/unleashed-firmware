from SCons.Builder import Builder
from SCons.Action import Action
import SCons

__OBJCOPY_ARM_BIN = "arm-none-eabi-objcopy"
__NM_ARM_BIN = "arm-none-eabi-nm"


def generate(env):
    env.SetDefault(
        BIN2DFU="${ROOT_DIR.abspath}/scripts/bin2dfu.py",
        OBJCOPY=__OBJCOPY_ARM_BIN,  # FIXME
        NM=__NM_ARM_BIN,  # FIXME
    )
    env.Append(
        BUILDERS={
            "HEXBuilder": Builder(
                action=Action(
                    '${OBJCOPY} -O ihex "${SOURCE}" "${TARGET}"',
                    "${HEXCOMSTR}",
                ),
                suffix=".hex",
                src_suffix=".elf",
            ),
            "BINBuilder": Builder(
                action=Action(
                    '${OBJCOPY} -O binary -S "${SOURCE}" "${TARGET}"',
                    "${BINCOMSTR}",
                ),
                suffix=".bin",
                src_suffix=".elf",
            ),
            "DFUBuilder": Builder(
                action=Action(
                    '${PYTHON3} "${BIN2DFU}" -i "${SOURCE}" -o "${TARGET}" -a ${IMAGE_BASE_ADDRESS} -l "Flipper Zero F${TARGET_HW}"',
                    "${DFUCOMSTR}",
                ),
                suffix=".dfu",
                src_suffix=".bin",
            ),
        }
    )


def exists(env):
    try:
        return env["OBJCOPY"]
    except KeyError:
        pass

    if objcopy := env.WhereIs(__OBJCOPY_ARM_BIN):
        return objcopy

    raise SCons.Errors.StopError("Could not detect objcopy for arm")
