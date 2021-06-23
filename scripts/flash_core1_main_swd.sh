#!/bin/bash

set -x -e

SCRIPT_DIR="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

rm $PROJECT_DIR/bootloader/.obj/f*/flash || true
make -C $PROJECT_DIR/bootloader -j9 flash

rm $PROJECT_DIR/firmware/.obj/f*/flash || true
make -C $PROJECT_DIR/firmware -j9 flash
