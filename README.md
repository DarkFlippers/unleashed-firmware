# Flipper Zero Firmware community repo

![](https://github.com/Flipper-Zero/wiki/blob/master/images/firmware-wiki-header.gif)

Welcome to [Flipper Zero](https://flipperzero.one/zero)'s Firmware repo! Our goal is to create nice and clean code along with good documentation, to make it a pleasure for everyone to work with. This repo will become completely public closer to the device shipping date. 

# Current state of Flipper developement

## Hardware

Current Flipper Zero prototype is based on board [Version 0 (F1B1C0.0)](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F1B1C0.0) that have a lots of bugs. We have finished the next version of PCB, where these bugs are fixed, and now waiting for it its manufacturing. This new board will be used as Developer Kit for early firmware development and will be sent to developers.

## Firmware

During the early prototyping stages of Flipper Zero, we have used a lot of 3rd-party code, sketches, and dirty demos just as proof of concept, and didn't think too much about architecture. This code splits into many incompatible pieces of code, and some of them don’t even have an UI. This repo is cleaned from all the dirty demos and prepared for contributors, so we will start porting all the legacy code here, following the new architecture.

Right now we are working on clean architecture and documentation for contributors. You can run firmware locally (with HAL stub).

* `docker-compose exec dev make -C target_lo` for build
* `docker-compose exec dev target_lo/build/target_lo` for run

Read more in [building instructions](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Firmware#building).

# Flipper developement roadmap

* ~~**Phase 0** Preparing for Kickstarter, prototyping UI, checkng hardware, prototype protocol sniffer, tag readers, etc. (compelete).~~
* **Phase 1.** (Current) Set up developing routines for massive contributors activity, architecture and documentation. Building hardware rig for remote testing.
* **Phase 2.** Start massive contributors program. Preparing Developments Kits for sending to few developers.
* **Phase 3.** Next PCB release [Version 1 (F2B0C1)](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F2B0C1.1) and sending it to more contributos.
* **Phase 4.** Release PCB based on STM32WB55RB and sending it to developers.
* **Phase 5.** Making all repositories publicly open.

# Task for phase 1: (Doing right now)

* Finalize firmware core architecture and document it for contributors onboarding. You can see progress in [Core project](https://github.com/Flipper-Zero/flipperzero-firmware-community/projects/3)
* Set up a test environment and CI. You can see progress in [Environment project](https://github.com/Flipper-Zero/flipperzero-firmware-community/projects/2)
* Create Flipper Zero software emulator with display and buttons [Task #22](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/22)
* Set up integration between wiki and issues, configure wiki generator from doc files: [Task #16](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/16)
* Finish the basic wiki pages: create feature description, UI sketches, links to related project/code, documentation for protocols/
* Make basic code examples [Task #15](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/15)
* Transfer old code to new architecture

**We are open for changes!** You can suggest changes to any part of the code, wiki, guidelines, workflow, automation, etc.

If you are deary to start, please read [contribution guide](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Contributing) about creating issue, editing wiki, improving codebase and configuring environment.

# Firmware

**[Firmware page](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Firmware)**

Flipper consists of the two main parts:

* Core: OS, HAL, FS, bootloader, FURI
* Applications: features like RFID or Tamagotchi, and also background tasks like button debouncing and backlight control.

## UI

Common UI feature (menu, screens...) at [UI page](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/UI)

## Features

* [Basic Features](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Basic-features)
* [Sub-1 GHz radio](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Sub-1-GHz-radio) (Transceiver Based on CC1101 chip for 315/433/868 MHz)
* [125 kHz RFID](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/125-kHz-RFID)
* [Infrared](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Infrared)
* [iButton contact keys](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/iButton-contact-keys)
* [USB](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/USB)
* [Bluetooth](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Bluetooth)
* [GPIO/HW Modules](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/GPIO)
* [NFC](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/NFC)
* [U2F](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/U2F)
* [Tamagotchi](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Tamagotchi)
* [USB](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/USB)
* [Plugins](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Plugins)

# Hardware

**[Hardware page](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware)**

# Links

* Project website: [flipperzero.one](https://flipperzero.one)
* Kickstarter page: [kickstarter.com](https://www.kickstarter.com/projects/flipper-devices/flipper-zero-tamagochi-for-hackers)
* Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)
