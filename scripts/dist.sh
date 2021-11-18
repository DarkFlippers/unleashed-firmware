#!/usr/bin/env bash

set -e

suffix="${DIST_SUFFIX:=local}"

rm -rf "dist/${TARGET}"
mkdir -p "dist/${TARGET}"

# copy build outputs
cp bootloader/.obj/${TARGET}/bootloader.elf \
    dist/${TARGET}/flipper-z-${TARGET}-bootloader-${suffix}.elf
cp bootloader/.obj/${TARGET}/bootloader.bin \
    dist/${TARGET}/flipper-z-${TARGET}-bootloader-${suffix}.bin
cp bootloader/.obj/${TARGET}/bootloader.dfu \
    dist/${TARGET}/flipper-z-${TARGET}-bootloader-${suffix}.dfu
cp bootloader/.obj/${TARGET}/bootloader.json \
    dist/${TARGET}/flipper-z-${TARGET}-bootloader-${suffix}.json
cp firmware/.obj/${TARGET}/firmware.elf \
    dist/${TARGET}/flipper-z-${TARGET}-firmware-${suffix}.elf
cp firmware/.obj/${TARGET}/firmware.bin \
    dist/${TARGET}/flipper-z-${TARGET}-firmware-${suffix}.bin
cp firmware/.obj/${TARGET}/firmware.dfu \
    dist/${TARGET}/flipper-z-${TARGET}-firmware-${suffix}.dfu
cp firmware/.obj/${TARGET}/firmware.json \
    dist/${TARGET}/flipper-z-${TARGET}-firmware-${suffix}.json

# generate full.bin
cp dist/${TARGET}/flipper-z-${TARGET}-bootloader-${suffix}.bin \
    dist/${TARGET}/flipper-z-${TARGET}-full-${suffix}.bin
dd if=/dev/null of=dist/${TARGET}/flipper-z-${TARGET}-full-${suffix}.bin bs=1 count=0 seek=32768 2> /dev/null
cat dist/${TARGET}/flipper-z-${TARGET}-firmware-${suffix}.bin \
    >>dist/${TARGET}/flipper-z-${TARGET}-full-${suffix}.bin \
    2> /dev/null

# generate full.dfu
./scripts/bin2dfu.py \
    -i dist/${TARGET}/flipper-z-${TARGET}-full-${suffix}.bin \
    -o dist/${TARGET}/flipper-z-${TARGET}-full-${suffix}.dfu \
    -a 0x08000000 \
    -l "Flipper Zero $(echo ${TARGET} | tr a-z A-Z)"

# generate full.json
./scripts/meta.py merge \
    -i dist/${TARGET}/flipper-z-${TARGET}-bootloader-${suffix}.json \
    dist/${TARGET}/flipper-z-${TARGET}-firmware-${suffix}.json \
    >dist/${TARGET}/flipper-z-${TARGET}-full-${suffix}.json

echo "Firmware binaries can be found at:"
echo -e "\t$(pwd)/dist/${TARGET}"
echo "Use this file to flash your Flipper:"
echo -e "\tflipper-z-${TARGET}-full-${suffix}.dfu"
