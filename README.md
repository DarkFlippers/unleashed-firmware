# Flipper Zero Firmware community repo

![](https://github.com/Flipper-Zero/wiki/blob/master/images/firmware-wiki-header.gif)

Welcome to [Flipper Zero](https://flipperzero.one/zero)'s Firmware repo! Our goal is to create nice and clean code along with good documentation, to make it a pleasure for everyone to work with. This repo will become completely public closer to the device shipping date. 

# Current state of Flipper developement

## Hardware

У нас есть плата версии [Version 0 (F1B1C0)](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F1B1C0.0) с кучей багов

(тут фотка платы и мб фотки костылей).

## Firmware

В процессе подготовки к кампании мы прототипировали работу с разными протоколами, графический интерфейс. Весь этот код вскоре будет опубликован здесь. Current code cleaned from all dirty demos and prepared for contributors.

Сейчас мы проектируем архитектуру. You can run firmware locally (with HAL stub).

* `docker-compose exec dev make -C target_lo` for build
* `docker-compose exec dev target_lo/build/target_lo` for run

(подробнее о сборке смотрите [building instructions](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Firmware#building)).

# Flipper developement roadmap

У нас такой план:

* **Phase 0** Preparing for Kickstarter, prototyping UI, checkng hardware, prototype protocol sniffer, tag readers, etc. (compelete).
* **Phase 1.** Разобраться с процессами, документацией и архитектурой (on now). Сейчас у нас нет железок для всех контрибьюторов, поэтому важно сделать стенд для удаленного запуска тестов и сделать эмулятор.
* **Phase 2.** Позвать толпу контрибьюторов, отбирать тех, кому высылать комплект железа
* **Phase 3.** Сделать плату [Version 1 (F2B0C1)](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F2B0C1.1) и разослать ее контрибьюторам. Начать отлаживать на железе.
* **Phase 4.** Сделать плату с блютузом на основе STM32WB55RB и разослать ее контрибьюторам.

# Task for phase 1: что нужно делать прямо сейчас

* Finalize firmware core architecture and document it for contributors onboarding. You can see progress in [Core project](https://github.com/Flipper-Zero/flipperzero-firmware-community/projects/3)
* Set up a test environment and CI. You can see progress in [Environment project](https://github.com/Flipper-Zero/flipperzero-firmware-community/projects/2)
* Create Flipper Zero software emulator with display and buttons [Task #22](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/22)
* Set up integration between wiki and issues, configure wiki generator from doc files: [Task #16](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/16)
* Finish the basic wiki pages: create feature description, UI sketches, links to related project/code, documentation for protocols/
* Make basic code examples [Task #15](https://github.com/Flipper-Zero/flipperzero-firmware-community/issues/15)
* Transfer old code to new architecture

**We are open to changes.** You can suggest changes to any part of the code, wiki, guidelines, workflow, automation, etc.

Если вы уже готовы нам помочь — read [contribution guide](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Contributing) about creating issue, editing wiki, improve codebase and configure environment.

# Firmware

**[Firmware page](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Firmware)**

Flipper consists of two main parts:

* Core: OS, HAL, FS, bootloader, FURI
* Applications: features like RFID or Tamagotchi, and also background tasks like button debouncing and control the backlight.

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
