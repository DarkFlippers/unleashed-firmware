_18 aug 2020 by @glitchcore_

# What is done

## Hardware release: F1B1C0.0

Current Flipper Zero prototype is based on board [Version 0 (F1B1C0.0)](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F1B1C0.0) that have a lot of bugs.

![](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/hw-F1B1C0.0.jpg)

## Firmware

During the early prototyping stages of Flipper Zero, we have used a lot of 3rd-party code, sketches, and dirty demos just as proof of concept, and didn't think too much about architecture. This code splits into many incompatible pieces of code, and some of them don’t even have an UI. This repo is cleaned from all the dirty demos and prepared for contributors, so we will start porting all the legacy code here, following the new architecture.

### Release 0.1

**[Download release](https://github.com/Flipper-Zero/flipperzero-firmware-community/releases/tag/0.1.0)**

You can run firmware locally (with HAL stub).

* `docker-compose exec dev make -C target_lo` for build
* `docker-compose exec dev target_lo/build/target_lo` for run

Read more in [building instructions](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Firmware#building).

# To do

## Flipper developement roadmap

* ~~**Phase 0** (compelete) Preparing for Kickstarter, prototyping UI, checkng hardware, prototype protocol sniffer, tag readers, etc.~~
* **Phase 1.** (Current) Set up developing routines for massive contributors activity, architecture and documentation. Building hardware rig for remote testing.
* **Phase 2.** Start massive contributors program. Preparing Developments Kits for sending to few developers.
* **Phase 3.** Next PCB release [Version 1 (F2B0C1)](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F2B0C1.1) and sending it to more contributos.
* **Phase 4.** Release PCB based on STM32WB55RB and sending it to developers.
* **Phase 5.** Making all repositories publicly open.

## Tasks for phase 1: (Doing right now)

Right now we are working on clean architecture and documentation for contributors.

* Finalize firmware core architecture and document it for contributors onboarding. You can see progress in [Core project](https://github.com/Flipper-Zero/flipperzero-firmware-community/projects/3)
* Set up a test environment and CI. You can see progress in [Environment project](https://github.com/Flipper-Zero/flipperzero-firmware-community/projects/2)
* Create Flipper Zero software emulator with display and buttons [Task #22](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/22)
* Set up integration between wiki and issues, configure wiki generator from doc files: [Task #16](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/16)
* Finish the basic wiki pages: create feature description, UI sketches, links to related project/code, documentation for protocols/
* Make basic code examples [Task #15](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/15)
* Transfer old code to new architecture

## Next hardware release

We have finished the next version of PCB, where a lot of bugs are fixed, and now waiting for its manufacturing. This new board will be used as Developer Kit for early firmware development and will be sent to developers.
