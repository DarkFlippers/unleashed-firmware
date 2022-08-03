import os


def BuildAppElf(env, app):
    work_dir = env.subst("$EXT_APPS_WORK_DIR")
    app_target_name = os.path.join(work_dir, app.appid)
    app_alias = f"{env['FIRMWARE_BUILD_CFG']}_{app.appid}"
    app_elf = env.Program(
        app_target_name,
        env.GlobRecursive("*.c*", os.path.join(work_dir, app._appdir)),
        APP_ENTRY=app.entry_point,
    )
    app_elf_dump = env.ObjDump(app_target_name)
    env.Alias(f"{app_alias}_list", app_elf_dump)

    app_stripped_elf = env.ELFStripper(
        os.path.join(env.subst("$PLUGIN_ELF_DIR"), app.appid), app_elf
    )
    env.Alias(app_alias, app_stripped_elf)
    return app_stripped_elf


def generate(env, **kw):
    env.SetDefault(EXT_APPS_WORK_DIR=kw.get("EXT_APPS_WORK_DIR", ".extapps"))
    env.VariantDir(env.subst("$EXT_APPS_WORK_DIR"), ".", duplicate=False)
    env.AddMethod(BuildAppElf)


def exists(env):
    return True
