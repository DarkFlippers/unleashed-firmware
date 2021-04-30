#!/bin/bash

set -x -e

rm bootloader/.obj/f*/flash || true
make -C bootloader -j9 flash

rm firmware/.obj/f*/flash || true
make -C firmware -j9 flash
