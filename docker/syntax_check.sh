#!/usr/bin/env bash

# set -e

CLANG_FORMAT_BIN="/usr/bin/clang-format-12"
PATH="$HOME/.cargo/bin:${PATH}"

PROJECT_DIR=$(pwd)

cd $PROJECT_DIR

echo "RUN C\C++ SYNTAX CHECK"
C_FILES=$(find . \
    -not \( -path './firmware/.obj' -prune \) \
    -not \( -path './firmware/targets' -prune \) \
    -not \( -path './assets' -prune \) \
    -not \( -path ./lib -prune \) \
    -name *.c -o -name *.h -o -name *.cpp)

ulimit -s 65536
$CLANG_FORMAT_BIN --verbose -style=file -n --Werror --ferror-limit=0 $C_FILES
c_syntax_rc=$?

echo "RUN RUST SYNTAX CHECK"
cd $PROJECT_DIR/core-rs && cargo fmt -- --check
rust_syntax_rc=$?

if [[ $rust_syntax_rc -eq 0 ]] && [[ $c_syntax_rc -eq 0 ]]; then
    echo "Code looks fine for me!"
    exit 0
fi

read -p "Do you want fix syntax? (y/n): " confirm && [[ $confirm == [yY] || $confirm == [yY][eE][sS] ]] || exit 1

cd $PROJECT_DIR/core-rs && cargo fmt --

cd $PROJECT_DIR

# We use root in container and clang-format rewriting files. We'll need change owner to original
local_user=$(stat -c '%u' .clang-format)
$CLANG_FORMAT_BIN -style=file -i $C_FILES
chown $local_user $C_FILES
