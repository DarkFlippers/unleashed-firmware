#!/bin/sh

# unofficial strict mode
set -eu;

FLIPPER_TOOLCHAIN_VERSION="3";

get_kernel_type()
{
    SYS_TYPE="$(uname -s)"
    if [ "$SYS_TYPE" = "Darwin" ]; then
        TOOLCHAIN_PATH="toolchain/x86_64-darwin";
    elif [ "$SYS_TYPE" = "Linux" ]; then
        TOOLCHAIN_PATH="toolchain/x86_64-linux";
    elif echo "$SYS_TYPE" | grep -q "MINGW"; then
        echo "In MinGW shell use \"fbt.cmd\" instead of \"fbt\"";
        exit 1;
    else
        echo "Sorry, your system is not supported. Please report your configuration to us.";
        exit 1;
    fi
}

check_download_toolchain()
{
    if [ ! -d "$SCRIPT_PATH/$TOOLCHAIN_PATH" ]; then
        download_toolchain;
    elif [ ! -f "$SCRIPT_PATH/$TOOLCHAIN_PATH/VERSION" ]; then
        download_toolchain;
    elif [ "$(cat "$SCRIPT_PATH/$TOOLCHAIN_PATH/VERSION")" -ne "$FLIPPER_TOOLCHAIN_VERSION" ]; then
        download_toolchain;
    fi
}

download_toolchain()
{
    chmod 755 "$SCRIPT_PATH/scripts/toolchain/unix-toolchain-download.sh";
    "$SCRIPT_PATH/scripts/toolchain/unix-toolchain-download.sh" "$FLIPPER_TOOLCHAIN_VERSION" || exit 1;
}

main()
{
    if [ -z "${SCRIPT_PATH:-}" ]; then
        echo "Manual running of this script is not allowed.";
        exit 1;
    fi
    get_kernel_type;  # sets TOOLCHAIN_PATH
    check_download_toolchain;
    PATH="$SCRIPT_PATH/$TOOLCHAIN_PATH/python/bin:$PATH";
    PATH="$SCRIPT_PATH/$TOOLCHAIN_PATH/bin:$PATH";
    PATH="$SCRIPT_PATH/$TOOLCHAIN_PATH/protobuf/bin:$PATH";
    PATH="$SCRIPT_PATH/$TOOLCHAIN_PATH/openocd/bin:$PATH";
}
main;
