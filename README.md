# Flipper Zero Firmware community repo

![](https://github.com/Flipper-Zero/wiki/blob/master/images/firmware-wiki-header.gif)  
This repo contains [Flipper Zero](https://flipperzero.one/zero)'s Firmware core, cleaned from all dirty demos, and prepared for contributors. Our goal is to create nice and clean code along with good documentation, to make it a pleasure for everyone to work with.  This repo will become completely public closer to the device shipping date. 

# Project structure
```
├── app                     # App
├── target_f1               # target f1
├── target_lo               # ??
├── wiki                    # Documentation generates from this files
└── wiki-deploy.sh          # Script to generate Wiki from local .md files
```

# Building

See [Building instructions](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Firmware#building)

# How to contribute

To contribute to code or to wiki make the following steps:

1. Read this wiki
2. Read [contribution guide](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Contributing)
3. Ensure you read and sign [Flipper CLA](https://cla-assistant.io/glitchcore/flipper-cla-host)
4. Read Flipper [Roadmap](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Flipper-roadmap)

# UI

Common UI feature (menu, screens, features) at [UI page](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/UI)

# Features

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

# PlatformIO & Arduino
* Easy IDE deploying 
* Cross platform HID device without drivers
* Ready to use library for LCD, Buttons, CC1101, GPIO, Infrared, 125khz, Bluetooth? NFC?

# Hardware

[Hardware specification](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-specification)

### Releases

All PCB releases named according to the following rules:

* `F` mean fw target. Different F-versions has different firmware.
* `B` mean body. Different B-versions has mechanical incompatibility. If B=0, PCB is not intended for installation in a case.
* `C` mean interConnections. If С=0, device contain single board and no interconnections. Otherwise, different C-versions is incompatible to each other.
* Number afer point mean sequential numbering of release.

List of releases:

* [Version 0 (F1B1C0)](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F1B1C0.0)
* [Version 1 (F2B0C1)](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Hardware-version-F2B0C1.1)

### Hardware components

* LCD display [buydisplay.com](https://www.buydisplay.com/1-4-inch-graphic-128x64-lcd-module-serial-spi-st7565-black-on-white)

# Firmware

[Firmware description page](https://github.com/Flipper-Zero/flipperzero-firmware-community/wiki/Firmware)

# Links

* Project website: [flipperzero.one](https://flipperzero.one)
* Kickstarter page: [kickstarter.com](https://www.kickstarter.com/projects/flipper-devices/flipper-zero-tamagochi-for-hackers)
* Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)
