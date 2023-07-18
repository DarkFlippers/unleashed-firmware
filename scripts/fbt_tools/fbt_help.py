targets_help = """Configuration variables:
"""

tail_help = """

TASKS:
Firmware & apps:
    firmware_all, fw_dist:
        Build firmware; create distribution package
    faps, fap_dist:
        Build all FAP apps
    fap_{APPID}, build APPSRC={APPID}; launch APPSRC={APPID}:
        Build FAP app with appid={APPID}; upload & start it over USB
    fap_deploy:
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
        Generate compilation_database.json
    lint, lint_py:
        run linters
    format, format_py:
        run code formatters
    firmware_pvs:
        generate a PVS-Studio report

How to open a shell with toolchain environment and other build tools:
    In your shell, type "source `./fbt -s env`". You can also use "." instead of "source".

For more targets & info, see documentation/fbt.md
"""


def generate(env, **kw):
    vars = kw["vars"]
    basic_help = vars.GenerateHelpText(env)
    env.Help(targets_help + basic_help + tail_help)


def exists(env):
    return True
