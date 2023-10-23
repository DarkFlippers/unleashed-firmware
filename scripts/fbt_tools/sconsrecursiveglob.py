import SCons
from fbt.util import GLOB_FILE_EXCLUSION
from SCons.Script import Flatten
from SCons.Node.FS import has_glob_magic


def GlobRecursive(env, pattern, node=".", exclude=[]):
    exclude = list(set(Flatten(exclude) + GLOB_FILE_EXCLUSION))
    # print(f"Starting glob for {pattern} from {node} (exclude: {exclude})")
    results = []
    if isinstance(node, str):
        node = env.Dir(node)
    # Only initiate actual recursion if special symbols can be found in 'pattern'
    if has_glob_magic(pattern):
        for f in node.glob("*", source=True, exclude=exclude):
            if isinstance(f, SCons.Node.FS.Dir):
                results += env.GlobRecursive(pattern, f, exclude)
        results += node.glob(
            pattern,
            source=True,
            exclude=exclude,
        )
    # Otherwise, just assume that file at path exists
    else:
        results.append(node.File(pattern))
    # print(f"Glob result for {pattern} from {node}: {results}")
    return results


def generate(env):
    env.AddMethod(GlobRecursive)


def exists(env):
    return True
