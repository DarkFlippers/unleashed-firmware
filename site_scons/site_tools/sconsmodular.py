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


def PhonyTarget(env, name, action, source=None, **kw):
    if not source:
        source = []
    phony_name = "phony_" + name
    env.Pseudo(phony_name)
    return env.AlwaysBuild(
        env.Alias(name, env.Command(phony_name, source, action, **kw))
    )


def generate(env):
    env.AddMethod(BuildModule)
    env.AddMethod(BuildModules)
    env.AddMethod(PhonyTarget)


def exists(env):
    return True
