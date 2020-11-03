# Flipper Zero Firmware community repo

[![Discord](https://img.shields.io/discord/740930220399525928.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](http://flipperzero.one/discord)

![](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/firmware-wiki-header.gif)

Welcome to [Flipper Zero](https://flipperzero.one/zero)'s Firmware repo! Our goal is to create nice and clean code along with good documentation, to make it a pleasure for everyone to work with. This repo will become completely public closer to the device shipping date. 

**We are open for changes!** You can suggest changes for any part of the code, workflow, automation, etc.

If you are deary to start, please read [contribution guide](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Contributing) about creating issue, editing wiki, improving codebase and configuring environment.

# Firmware

**[Firmware page](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Firmware)**

## Update firmware

Flipper Zero's firmware consists of two components: Bootloader and main firmware. Bootloader controls firmware update process over USB. You need working bootloader installed before update firmware over USB.

1. Download latest [Firmware](https://update.flipperzero.one/full_firmware_latest.bin) 

2. Reboot Flipper to [Firmware update mode](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/UI#reboot-to-bootloader-firmware-update-mode)

3. Run `dfu-util -D firmware.bin -a 0 -s 0x08008000:leave`

## Build from source

You can run firmware locally (with HAL stub):

* `docker-compose exec dev make -C firmware TARGET=local APP_TEST=1 run` for running tests
* `docker-compose exec dev make -C firmware TARGET=local APP_*=1 run` for running examples (see `applications/applications.mk` for list of applications/examples)

Or you can use your dev. board:

`docker-compose exec dev make -C firmware TARGET=f2 APP_*=1 flash` for build and flash dev board (see `applications/applications.mk` for list of applications/examples)

# Links

* Discord server: [flipperzero.one/discord](https://flipperzero.one/discord)
* Project website: [flipperzero.one](https://flipperzero.one)
* Kickstarter page: [kickstarter.com](https://www.kickstarter.com/projects/flipper-devices/flipper-zero-tamagochi-for-hackers)
* Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)
