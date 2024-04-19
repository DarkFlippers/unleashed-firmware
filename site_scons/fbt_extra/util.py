from ansi.color import fg
from fbt.util import link_dir


def link_elf_dir_as_latest(env, elf_node):
    elf_dir = elf_node.Dir(".")
    latest_dir = env.Dir("#build/latest")
    print(fg.green(f"Linking {elf_dir} as latest built dir (./build/latest/)"))
    return link_dir(latest_dir.abspath, elf_dir.abspath, env["PLATFORM"] == "win32")


def should_gen_cdb_and_link_dir(env, requested_targets):
    explicitly_building_updater = False
    # Hacky way to check if updater-related targets were requested
    for build_target in requested_targets:
        if "updater" in str(build_target) and "package" not in str(build_target):
            explicitly_building_updater = True

    is_updater = not env["IS_BASE_FIRMWARE"]
    # If updater is explicitly requested, link to the latest updater
    # Otherwise, link to firmware
    return (is_updater and explicitly_building_updater) or (
        not is_updater and not explicitly_building_updater
    )
