Welcome to the flipperzero-firmware wiki!

Flipper zero is open source multi-tool device for researching and pentesting radio protocols, access control systems, hardware, and more.

# Building

See [Building instructions](Firmware#building)

# How to contribute

To contribute to code or to wiki make the following steps:

1. Read this wiki
2. Read [contribution guide](Contributing)
3. Ensure you read and sign [Flipper CLA](https://cla-assistant.io/glitchcore/flipper-cla-host)
4. Read Flipper [Roadmap](Flipper-roadmap)

# UI

Common UI feature (menu, screens, features) at [UI page](UI)

# Features

* [Basic Features](Basic-features)
* [Sub-1 GHz radio](Sub-1-GHz-radio) (Transceiver Based on CC1101 chip for 315/433/868 MHz)
* [125 kHz RFID](125-kHz-RFID)
* [Infrared](Infrared)
* [iButton contact keys](iButton-contact-keys)
* [USB](USB)
* [Bluetooth](Bluetooth)
* [GPIO/HW Modules](GPIO)
* [NFC](NFC)
* [U2F](U2F)
* [Tamagotchi](Tamagotchi)
* [USB](USB)
* [Plugins](Plugins)

# PlatformIO & Arduino
* Easy IDE deploying 
* Cross platform HID device without drivers
* Ready to use library for LCD, Buttons, CC1101, GPIO, Infrared, 125khz, Bluetooth? NFC?

# Hardware

[Hardware specification](Hardware-specification)

### Releases

All PCB releases named according to the following rules:

* `F` mean fw target. Different F-versions has different firmware.
* `B` mean body. Different B-versions has mechanical incompatibility. If B=0, PCB is not intended for installation in a case.
* `C` mean interConnections. If С=0, device contain single board and no interconnections. Otherwise, different C-versions is incompatible to each other.
* Number afer point mean sequential numbering of release.

List of releases:

* [Version 0 (F1B1C0)](Hardware-version-F1B1C0.0)
* [Version 1 (F2B0C1)](Hardware-version-F2B0C1.1)

### Hardware components

* LCD display [buydisplay.com](https://www.buydisplay.com/1-4-inch-graphic-128x64-lcd-module-serial-spi-st7565-black-on-white)

# Firmware

[Firmware description page](Firmware)

# Links

* Project website: [flipperzero.one](https://flipperzero.one)
* Kickstarter page: [kickstarter.com](https://www.kickstarter.com/projects/flipper-devices/flipper-zero-tamagochi-for-hackers)
* Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)