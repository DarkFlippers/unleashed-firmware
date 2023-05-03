import atexit

import SCons.Errors
from ansi.color import fg, fx
from SCons.Script import GetBuildFailures


def bf_to_str(bf):
    """Convert an element of GetBuildFailures() to a string
    in a useful way."""

    if bf is None:  # unknown targets product None in list
        return "(unknown tgt)"
    elif isinstance(bf, SCons.Errors.StopError):
        return fg.yellow(str(bf))
    elif bf.node:
        return fg.yellow(str(bf.node)) + ": " + bf.errstr
    elif bf.filename:
        return fg.yellow(bf.filename) + ": " + bf.errstr
    return fg.yellow("unknown failure: ") + bf.errstr


def display_build_status():
    """Display the build status.  Called by atexit.
    Here you could do all kinds of complicated things."""
    bf = GetBuildFailures()
    if bf:
        # bf is normally a list of build failures; if an element is None,
        # it's because of a target that scons doesn't know anything about.
        failures_message = "\n".join([bf_to_str(x) for x in bf if x is not None])
        print()
        print(fg.brightred(fx.bold("*" * 10 + " FBT ERRORS " + "*" * 10)))
        print(failures_message)


atexit.register(display_build_status)
