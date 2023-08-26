#!/bin/sh

# shellcheck disable=SC2034,SC2016,SC2086

# public variables
DEFAULT_SCRIPT_PATH="$(pwd -P)";
FBT_TOOLCHAIN_VERSION="${FBT_TOOLCHAIN_VERSION:-"23"}";

if [ -z ${FBT_TOOLCHAIN_PATH+x} ] ; then
    FBT_TOOLCHAIN_PATH_WAS_SET=0;
else
    FBT_TOOLCHAIN_PATH_WAS_SET=1;
fi

FBT_TOOLCHAIN_PATH="${FBT_TOOLCHAIN_PATH:-$DEFAULT_SCRIPT_PATH}";
FBT_VERBOSE="${FBT_VERBOSE:-""}";

fbtenv_show_usage()
{
    echo "Running this script manually is wrong, please source it";
    echo "Example:";
    printf "\tsource scripts/toolchain/fbtenv.sh\n";
    echo "To restore your environment, source fbtenv.sh with '--restore'."
    echo "Example:";
    printf "\tsource scripts/toolchain/fbtenv.sh --restore\n";
}

fbtenv_curl()
{
    curl --progress-bar -SLo "$1" "$2";
}

fbtenv_wget()
{
    wget --show-progress --progress=bar:force -qO "$1" "$2";
}

fbtenv_restore_env()
{
    TOOLCHAIN_ARCH_DIR_SED="$(echo "$TOOLCHAIN_ARCH_DIR" | sed 's/\//\\\//g')"
    PATH="$(echo "$PATH" | sed "s/$TOOLCHAIN_ARCH_DIR_SED\/python\/bin://g")";
    PATH="$(echo "$PATH" | sed "s/$TOOLCHAIN_ARCH_DIR_SED\/bin://g")";
    PATH="$(echo "$PATH" | sed "s/$TOOLCHAIN_ARCH_DIR_SED\/protobuf\/bin://g")";
    PATH="$(echo "$PATH" | sed "s/$TOOLCHAIN_ARCH_DIR_SED\/openocd\/bin://g")";
    PATH="$(echo "$PATH" | sed "s/$TOOLCHAIN_ARCH_DIR_SED\/openssl\/bin://g")";
    if [ -n "${PS1:-""}" ]; then
        PS1="$(echo "$PS1" | sed 's/\[fbt\]//g')";
    elif [ -n "${PROMPT:-""}" ]; then
        PROMPT="$(echo "$PROMPT" | sed 's/\[fbt\]//g')";
    fi

    if [ -n "$SAVED_SSL_CERT_FILE" ]; then
        export SSL_CERT_FILE="$SAVED_SSL_CERT_FILE";
        export REQUESTS_CA_BUNDLE="$SAVED_REQUESTS_CA_BUNDLE";
    else
        unset SSL_CERT_FILE;
        unset REQUESTS_CA_BUNDLE;
    fi

    if [ "$SYS_TYPE" = "Linux" ]; then
        if [ -n "$SAVED_TERMINFO_DIRS" ]; then
            export TERMINFO_DIRS="$SAVED_TERMINFO_DIRS";
        else
            unset TERMINFO_DIRS;
        fi
        unset SAVED_TERMINFO_DIRS;
    fi

    export PYTHONNOUSERSITE="$SAVED_PYTHONNOUSERSITE";
    export PYTHONPATH="$SAVED_PYTHONPATH";
    export PYTHONHOME="$SAVED_PYTHONHOME";

    unset SAVED_SSL_CERT_FILE;
    unset SAVED_REQUESTS_CA_BUNDLE;
    unset SAVED_PYTHONNOUSERSITE;
    unset SAVED_PYTHONPATH;
    unset SAVED_PYTHONHOME;

    unset FBT_TOOLCHAIN_VERSION;
    unset FBT_TOOLCHAIN_PATH;
}

