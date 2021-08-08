# Flipper Zero Firmware community repo

[![Discord](https://img.shields.io/discord/740930220399525928.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](http://flipperzero.one/discord)

<img src="https://habrastorage.org/webt/eo/m0/e4/eom0e4btudte7nrhnyic-laiog0.png" />

Welcome to [Flipper Zero](https://flipperzero.one/)'s Firmware repo!
Our goal is to create nice and clean code along with good documentation, to make it a pleasure for everyone to work with.
This repo will become completely public closer to the device shipping date.

# Update firmware

<a href="https://update.flipperzero.one/">Get Latest Firmware from Update Server</a>


Flipper Zero's firmware consists of three components:

- Core2 firmware set - proprietary componenets by ST: FUS + radio stack.
- Core1 Bootloader - controls basic hardware initialization and loads firmware
- Core1 Firmware - HAL + OS + Drivers + Applications

All 3 of them must be flashed in order described.

## With STLink

### Core2 flashing procedures

Prerequisites:

- Linux / MacOs
- Terminal
- STM32_Programmer_CLI added to $PATH

One liner: `./flash_core2_ble.sh`

### Core1 Bootloader + Firmware

Prerequisites:

- Linux / MacOs
- Terminal
- Arm gcc noneabi
- openocd

One liner: `./flash_core1_main.sh`

## With USB DFU 

1. Download latest [Firmware](https://update.flipperzero.one)

2. Reboot Flipper to Bootloader 
 - Press and hold `← Left` + `↩ Back` for reset 
 - Release `← Left` and keep holding `↩ Back` until blue LED lights up
 - Release `↩ Back`
<img src="https://habrastorage.org/webt/uu/c3/g2/uuc3g2n36f2sju19rskcvjzjf6w.png" />

1. Run `dfu-util -D full.dfu -a 0`

# Build from source

`docker-compose exec dev make -C firmware flash` for build and flash dev board (see `applications/applications.mk` for list of applications/examples)

# Links
* Task tracker: [Jira](https://flipperzero.atlassian.net/)
* Discord server: [flipperzero.one/discord](https://flipperzero.one/discord)
* Project website: [flipperzero.one](https://flipperzero.one)
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
 * dolphin - dolphin service and supplientary apps
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
 * storage-settings - storage settings app
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
- firmware - firmware for flipper
 * targets - targets' hal and implementation
- lib - different libraries and drivers that apps and firmware uses
 * ST25RFAL002 - ST253916 driver and NFC hal
 * STM32CubeWB - STM32WB hal
 * app-scened-template - scened template app library
 * app-template - template app library
 * callback-connector - callback connector library
 * common-api - common api delaration library
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
- scripts - supplimentary scripts
