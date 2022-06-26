import posixpath
import os


def BuildModule(env, module):
    src_dir = str(env.Dir(".").srcdir or os.getcwd())
    module_sconscript = posixpath.join(src_dir, module, "SConscript")
    if not os.path.exists(module_sconscript):
        module_sconscript = posixpath.join(src_dir, f"{module}.scons")
        if not os.path.exists(module_sconscript):
            print(f"Cannot build module {module}: scons file not found")
            Exit(2)

    return env.SConscript(
        module_sconscript,
        variant_dir=posixpath.join(env.subst("$BUILD_DIR"), module),
        duplicate=0,
    )


def BuildModules(env, modules):
    result = []
    for module in modules:
        build_res = env.BuildModule(module)
        # print("module ", module, build_res)
        if build_res is None:
            continue
        result.append(build_res)
    return result


def generate(env):
    env.AddMethod(BuildModule)
    env.AddMethod(BuildModules)


def exists(env):
    return True