fbtenv_check_sourced()
{
    if [ -n "${FBT_SKIP_CHECK_SOURCED:-""}" ]; then
        return 0;
    fi
    case "${ZSH_EVAL_CONTEXT:-""}" in *:file:*)
        setopt +o nomatch;  # disabling 'no match found' warning in zsh
        return 0;;
    esac
    if [ ${0##*/} = "fbtenv.sh" ]; then  # exluding script itself
        fbtenv_show_usage;
        return 1;
    fi
    case ${0##*/} in dash|-dash|bash|-bash|ksh|-ksh|sh|-sh|*.sh|fbt|ufbt)
        return 0;;
    esac
    fbtenv_show_usage;
    return 1;
}

fbtenv_check_if_sourced_multiple_times()
{
    if ! echo "${PS1:-""}" | grep -qF "[fbt]"; then
        if ! echo "${PROMPT:-""}" | grep -qF "[fbt]"; then
            return 0;
        fi
    fi
    return 1;
}

fbtenv_set_shell_prompt()
{
    if [ -n "${PS1:-""}" ]; then
        PS1="[fbt]$PS1";
    elif [ -n "${PROMPT:-""}" ]; then
        PROMPT="[fbt]$PROMPT";
    fi
    return 0;  # all other shells
}

fbtenv_check_env_vars()
{
    # Return error if FBT_TOOLCHAIN_PATH is not set before script is sourced or if fbt executable is not in DEFAULT_SCRIPT_PATH
    if [ "$FBT_TOOLCHAIN_PATH_WAS_SET" -eq 0 ] && [ ! -x "$DEFAULT_SCRIPT_PATH/fbt" ] && [ ! -x "$DEFAULT_SCRIPT_PATH/ufbt" ] ; then
        echo "Please source this script from [u]fbt root directory, or specify 'FBT_TOOLCHAIN_PATH' variable manually";
        echo "Example:";
        printf "\tFBT_TOOLCHAIN_PATH=lang/c/flipperzero-firmware source lang/c/flipperzero-firmware/scripts/fbtenv.sh\n";
        echo "If current directory is right, type 'unset FBT_TOOLCHAIN_PATH' and try again"
        return 1;
    fi
    return 0;
}

fbtenv_get_kernel_type()
{
    SYS_TYPE="$(uname -s)";
    ARCH_TYPE="$(uname -m)";
    if [ "$ARCH_TYPE" != "x86_64" ] && [ "$SYS_TYPE" != "Darwin" ]; then
        echo "We only provide toolchain for x86_64 CPUs, sorry..";
        return 1;
    fi
    if [ "$SYS_TYPE" = "Darwin" ]; then
        fbtenv_check_rosetta || return 1;
        TOOLCHAIN_ARCH_DIR="$FBT_TOOLCHAIN_PATH/toolchain/x86_64-darwin";
        if [ -z "${FBT_TOOLS_CUSTOM_LINK:-}" ]; then
            TOOLCHAIN_URL="https://update.flipperzero.one/builds/toolchain/gcc-arm-none-eabi-10.3-x86_64-darwin-flipper-$FBT_TOOLCHAIN_VERSION.tar.gz";
        else
            echo "info: custom toolchain link is used";
            TOOLCHAIN_URL=$FBT_TOOLS_CUSTOM_LINK;
        fi
    elif [ "$SYS_TYPE" = "Linux" ]; then
        TOOLCHAIN_ARCH_DIR="$FBT_TOOLCHAIN_PATH/toolchain/x86_64-linux";
        if [ -z "${FBT_TOOLS_CUSTOM_LINK:-}" ]; then
            TOOLCHAIN_URL="https://update.flipperzero.one/builds/toolchain/gcc-arm-none-eabi-10.3-x86_64-linux-flipper-$FBT_TOOLCHAIN_VERSION.tar.gz";
        else
            echo "info: custom toolchain link is used";
            TOOLCHAIN_URL=$FBT_TOOLS_CUSTOM_LINK;
        fi
    elif echo "$SYS_TYPE" | grep -q "MINGW"; then
        echo "In MinGW shell, use \"[u]fbt.cmd\" instead of \"[u]fbt\"";
        return 1;
    else
        echo "Your system configuration is not supported. Sorry.. Please report us your configuration.";
        return 1;
    fi
    return 0;
}

fbtenv_check_rosetta()
{
    if [ "$ARCH_TYPE" = "arm64" ]; then
        if ! pgrep -q oahd; then
            echo "Flipper Zero Toolchain needs Rosetta2 to run under Apple Silicon";
            echo "Please instal it by typing 'softwareupdate --install-rosetta --agree-to-license'";
            return 1;
        fi
    fi
    return 0;
}

fbtenv_check_tar()
{
    printf "Checking for tar..";
    if ! tar --version > /dev/null 2>&1; then
        echo "no";
        return 1;
    fi
    echo "yes";
    return 0;
}

fbtenv_check_downloaded_toolchain()
{
    printf "Checking if downloaded toolchain tgz exists..";
    if [ ! -f "$FBT_TOOLCHAIN_PATH/toolchain/$TOOLCHAIN_TAR" ]; then
        echo "no";
        return 1;
    fi
    echo "yes";
    return 0;
}

fbtenv_download_toolchain_tar()
{
    echo "Downloading toolchain:";
    mkdir -p "$FBT_TOOLCHAIN_PATH/toolchain" || return 1;
    "$FBT_DOWNLOADER" "$FBT_TOOLCHAIN_PATH/toolchain/$TOOLCHAIN_TAR.part" "$TOOLCHAIN_URL" || return 1;
    # restoring oroginal filename if file downloaded successfully
    mv "$FBT_TOOLCHAIN_PATH/toolchain/$TOOLCHAIN_TAR.part" "$FBT_TOOLCHAIN_PATH/toolchain/$TOOLCHAIN_TAR"
    echo "done";
    return 0;
}

fbtenv_remove_old_toolchain()
{
    printf "Removing old toolchain..";
    rm -rf "${TOOLCHAIN_ARCH_DIR:?}";
    echo "done";
}

fbtenv_show_unpack_percentage()
{
    LINE=0;
    while read -r line; do
        LINE=$(( LINE + 1 ));
        if [ $(( LINE % 300 )) -eq 0 ]; then
            printf "#";
        fi
    done
    echo " 100.0%";
}

fbtenv_unpack_toolchain()
{
    echo "Unpacking toolchain to '$FBT_TOOLCHAIN_PATH/toolchain':";
    tar -xvf "$FBT_TOOLCHAIN_PATH/toolchain/$TOOLCHAIN_TAR" -C "$FBT_TOOLCHAIN_PATH/toolchain" 2>&1 | fbtenv_show_unpack_percentage;
    mkdir -p "$FBT_TOOLCHAIN_PATH/toolchain" || return 1;
    mv "$FBT_TOOLCHAIN_PATH/toolchain/$TOOLCHAIN_DIR" "$TOOLCHAIN_ARCH_DIR" || return 1;
    echo "done";
    return 0;
}

fbtenv_cleanup()
{
    if [ -n "${FBT_TOOLCHAIN_PATH:-""}" ]; then
        printf "Cleaning up..";
        rm -rf "${FBT_TOOLCHAIN_PATH:?}/toolchain/"*.part;
        if [ -z "${FBT_PRESERVE_TAR:-""}" ]; then
            rm -rf "${FBT_TOOLCHAIN_PATH:?}/toolchain/"*.tar.gz;
        fi
        echo "done";
    fi
    trap - 2;
    return 0;
}

fbtenv_curl_wget_check()
{
    printf "Checking curl..";
    if ! curl --version > /dev/null 2>&1; then
        echo "no";
        printf "Checking wget..";
        if ! wget --version > /dev/null 2>&1; then
            echo "no";
            echo "No curl or wget found in your PATH";
            echo "Please provide it or download this file:";
            echo;
            echo "$TOOLCHAIN_URL";
            echo;
            echo "And place in $FBT_TOOLCHAIN_PATH/toolchain/ dir manually";
            return 1;
        fi
        echo "yes"
        FBT_DOWNLOADER="fbtenv_wget";
        return 0;
    fi
    echo "yes"
    FBT_DOWNLOADER="fbtenv_curl";
    return 0;
}

fbtenv_check_download_toolchain()
{
    if [ ! -d "$TOOLCHAIN_ARCH_DIR" ]; then
        fbtenv_download_toolchain || return 1;
    elif [ ! -f "$TOOLCHAIN_ARCH_DIR/VERSION" ]; then
        fbtenv_download_toolchain || return 1;
    elif [ "$(cat "$TOOLCHAIN_ARCH_DIR/VERSION")" -ne "$FBT_TOOLCHAIN_VERSION" ]; then
        echo "FBT: starting toolchain upgrade process.."
        fbtenv_download_toolchain || return 1;
    fi
    return 0;
}

fbtenv_download_toolchain()
{
    fbtenv_check_tar || return 1;
    TOOLCHAIN_TAR="$(basename "$TOOLCHAIN_URL")";
    TOOLCHAIN_DIR="$(echo "$TOOLCHAIN_TAR" | sed "s/-$FBT_TOOLCHAIN_VERSION.tar.gz//g")";
    trap fbtenv_cleanup 2;  # trap will be restored in fbtenv_cleanup
    if ! fbtenv_check_downloaded_toolchain; then
        fbtenv_curl_wget_check || return 1;
        fbtenv_download_toolchain_tar || return 1;
    fi
    fbtenv_remove_old_toolchain;
    fbtenv_unpack_toolchain || return 1;
    fbtenv_cleanup;
    return 0;
}

fbtenv_print_config()
{
    if [ -n "${FBT_VERBOSE:-""}" ]; then
        echo "FBT: using toolchain version $(cat "$TOOLCHAIN_ARCH_DIR/VERSION")";
        if [ -n "${FBT_SKIP_CHECK_SOURCED:-""}" ]; then
            echo "FBT: fbtenv will not check if it is sourced or not";
        fi
        if [ -n "${FBT_PRESERVE_TAR:-""}" ]; then
            echo "FBT: toolchain archives will be saved";
        fi
    fi
}

fbtenv_main()
{
    fbtenv_check_sourced || return 1;
    fbtenv_get_kernel_type || return 1;
    if [ "$1" = "--restore" ]; then
        fbtenv_restore_env;
        return 0;
    fi
    if ! fbtenv_check_if_sourced_multiple_times; then
        return 0;
    fi;
    fbtenv_check_env_vars || return 1;
    fbtenv_check_download_toolchain || return 1;
    fbtenv_set_shell_prompt;
    fbtenv_print_config;
    PATH="$TOOLCHAIN_ARCH_DIR/python/bin:$PATH";
    PATH="$TOOLCHAIN_ARCH_DIR/bin:$PATH";
    PATH="$TOOLCHAIN_ARCH_DIR/protobuf/bin:$PATH";
    PATH="$TOOLCHAIN_ARCH_DIR/openocd/bin:$PATH";
    PATH="$TOOLCHAIN_ARCH_DIR/openssl/bin:$PATH";
    export PATH;

    export SAVED_SSL_CERT_FILE="${SSL_CERT_FILE:-""}";
    export SAVED_REQUESTS_CA_BUNDLE="${REQUESTS_CA_BUNDLE:-""}";
    export SAVED_PYTHONNOUSERSITE="${PYTHONNOUSERSITE:-""}";
    export SAVED_PYTHONPATH="${PYTHONPATH:-""}";
    export SAVED_PYTHONHOME="${PYTHONHOME:-""}";

    export SSL_CERT_FILE="$TOOLCHAIN_ARCH_DIR/python/lib/python3.11/site-packages/certifi/cacert.pem";
    export REQUESTS_CA_BUNDLE="$SSL_CERT_FILE";
    export PYTHONNOUSERSITE=1;
    export PYTHONPATH=;
    export PYTHONHOME=;

    if [ "$SYS_TYPE" = "Linux" ]; then
        export SAVED_TERMINFO_DIRS="${TERMINFO_DIRS:-""}";
        export TERMINFO_DIRS="$TOOLCHAIN_ARCH_DIR/ncurses/share/terminfo";
    fi
}

fbtenv_main "${1:-""}";
