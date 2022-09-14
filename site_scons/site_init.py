from SCons.Script import GetBuildFailures

import sys
import os
import atexit

sys.path.insert(0, os.path.join(os.getcwd(), "scripts"))
sys.path.insert(0, os.path.join(os.getcwd(), "lib/cxxheaderparser"))


def bf_to_str(bf):
    """Convert an element of GetBuildFailures() to a string
    in a useful way."""
    import SCons.Errors

    if bf is None:  # unknown targets product None in list
        return "(unknown tgt)"
    elif isinstance(bf, SCons.Errors.StopError):
        return str(bf)
    elif bf.node:
        return str(bf.node) + ": " + bf.errstr
    elif bf.filename:
        return bf.filename + ": " + bf.errstr
    return "unknown failure: " + bf.errstr


def display_build_status():
    """Display the build status.  Called by atexit.
    Here you could do all kinds of complicated things."""
    bf = GetBuildFailures()
    if bf:
        # bf is normally a list of build failures; if an element is None,
        # it's because of a target that scons doesn't know anything about.
        failures_message = "\n".join(
            ["Failed building %s" % bf_to_str(x) for x in bf if x is not None]
        )
        print("*" * 10, "ERRORS", "*" * 10)
        print(failures_message)


atexit.register(display_build_status)
