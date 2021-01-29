# Flipper Zero Firmware community repo

[![Discord](https://img.shields.io/discord/740930220399525928.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](http://flipperzero.one/discord)

<img src="https://habrastorage.org/webt/eo/m0/e4/eom0e4btudte7nrhnyic-laiog0.png" />

Welcome to [Flipper Zero](https://flipperzero.one/)'s Firmware repo!
Our goal is to create nice and clean code along with good documentation, to make it a pleasure for everyone to work with.
This repo will become completely public closer to the device shipping date.

# Update firmware

<a href="https://update.flipperzero.one/full_firmware_latest.bin"><img width="300" src="https://update.flipperzero.one/latest-firmware-banner.png" /></a>


Flipper Zero's firmware consists of three components:

- Core2 firmware - proprietary componenets by ST: FUS + radio stack.
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

1. Download latest [Firmware](https://update.flipperzero.one/full_firmware_latest.bin)

2. Reboot Flipper to Bootloader 
 - Press and hold `← Left` + `↩ Back` for reset 
 - Release `← Left` and keep holding `↩ Back` until blue LED lights up
 - Release `↩ Back`
<img src="https://habrastorage.org/webt/uu/c3/g2/uuc3g2n36f2sju19rskcvjzjf6w.png" />

1. Run `dfu-util -D full_firmware_latest.bin -a 0 -s 0x08000000:leave`

# Build from source

`docker-compose exec dev make -C firmware TARGET=f4 APP_RELEASE=1 flash` for build and flash dev board (see `applications/applications.mk` for list of applications/examples)

# Links
* Task tracker: [Jira](https://flipperzero.atlassian.net/)
* Discord server: [flipperzero.one/discord](https://flipperzero.one/discord)
* Project website: [flipperzero.one](https://flipperzero.one)
* Kickstarter page: [kickstarter.com](https://www.kickstarter.com/projects/flipper-devices/flipper-zero-tamagochi-for-hackers)
* Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)
