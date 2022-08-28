# Flipper Zero Firmware

[![Discord](https://img.shields.io/discord/740930220399525928.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](http://flipperzero.one/discord)

![Show me the code](https://habrastorage.org/webt/eo/m0/e4/eom0e4btudte7nrhnyic-laiog0.png)

Welcome to [Flipper Zero](https://flipperzero.one/)'s Firmware repo!
Our goal is to create nice and clean code with good documentation, to make it a pleasure for everyone to work with.

# Clone the Repository

You should clone with 
```shell
$ git clone --recursive https://github.com/flipperdevices/flipperzero-firmware.git
```

# Update firmware

[Get Latest Firmware from Update Server](https://update.flipperzero.one/)

Flipper Zero's firmware consists of two components:

- Core2 firmware set - proprietary components by ST: FUS + radio stack. FUS is flashed at factory and you should never update it.
- Core1 Firmware - HAL + OS + Drivers + Applications.

They both must be flashed in the order described.

## With offline update package

With Flipper attached over USB:

`./fbt flash_usb`

Just building the package:

`./fbt updater_package`

To update, copy the resulting directory to Flipper's SD card and navigate to `update.fuf` file in Archive app. 

## With STLink

### Core1 Firmware

Prerequisites:

- Linux / macOS
- Terminal
- [arm-gcc-none-eabi](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
- openocd

One liner: `./fbt firmware_flash`

## With USB DFU 

1. Download latest [Firmware](https://update.flipperzero.one)

2. Reboot Flipper to Bootloader
 - Press and hold `← Left` + `↩ Back` for reset 
 - Release `↩ Back` and keep holding `← Left` until blue LED lights up
 - Release `← Left`

3. Run `dfu-util -D full.dfu -a 0`

# Build on Linux/macOS

Check out `documentation/fbt.md` for details on building and flashing firmware. 

## macOS Prerequisites

Make sure you have [brew](https://brew.sh) and install all the dependencies:
```sh
brew bundle --verbose
```

## Linux Prerequisites

The FBT tool handles everything, only `git` is required.

### Optional dependencies

- openocd (debugging/flashing over SWD)
- heatshrink (compiling image assets)
- clang-format (code formatting)
- dfu-util (flashing over USB DFU)
- protobuf (compiling proto sources)

For example, to install them on Debian, use:
```sh
apt update
apt install openocd clang-format-13 dfu-util protobuf-compiler
```

heatshrink has to be compiled [from sources](https://github.com/atomicobject/heatshrink).

## Compile everything

```sh
./fbt
```

Check `dist/` for build outputs.

Use **`flipper-z-{target}-full-{suffix}.dfu`** to flash your device.

## Flash everything

Connect your device via ST-Link and run:
```sh
./fbt firmware_flash
```

# Links

* Discord: [flipp.dev/discord](https://flipp.dev/discord)
* Website: [flipperzero.one](https://flipperzero.one)
* Kickstarter page: [kickstarter.com](https://www.kickstarter.com/projects/flipper-devices/flipper-zero-tamagochi-for-hackers)
* Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)

# Project structure

- `applications`    - Applications and services used in firmware
- `assets`          - Assets used by applications and services
- `furi`            - Furi Core: os level primitives and helpers
- `debug`           - Debug tool: GDB-plugins, SVD-file and etc
- `documentation`   - Documentation generation system configs and input files
- `firmware`        - Firmware source code
- `lib`             - Our and 3rd party libraries, drivers and etc...
- `scripts`         - Supplementary scripts and python libraries home

Also pay attention to `ReadMe.md` files inside of those directories.
