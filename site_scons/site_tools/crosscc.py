from SCons.Errors import StopError
from SCons.Tool import asm
from SCons.Tool import gcc
from SCons.Tool import gxx
from SCons.Tool import ar
from SCons.Tool import gnulink
import strip
import gdb
import objdump

from SCons.Action import _subproc
import subprocess


def prefix_commands(env, command_prefix, cmd_list):
    for command in cmd_list:
        if command in env:
            env[command] = command_prefix + env[command]


def _get_tool_version(env, tool):
    verstr = "version unknown"
    proc = _subproc(
        env,
        env.subst("${%s} --version" % tool),
        stdout=subprocess.PIPE,
        stderr="devnull",
        stdin="devnull",
        universal_newlines=True,
        error="raise",
        shell=True,
    )
    if proc:
        verstr = proc.stdout.readline()
        proc.communicate()
    return verstr


def generate(env, **kw):
    for orig_tool in (asm, gcc, gxx, ar, gnulink, strip, gdb, objdump):
        orig_tool.generate(env)
    env.SetDefault(
        TOOLCHAIN_PREFIX=kw.get("toolchain_prefix"),
    )
    prefix_commands(
        env,
        env.subst("$TOOLCHAIN_PREFIX"),
        [
            "AR",
            "AS",
            "CC",
            "CXX",
            "OBJCOPY",
            "RANLIB",
            "STRIP",
            "GDB",
            "GDBPY",
            "OBJDUMP",
        ],
    )
    # Call CC to check version
    if whitelisted_versions := kw.get("versions", ()):
        cc_version = _get_tool_version(env, "CC")
        # print("CC version =", cc_version)
        # print(list(filter(lambda v: v in cc_version, whitelisted_versions)))
        if not any(filter(lambda v: v in cc_version, whitelisted_versions)):
            raise StopError(
                f"Toolchain version is not supported. Allowed: {whitelisted_versions}, toolchain: {cc_version} "
            )


def exists(env):
    return True
