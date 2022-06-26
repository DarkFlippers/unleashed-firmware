import SCons


def GlobRecursive(env, pattern, node=".", exclude=None):
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
    # print(f"Glob for {pattern} from {node}: {results}")
    return results


def generate(env):
    env.AddMethod(GlobRecursive)


def exists(env):
    return True
