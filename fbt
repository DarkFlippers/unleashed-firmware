#!/bin/sh

# shellcheck disable=SC2086 source=/dev/null
# unofficial strict mode
set -eu;

# private variables
SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd -P)";
SCONS_DEFAULT_FLAGS="-Q --warn=target-not-built";

# public variables
FBT_NOENV="${FBT_NOENV:-""}";
FBT_NO_SYNC="${FBT_NO_SYNC:-""}";
FBT_TOOLCHAIN_PATH="${FBT_TOOLCHAIN_PATH:-$SCRIPT_PATH}";

if [ -z "$FBT_NOENV" ]; then
    . "$SCRIPT_PATH/scripts/toolchain/fbtenv.sh";
fi

if [ -z "$FBT_NO_SYNC" ]; then
    if [ ! -d "$SCRIPT_PATH/.git" ]; then
        echo "\".git\" directory not found, please clone repo via \"git clone --recursive\"";
        exit 1;
    fi
    git submodule update --init;
fi

python3 "$SCRIPT_PATH/lib/scons/scripts/scons.py" $SCONS_DEFAULT_FLAGS "$@"
