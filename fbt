#!/bin/sh

# shellcheck disable=SC2086 source=/dev/null
# unofficial strict mode
set -eu;

# private variables
N_CORES="$(getconf _NPROCESSORS_ONLN)";
N_GIT_THREADS="$(($N_CORES * 2))";
SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd -P)";
SCONS_DEFAULT_FLAGS="--warn=target-not-built";
SCONS_EP="python3 -m SCons";

# public variables
FBT_NO_SYNC="${FBT_NO_SYNC:-""}";
FBT_TOOLCHAIN_PATH="${FBT_TOOLCHAIN_PATH:-$SCRIPT_PATH}";
FBT_VERBOSE="${FBT_VERBOSE:-""}";
FBT_GIT_SUBMODULE_SHALLOW="${FBT_GIT_SUBMODULE_SHALLOW:-""}";

FBT_VERBOSE="$FBT_VERBOSE" . "$SCRIPT_PATH/scripts/toolchain/fbtenv.sh";

if [ -z "$FBT_VERBOSE" ]; then
    SCONS_DEFAULT_FLAGS="$SCONS_DEFAULT_FLAGS -Q";
fi

if [ -z "$FBT_NO_SYNC" ]; then
    if [ ! -e "$SCRIPT_PATH/.git" ]; then
        echo "\".git\" directory not found, please clone repo via \"git clone\"";
        exit 1;
    fi
    _FBT_CLONE_FLAGS="--jobs $N_GIT_THREADS";
    if [ ! -z "$FBT_GIT_SUBMODULE_SHALLOW" ]; then
        _FBT_CLONE_FLAGS="$_FBT_CLONE_FLAGS --depth 1";
    fi

    git submodule update --init --recursive $_FBT_CLONE_FLAGS;
fi

$SCONS_EP $SCONS_DEFAULT_FLAGS "$@"
