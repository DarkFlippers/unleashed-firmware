#!/bin/sh
# shellcheck disable=SC2086,SC2034

# unofficial strict mode
set -eu;

check_system()
{
    VER="$1";  # toolchain version
    printf "Checking kernel type..";
    SYS_TYPE="$(uname -s)"
    if [ "$SYS_TYPE" = "Darwin" ]; then
        echo "darwin";
        TOOLCHAIN_URL="https://update.flipperzero.one/builds/toolchain/gcc-arm-none-eabi-10.3-x86_64-darwin-flipper-$VER.tar.gz";
        TOOLCHAIN_PATH="toolchain/x86_64-darwin";
    elif [ "$SYS_TYPE" = "Linux" ]; then
        echo "linux";
        TOOLCHAIN_URL="https://update.flipperzero.one/builds/toolchain/gcc-arm-none-eabi-10.3-x86_64-linux-flipper-$VER.tar.gz";
        TOOLCHAIN_PATH="toolchain/x86_64-linux";
    else
        echo "unsupported.";
        echo "Your system is unsupported.. sorry..";
        exit 1;
    fi
}

check_tar()
{
    printf "Checking tar..";
    if ! tar --version > /dev/null 2>&1; then
        echo "no";
        exit 1;
    fi
    echo "yes";
}


curl_wget_check()
{
    printf "Checking curl..";
    if ! curl --version > /dev/null 2>&1; then
        echo "no";
        printf "Checking wget..";
        if ! wget --version > /dev/null 2>&1; then
            echo "no";
            echo "No curl or wget found in your PATH.";
            echo "Please provide it or download this file:";
            echo;
            echo "$TOOLCHAIN_URL";
            echo;
            echo "And place in repo root dir mannualy.";
            exit 1;
        fi
        echo "yes"
        DOWNLOADER="wget";
        DOWNLOADER_ARGS="--show-progress --progress=bar:force -qO";
        return;
    fi
    echo "yes"
    DOWNLOADER="curl";
    DOWNLOADER_ARGS="--progress-bar -SLo";
}

check_downloaded_toolchain()
{
    printf "Checking downloaded toolchain tgz..";
    if [ -f "$REPO_ROOT/$TOOLCHAIN_TAR" ]; then
        echo "yes";
        return 0;
    fi
    echo "no";
    return 1;
}

download_toolchain()
{
    echo "Downloading toolchain:";
    "$DOWNLOADER" $DOWNLOADER_ARGS "$REPO_ROOT/$TOOLCHAIN_TAR" "$TOOLCHAIN_URL";
    echo "done";
}

remove_old_tooclhain()
{
    printf "Removing old toolchain (if exist)..";
    rm -rf "${REPO_ROOT:?}/$TOOLCHAIN_PATH";
    echo "done";
}

show_unpack_percentage()
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

unpack_toolchain()
{
    echo "Unpacking toolchain:";
    tar -xvf "$REPO_ROOT/$TOOLCHAIN_TAR" -C "$REPO_ROOT/" 2>&1 | show_unpack_percentage;
    mkdir -p "$REPO_ROOT/toolchain";
    mv "$REPO_ROOT/$TOOLCHAIN_DIR" "$REPO_ROOT/$TOOLCHAIN_PATH/";
    echo "done";
}

clearing()
{
    printf "Clearing..";
    rm -rf "${REPO_ROOT:?}/$TOOLCHAIN_TAR";
    echo "done";
}

main()
{
    SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd -P)"
    REPO_ROOT="$(cd "$SCRIPT_PATH/../../" && pwd)";
    check_system "$1";  # recives TOOLCHAIN_VERSION, defines TOOLCHAIN_URL and TOOLCHAIN_PATH
    check_tar;
    TOOLCHAIN_TAR="$(basename "$TOOLCHAIN_URL")";
    TOOLCHAIN_DIR="$(echo "$TOOLCHAIN_TAR" | sed "s/-$VER.tar.gz//g")";
    if ! check_downloaded_toolchain; then
        curl_wget_check;
        download_toolchain;
    fi
    remove_old_tooclhain;
    unpack_toolchain;
}

trap clearing EXIT;
trap clearing 2;  # SIGINT not coverable by EXIT
main "$1";  # toochain version
