import os
import sys
import traceback

import SCons.Warnings as Warnings
from ansi.color import fg
from SCons.Errors import UserError

# from SCons.Script.Main import find_deepest_user_frame


def find_deepest_user_frame(tb):
    tb.reverse()

    # find the deepest traceback frame that is not part
    # of SCons:
    for frame in tb:
        filename = frame[0]
        if filename.find("fbt_tweaks") != -1:
            continue
        if filename.find(os.sep + "SCons" + os.sep) == -1:
            return frame
    return tb[0]


def fbt_warning(e):
    filename, lineno, routine, dummy = find_deepest_user_frame(
        traceback.extract_stack()
    )
    fbt_line = "\nfbt: warning: %s\n" % e.args[0]
    sys.stderr.write(fg.boldmagenta(fbt_line))
    fbt_line = (
        fg.yellow("%s, line %d, " % (routine, lineno)) + 'in file "%s"\n' % filename
    )
    sys.stderr.write(fg.yellow(fbt_line))


def generate(env):
    if env.get("UFBT_WORK_DIR"):
        raise UserError(
            "You're trying to use a new format SDK on a legacy ufbt version. "
            "Please update ufbt to a version from PyPI: https://pypi.org/project/ufbt/"
        )
    Warnings._warningOut = fbt_warning


def exists():
    return True
