targets_help = """Configuration variables:
"""

tail_help = """

TASKS:
Building:
    firmware_all, fw_dist:
        Build firmware; create distribution package
    faps, fap_dist:
        Build all FAP apps
    fap_{APPID}, launch_app APPSRC={APPID}:
        Build FAP app with appid={APPID}; upload & start it over USB
    faps_copy:
        Build and upload all FAP apps over USB

Flashing & debugging:
    flash, flash_blackmagic, jflash:
        Flash firmware to target using debug probe
    flash_usb, flash_usb_full: 
        Install firmware using self-update package
    debug, debug_other, blackmagic: 
        Start GDB

Other:
    cli:
        Open a Flipper CLI session over USB
    firmware_cdb, updater_cdb:
        Generate сompilation_database.json
    lint, lint_py:
        run linters
    format, format_py:
        run code formatters

For more targets & info, see documentation/fbt.md
"""


def generate(env, **kw):
    vars = kw["vars"]
    basic_help = vars.GenerateHelpText(env)
    env.Help(targets_help + basic_help + tail_help)


def exists(env):
    return True
