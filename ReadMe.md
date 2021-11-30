# Flipper Zero Firmware

[![Discord](https://img.shields.io/discord/740930220399525928.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](http://flipperzero.one/discord)

![Show me the code](https://habrastorage.org/webt/eo/m0/e4/eom0e4btudte7nrhnyic-laiog0.png)

Welcome to [Flipper Zero](https://flipperzero.one/)'s Firmware repo!
Our goal is to create nice and clean code with good documentation, to make it a pleasure for everyone to work with.

# Update firmware

[Get Latest Firmware from Update Server](https://update.flipperzero.one/)


Flipper Zero's firmware consists of three components:

- Core2 firmware set - proprietary components by ST: FUS + radio stack.
- Core1 Bootloader - controls basic hardware initialization and loads firmware
- Core1 Firmware - HAL + OS + Drivers + Applications

All 3 of them must be flashed in order described.

## With STLink

### Core2 flashing procedures

Prerequisites:

- Linux / macOS
- Terminal
- STM32_Programmer_CLI added to $PATH

One liner: `./flash_core2_ble.sh`

### Core1 Bootloader + Firmware

Prerequisites:

- Linux / macOS
- Terminal
- [arm-gcc-none-eabi](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
- openocd

One liner: `./flash_core1_main.sh`

## With USB DFU 

1. Download latest [Firmware](https://update.flipperzero.one)

2. Reboot Flipper to Bootloader 
 - Press and hold `← Left` + `↩ Back` for reset 
 - Release `↩ Back` and keep holding `← Left` until blue LED lights up
 - Release `← Left`
<!-- ![Switch to DFU sequence](https://habrastorage.org/webt/uu/c3/g2/uuc3g2n36f2sju19rskcvjzjf6w.png) -->

3. Run `dfu-util -D full.dfu -a 0`

# Build with Docker

## Prerequisites

1. Install [Docker Engine and Docker Compose](https://www.docker.com/get-started)
2. Prepare the container:
   ```sh
   docker-compose up -d
   ```

## Compile everything

```sh
docker-compose exec dev make
```

Check `dist/` for build outputs.

Use **`flipper-z-{target}-full-{suffix}.dfu`** to flash your device.

# Build on Linux/macOS

## macOS Prerequisites

Make sure you have [brew](https://brew.sh) and install all the dependencies:
```sh
brew bundle --verbose
```

## Linux Prerequisites

### gcc-arm-none-eabi

```sh
toolchain="gcc-arm-none-eabi-10.3-2021.10"
toolchain_package="$toolchain-$(uname -m)-linux"

wget -P /opt "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/$toolchain_package.tar.bz2"

tar xjf /opt/$toolchain_package.tar.bz2 -C /opt
rm /opt/$toolchain_package.tar.bz2

for file in /opt/$toolchain/bin/* ; do ln -s "${file}" "/usr/bin/$(basename ${file})" ; done
```

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
make
```

Check `dist/` for build outputs.

Use **`flipper-z-{target}-full-{suffix}.dfu`** to flash your device.

## Flash everything

Connect your device via ST-Link and run:
```sh
make whole
```

# Links
* Discord: [flipp.dev/discord](https://flipp.dev/discord)
* Website: [flipperzero.one](https://flipperzero.one)
* Kickstarter page: [kickstarter.com](https://www.kickstarter.com/projects/flipper-devices/flipper-zero-tamagochi-for-hackers)
* Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)

# Folders structure

- applications - application and services
  * accessor - Wiegand server
  * archive - Archive and file manager 
  * bt - BLE service and application
  * cli - Console service
  * debug_tools - different tools that we use on factory and for debug
  * dialogs - service for showing GUI dialogs
  * dolphin - dolphin service and supplementary apps
  * gpio-tester - GPIO control application
  * gui - GUI service
  * ibutton - ibutton application, onewire keys and more
  * input - input service
  * irda - irda application, controls your IR devices 
  * irda_monitor - irda debug tool 
  * lfrfid - LF RFID application
  * lfrfid-debug - LF RFID debug tool
  * loader - application loader service
  * menu - main menu service
  * music-player - music player app (demo)
  * nfc - NFC application, HF rfid, EMV and etc
  * notification - notification service 
  * power - power service
  * power-observer - power debug tool
  * scened-app-example - c++ application example 
  * storage - storage service, internal + sdcard
  * storage_settings - storage settings app
  * subghz - subghz application, 433 fobs and etc
  * tests - unit tests and etc
- assets - assets used by applications and services
  * compiled - compilation results
  * icons - source icons images
- bootloader - bootloader for flipper
  * src - bootloader sources
  * targets - targets' hal and implementation
- core - core libraries: home for furi
- debug - debug helpers, plugins and tools
- docker - docker image sources (used for automated firmware build)
- documentation - documentation generation system configs and input files
- firmware - firmware for flipper
  * targets - targets' hal and implementation
- lib - different libraries and drivers that apps and firmware uses
  * ST25RFAL002 - ST253916 driver and NFC hal
  * STM32CubeWB - STM32WB hal
  * app-scened-template - scened template app library
  * app-template - template app library
  * callback-connector - callback connector library
  * common-api - common api declaration library
  * cyfral - cyfral library
  * drivers - drivers that we wrote
  * fatfs - external storage file system
  * fnv1a-hash - fnv1a hash library 
  * irda - irda library
  * littlefs - internal storage file system
  * mlib - algorithms and containers 
  * nfc_protocols - nfc protocols library
  * onewire - one wire library 
  * qrcode - qr code generator library
  * subghz - subghz library
  * toolbox - toolbox of things that we are using but don't place in core
  * u8g2 - graphics library that we use to draw GUI
- make - make helpers
- scripts - supplementary scripts
