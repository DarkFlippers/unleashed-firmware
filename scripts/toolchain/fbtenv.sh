#!/bin/sh

# shellcheck disable=SC2034,SC2016,SC2086

# public variables
DEFAULT_SCRIPT_PATH="$(pwd -P)";
SCRIPT_PATH="${SCRIPT_PATH:-$DEFAULT_SCRIPT_PATH}";
FBT_TOOLCHAIN_VERSION="${FBT_TOOLCHAIN_VERSION:-"21"}";
FBT_TOOLCHAIN_PATH="${FBT_TOOLCHAIN_PATH:-$SCRIPT_PATH}";
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
    PATH="$(echo "$PATH" | /usr/bin/sed "s/$TOOLCHAIN_ARCH_DIR_SED\/python\/bin://g")";
    PATH="$(echo "$PATH" | /usr/bin/sed "s/$TOOLCHAIN_ARCH_DIR_SED\/bin://g")";
    PATH="$(echo "$PATH" | /usr/bin/sed "s/$TOOLCHAIN_ARCH_DIR_SED\/protobuf\/bin://g")";
    PATH="$(echo "$PATH" | /usr/bin/sed "s/$TOOLCHAIN_ARCH_DIR_SED\/openocd\/bin://g")";
    PATH="$(echo "$PATH" | /usr/bin/sed "s/$TOOLCHAIN_ARCH_DIR_SED\/openssl\/bin://g")";
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
    export PYTHONNOUSERSITE="$SAVED_PYTHONNOUSERSITE";
    export PYTHONPATH="$SAVED_PYTHONPATH";
    export PYTHONHOME="$SAVED_PYTHONHOME";

    unset SAVED_SSL_CERT_FILE;
    unset SAVED_REQUESTS_CA_BUNDLE;
    unset SAVED_PYTHONNOUSERSITE;
    unset SAVED_PYTHONPATH;
    unset SAVED_PYTHONHOME;

    unset SCRIPT_PATH;
    unset FBT_TOOLCHAIN_VERSION;
    unset FBT_TOOLCHAIN_PATH;
}

fbtenv_check_sourced()
{
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
    echo "Warning! FBT environment script was sourced more than once!";
    echo "You might be doing things wrong, please open a new shell!";
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

fbtenv_check_script_path()
{
    if [ ! -x "$SCRIPT_PATH/fbt" ] && [ ! -x "$SCRIPT_PATH/ufbt" ] ; then
        echo "Please source this script from [u]fbt root directory, or specify 'SCRIPT_PATH' variable manually";
        echo "Example:";
        printf "\tSCRIPT_PATH=lang/c/flipperzero-firmware source lang/c/flipperzero-firmware/scripts/fbtenv.sh\n";
        echo "If current directory is right, type 'unset SCRIPT_PATH' and try again"
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
        TOOLCHAIN_URL="https://update.flipperzero.one/builds/toolchain/gcc-arm-none-eabi-10.3-x86_64-darwin-flipper-$FBT_TOOLCHAIN_VERSION.tar.gz";
    elif [ "$SYS_TYPE" = "Linux" ]; then
        TOOLCHAIN_ARCH_DIR="$FBT_TOOLCHAIN_PATH/toolchain/x86_64-linux";
        TOOLCHAIN_URL="https://update.flipperzero.one/builds/toolchain/gcc-arm-none-eabi-10.3-x86_64-linux-flipper-$FBT_TOOLCHAIN_VERSION.tar.gz";
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
        if ! /usr/bin/pgrep -q oahd; then
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

fbtenv_remove_old_tooclhain()
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
    echo "Unpacking toolchain:";
    tar -xvf "$FBT_TOOLCHAIN_PATH/toolchain/$TOOLCHAIN_TAR" -C "$FBT_TOOLCHAIN_PATH/toolchain" 2>&1 | fbtenv_show_unpack_percentage;
    mkdir -p "$FBT_TOOLCHAIN_PATH/toolchain" || return 1;
    mv "$FBT_TOOLCHAIN_PATH/toolchain/$TOOLCHAIN_DIR" "$TOOLCHAIN_ARCH_DIR" || return 1;
    echo "done";
    return 0;
}

fbtenv_clearing()
{
    printf "Cleaning up..";
    if [ -n "${FBT_TOOLCHAIN_PATH:-""}" ]; then
        rm -rf "${FBT_TOOLCHAIN_PATH:?}/toolchain/"*.tar.gz;
        rm -rf "${FBT_TOOLCHAIN_PATH:?}/toolchain/"*.part;
    fi
    echo "done";
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
    trap fbtenv_clearing 2;  # trap will be restored in fbtenv_clearing
    if ! fbtenv_check_downloaded_toolchain; then
        fbtenv_curl_wget_check || return 1;
        fbtenv_download_toolchain_tar || return 1;
    fi
    fbtenv_remove_old_tooclhain;
    fbtenv_unpack_toolchain || return 1;
    fbtenv_clearing;
    return 0;
}

fbtenv_print_version()
{
    if [ -n "$FBT_VERBOSE" ]; then
        echo "FBT: using toolchain version $(cat "$TOOLCHAIN_ARCH_DIR/VERSION")";
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
    fbtenv_check_if_sourced_multiple_times;  # many source it's just a warning
    fbtenv_check_script_path || return 1;
    fbtenv_check_download_toolchain || return 1;
    fbtenv_set_shell_prompt;
    fbtenv_print_version;
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
}

fbtenv_main "${1:-""}";
