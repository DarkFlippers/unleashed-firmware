# Flipper Zero Firmware community repo

[![Discord](https://img.shields.io/discord/740930220399525928.svg?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](http://flipperzero.one/discord)

<img src="https://habrastorage.org/webt/eo/m0/e4/eom0e4btudte7nrhnyic-laiog0.png" />

Welcome to [Flipper Zero](https://flipperzero.one/)'s Firmware repo! Our goal is to create nice and clean code along with good documentation, to make it a pleasure for everyone to work with. This repo will become completely public closer to the device shipping date. 

# Update firmware

<a href="https://update.flipperzero.one/full_firmware_latest.bin"><img width="300" src="https://update.flipperzero.one/latest-firmware-banner.png" /></a>


Flipper Zero's firmware consists of two components: Bootloader and main firmware. Bootloader controls firmware update process over USB. You need working bootloader installed before update firmware over USB.

1. Download latest [Firmware](https://update.flipperzero.one/full_firmware_latest.bin)

2. Reboot Flipper to Bootloader 
 - Press and hold `← Left` + `↩ Back` for reset 
 - Release `← Left` and keep holding `↩ Back` until blue LED lights up
 - Release `↩ Back`
<img src="https://habrastorage.org/webt/uu/c3/g2/uuc3g2n36f2sju19rskcvjzjf6w.png" />

1. Run `dfu-util -D firmware.bin -a 0 -s 0x08008000:leave`

## Build from source

You can run firmware locally (with HAL stub):

* `docker-compose exec dev make -C firmware TARGET=local APP_TEST=1 run` for running tests
* `docker-compose exec dev make -C firmware TARGET=local APP_*=1 run` for running examples (see `applications/applications.mk` for list of applications/examples)

Or on your flipper:

`docker-compose exec dev make -C firmware TARGET=f4 APP_*=1 flash` for build and flash dev board (see `applications/applications.mk` for list of applications/examples)

# Links
* Task tracker: [Jira](https://flipperzero.atlassian.net/)
* Discord server: [flipperzero.one/discord](https://flipperzero.one/discord)
* Project website: [flipperzero.one](https://flipperzero.one)
* Kickstarter page: [kickstarter.com](https://www.kickstarter.com/projects/flipper-devices/flipper-zero-tamagochi-for-hackers)
* Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)
