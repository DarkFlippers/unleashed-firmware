<h3 align="center">
<a href="https://github.com/Eng1n33r/flipperzero-firmware">
<img src="https://user-images.githubusercontent.com/10697207/186202043-26947e28-b1cc-459a-8f20-ffcc7fc0c71c.png" align="center" alt="fzCUSTOM" border="0">
</a>
</h3>

Welcome to Flipper Zero's Custom Firmware repo! 
Our goal is to make any features possible in this device without any limitations! 

Please help us implement emulation for all subghz dynamic (rolling code) protocols and static code brute-force app!

<br>

### This software is for experimental purposes only and is not meant for any illegal activity/purposes. <br> We do not condone illegal activity and strongly encourage keeping transmissions to legal/valid uses allowed by law. <br> Also this software is made without any support from Flipper Devices and in no way related to official devs. 

<br>
Our Discord Community:
<br>
<a href="https://discord.gg/58D6E8BtTU"><img src="https://discordapp.com/api/guilds/937479784148115456/widget.png?style=banner4" alt="Unofficial Discord Community"></a>

<br>
<br>
<br>

# What's changed
* SubGHz regional TX restrictions removed
* SubGHz frequecy range can be extended in settings file (Warning: It can damage flipper's hardware)
* Many rolling code protocols now have the ability to save & send captured signals
* FAAC SLH (Spa) & BFT Mitto (secure with seed) manual creation
* Custom community plugins and games added
* Extra SubGHz frequencies + extra Mifare Classic keys
* Picopass/iClass plugin included in releases
* Recompiled IR TV Universal Remote for ALL buttons
* Universal A/C and Audio(soundbars, etc.) remote
* Other small fixes and changes throughout

See changelog in releases for latest updates!

### Current modified and new SubGhz protocols list:
- HCS101
- An-Motors
- CAME Atomo
- FAAC SLH (Spa) [if cloning existing remote - external seed calculation required]
- BFT Mitto [if cloning existing remote - external seed calculation required]
- Keeloq (+ proper manufacturer codes selection) [Not ALL systems supported yet!]
- Nice Flor S
- Security+ v1 & v2
- Star Line

## Support us so we can buy equipment and develop new features
* ETH/BSC/ERC20-Tokens: `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`

**Big thanks to all sponsors!**

### Community apps included:

- ESP8266 Deauther plugin [(by SequoiaSan)](https://github.com/SequoiaSan/FlipperZero-Wifi-ESP8266-Deauther-Module)
- WiFi Scanner plugin [(by SequoiaSan)](https://github.com/SequoiaSan/FlipperZero-WiFi-Scanner_Module)
- MultiConverter plugin [(by theisolinearchip)](https://github.com/theisolinearchip/flipperzero_stuff)
- WAV player plugin (fixed) [(OFW: DrZlo13)](https://github.com/flipperdevices/flipperzero-firmware/tree/zlo/wav-player)
- UPC-A Barcode generator plugin [(by McAzzaMan)](https://github.com/McAzzaMan/flipperzero-firmware/tree/UPC-A_Barcode_Generator/applications/barcode_generator)
- GPIO: Sentry Safe plugin [(by H4ckd4ddy)](https://github.com/H4ckd4ddy/flipperzero-sentry-safe-plugin)
- ESP32: WiFi Marauder companion plugin [(by 0xchocolate)](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion)
- NRF24: Sniffer & MouseJacker (with changes) [(by mothball187)](https://github.com/mothball187/flipperzero-nrf24/tree/main/mousejacker)
- Simple Clock (fixed) !! New version WIP, wait for updates !! [(Original by CompaqDisc)](https://gist.github.com/CompaqDisc/4e329c501bd03c1e801849b81f48ea61)
- UniversalRF Remix (with changes)(only RAW subghz files) [(by ESurge)(Original UniversalRF by jimilinuxguy)](https://github.com/ESurge/flipperzero-firmware-unirfremix)
- Tetris (with fixes) [(by jeffplang)](https://github.com/jeffplang/flipperzero-firmware/tree/tetris_game/applications/tetris_game)
- Spectrum Analyzer (with changes) [(by jolcese)](https://github.com/jolcese/flipperzero-firmware/tree/spectrum/applications/spectrum_analyzer) - [Ultra Narrow mode & scan channels non-consecutively](https://github.com/theY4Kman/flipperzero-firmware/commits?author=theY4Kman)
- Arkanoid (with fixes) [(by gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins)
- Tic Tac Toe (with fixes) [(by gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins)

### Other changes

- BadUSB Keyboard layouts [(by rien > dummy-decoy)](https://github.com/dummy-decoy/flipperzero-firmware/tree/dummy_decoy/bad_usb_keyboard_layout)
- New frequency analyzer - [(by ClusterM)](https://github.com/ClusterM)

# Instructions
## [- How to install firmware](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/HowToInstall.md)

## [- How to build firmware](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/HowToBuild.md)

## [- BadUSB: how to add new keyboard layouts](https://github.com/dummy-decoy/flipperzero_badusb_kl)

## [- How to change Flipper name](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/CustomFlipperName.md)

### **Plugins**

## [- Configure UniversalRF Remix App](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/UniRFRemix.md)

## [- Barcode Generator](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/BarcodeGenerator.md)

## [- Multi Converter](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/MultiConverter.md)

## [- WAV Player sample files & how to convert](https://github.com/UberGuidoZ/Flipper/tree/main/Wav_Player#readme)

### **Plugins that works with external hardware**

## [- How to use: [NRF24] plugins](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/NRF24.md)

## [- How to use: [WiFi] Scanner](https://github.com/SequoiaSan/FlipperZero-WiFi-Scanner_Module#readme)

## [- How to use: [ESP8266] Deauther](https://github.com/SequoiaSan/FlipperZero-Wifi-ESP8266-Deauther-Module#readme)

## [- How to use: [ESP32] WiFi Marauder](https://github.com/UberGuidoZ/Flipper/tree/main/Wifi_DevBoard)

## [- [WiFi] Scanner - Web Flasher for module firmware](https://sequoiasan.github.io/FlipperZero-WiFi-Scanner_Module/)

## [- [ESP8266] Deauther - Web Flasher for module firmware](https://sequoiasan.github.io/FlipperZero-Wifi-ESP8266-Deauther-Module/)

## [- Windows: How to Upload .bin to ESP32/ESP8266](https://github.com/SequoiaSan/Guide-How-To-Upload-bin-to-ESP8266-ESP32)

## [- How to use: [GPIO] SentrySafe plugin](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/SentrySafe.md)

### **SubGHz**

## [- Transmission is blocked? - How to extend SubGHz frequency range](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/DangerousSettings.md)

## [- How to add extra SubGHz frequencies](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/SubGHzSettings.md)

<br>
<br>

# Where I can find IR, SubGhz, ... files, DBs, and other stuff?
## [Awesome Flipper Zero - Github](https://github.com/djsime1/awesome-flipperzero)
## [UberGuidoZ Playground - Large collection of files - Github](https://github.com/UberGuidoZ/Flipper)
## [CAME-12bit, NICE-12bit, Linear-10bit, PT-2240 - SubGHz fixed code bruteforce](https://github.com/tobiabocchi/flipperzero-bruteforce)

<br>
<br>

# Links

* Unofficial Discord: [discord.gg/58D6E8BtTU](https://discord.gg/58D6E8BtTU)
* Docs by atmanos / How to write your own app (outdated API): [https://flipper.atmanos.com/docs/overview/intro](https://flipper.atmanos.com/docs/overview/intro)

* Official Docs: [http://docs.flipperzero.one](http://docs.flipperzero.one)
* Official Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)

# Project structure

- `applications`    - Applications and services used in firmware
- `assets`          - Assets used by applications and services
- `furi`            - Furi Core: os level primitives and helpers
- `debug`           - Debug tool: GDB-plugins, SVD-file and etc
- `documentation`   - Documentation generation system configs and input files
- `firmware`        - Firmware source code
- `lib`             - Our and 3rd party libraries, drivers and etc...
- `site_scons`      - Build helpers
- `scripts`         - Supplementary scripts and python libraries home

Also pay attention to `ReadMe.md` files inside of those directories.
