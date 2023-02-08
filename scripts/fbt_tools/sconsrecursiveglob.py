import SCons
from SCons.Script import Flatten
from fbt.util import GLOB_FILE_EXCLUSION


def GlobRecursive(env, pattern, node=".", exclude=[]):
    exclude = list(set(Flatten(exclude) + GLOB_FILE_EXCLUSION))
    # print(f"Starting glob for {pattern} from {node} (exclude: {exclude})")
    results = []
    if isinstance(node, str):
        node = env.Dir(node)
    for f in node.glob("*", source=True, exclude=exclude):
        if isinstance(f, SCons.Node.FS.Dir):
            results += env.GlobRecursive(pattern, f, exclude)
    results += node.glob(
        pattern,
        source=True,
        exclude=exclude,
    )
    # print(f"Glob result for {pattern} from {node}: {results}")
    return results


def generate(env):
    env.AddMethod(GlobRecursive)


def exists(env):
    return True
