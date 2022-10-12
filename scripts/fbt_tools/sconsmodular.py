import posixpath
import os
from SCons.Errors import UserError


def BuildModule(env, module):
    src_dir = str(env.Dir(".").srcdir or os.getcwd())
    module_sconscript = posixpath.join(src_dir, module, "SConscript")
    if not os.path.exists(module_sconscript):
        module_sconscript = posixpath.join(src_dir, f"{module}.scons")
        if not os.path.exists(module_sconscript):
            raise UserError(f"Cannot build module {module}: scons file not found")

    env.Append(PY_LINT_SOURCES=[module_sconscript])
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
    command = env.Command(phony_name, source, action, **kw)
    env.AlwaysBuild(env.Alias(name, command))
    return command


def ChangeFileExtension(env, fnode, ext):
    return env.File(f"#{os.path.splitext(fnode.path)[0]}{ext}")


def generate(env):
    env.AddMethod(BuildModule)
    env.AddMethod(BuildModules)
    env.AddMethod(PhonyTarget)
    env.AddMethod(ChangeFileExtension)


def exists(env):
    return True
