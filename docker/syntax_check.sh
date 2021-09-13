#!/usr/bin/env bash

# set -e

CLANG_FORMAT_BIN="/usr/bin/clang-format-12"

PROJECT_DIR=$(pwd)

cd "$PROJECT_DIR" || exit 

echo "RUN C\C++ SYNTAX CHECK"
C_FILES=$(find . \
    -not \( -path './firmware/.obj' -prune \) \
    -not \( -path './firmware/targets' -prune \) \
    -not \( -path './assets' -prune \) \
    -not \( -path ./lib -prune \) \
    -name *.c -o -name *.h -o -name *.cpp)

ulimit -s 65536
$CLANG_FORMAT_BIN --version
errors=$($CLANG_FORMAT_BIN --verbose -style=file -n --Werror --ferror-limit=0 $C_FILES |& tee /dev/stderr | sed '/^Formatting/d')

if [[ -z "$errors" ]]; then
    echo "Code looks fine for me!"
    exit 0
fi

if [[ -n "${SET_GH_OUTPUT}" ]]; then
    errors="${errors//'%'/'%25'}"
    errors="${errors//$'\n'/'%0A'}"
    errors="${errors//$'\r'/'%0D'}"
    echo "::set-output name=errors::$errors"
fi

read -p "Do you want fix syntax? (y/n): " confirm && [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1

cd "$PROJECT_DIR" || exit 

# We use root in container and clang-format rewriting files. We'll need change owner to original
local_user=$(stat -c '%u' .clang-format)
$CLANG_FORMAT_BIN -style=file -i $C_FILES
chown $local_user $C_FILES
