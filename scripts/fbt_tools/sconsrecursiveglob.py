import itertools

import SCons
from fbt.util import GLOB_FILE_EXCLUSION
from SCons.Node.FS import has_glob_magic
from SCons.Script import Flatten


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
    ## Debug
    # print(f"Glob result for {pattern} from {node}: {results}")
    return results


def GatherSources(env, sources_list, node="."):
    """Glob every include pattern in sources_list, minus its "!exclusions".

    Both halves have to arrive in the same call - an exclusion only filters the patterns it is
    handed with. An exclusion is a bare leaf name, matched at any depth: "!plugins" drops every
    directory called plugins under node, not just the top one.

    Order is preserved, not sorted: it becomes the link order of whatever is built from it, so
    de-duplicating through a set() here would make the output depend on PYTHONHASHSEED.
    """
    sources_list = list(dict.fromkeys(Flatten(sources_list)))
    include_sources = list(filter(lambda x: not x.startswith("!"), sources_list))
    exclude_sources = list(x[1:] for x in sources_list if x.startswith("!"))
    gathered_sources = list(
        itertools.chain.from_iterable(
            env.GlobRecursive(
                source_type,
                node,
                exclude=exclude_sources,
            )
            for source_type in include_sources
        )
    )
    ## Debug
    # print(
    #     f"Gathered sources for {sources_list} from {node}: {list(f.path for f in gathered_sources)}"
    # )
    return gathered_sources


def generate(env):
    env.AddMethod(GlobRecursive)
    env.AddMethod(GatherSources)


def exists(env):
    return True
