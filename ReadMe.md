<h3 align="center">
<a href="https://github.com/Eng1n33r/flipperzero-firmware">
<img src="https://user-images.githubusercontent.com/10697207/186202043-26947e28-b1cc-459a-8f20-ffcc7fc0c71c.png" align="center" alt="fzCUSTOM" border="0">
</a>
</h3>

### Welcome to Flipper Zero Unleashed Firmware repo! 
Our goal is to make any features possible in this device without any limitations! 

Please help us implement emulation for all subghz dynamic (rolling code) protocols!

<br>

### This software is for experimental purposes only and is not meant for any illegal activity/purposes. <br> We do not condone illegal activity and strongly encourage keeping transmissions to legal/valid uses allowed by law. <br> Also this software is made without any support from Flipper Devices and in no way related to official devs. 

<br>
Our Discord Community:
<br>
<a href="https://discord.gg/flipperzero-unofficial"><img src="https://discordapp.com/api/guilds/937479784148115456/widget.png?style=banner4" alt="Unofficial Discord Community"></a>

<br>
<br>
<br>

# What's changed
* SubGHz regional TX restrictions removed
* SubGHz frequecy range can be extended in settings file (Warning: It can damage flipper's hardware)
* Many rolling code protocols now have the ability to save & send captured signals
* FAAC SLH (Spa) & BFT Mitto (secure with seed) manual creation
* Sub-GHz static code brute-force plugin
* LFRFID Fuzzer plugin
* Custom community plugins and games added
* Extra SubGHz frequencies + extra Mifare Classic keys
* Picopass/iClass plugin included in releases
* Recompiled IR TV Universal Remote for ALL buttons
* Universal remote for Projectors, Fans, A/Cs and Audio(soundbars, etc.)
* BadUSB keyboard layouts
* Customizable Flipper name
* SubGHz -> Press OK in frequency analyzer to use detected frequency in Read modes
* Other small fixes and changes throughout
* See other changes in changelog and in readme below

Also check changelog in releases for latest updates!

### Current modified and new SubGHz protocols list:
- HCS101
- An-Motors
- CAME Atomo
- FAAC SLH (Spa) [External seed calculation required]
- BFT Mitto [External seed calculation required]
- Keeloq [Not ALL systems supported yet!]
- Nice Flor S
- Security+ v1 & v2
- Star Line (saving only)

## Support us so we can buy equipment and develop new features
* ETH/BSC/ERC20-Tokens: `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`

### Community apps included:

- RFID Fuzzer plugin [(by Ganapati)](https://github.com/Eng1n33r/flipperzero-firmware/pull/54) with changes by @xMasterX & New protocols by @mvanzanten
- Sub-GHz bruteforce plugin [(by Ganapati & xMasterX)](https://github.com/Eng1n33r/flipperzero-firmware/pull/57) & Refactored by @derskythe
- Sub-GHz playlist plugin [(by darmiel)](https://github.com/Eng1n33r/flipperzero-firmware/pull/62)
- ESP8266 Deauther plugin [(by SequoiaSan)](https://github.com/SequoiaSan/FlipperZero-Wifi-ESP8266-Deauther-Module)
- WiFi Scanner plugin [(by SequoiaSan)](https://github.com/SequoiaSan/FlipperZero-WiFi-Scanner_Module)
- MultiConverter plugin [(by theisolinearchip)](https://github.com/theisolinearchip/flipperzero_stuff)
- WAV player plugin (fixed) [(OFW: DrZlo13)](https://github.com/flipperdevices/flipperzero-firmware/tree/zlo/wav-player)
- UPC-A Barcode generator plugin [(by McAzzaMan)](https://github.com/McAzzaMan/flipperzero-firmware/tree/UPC-A_Barcode_Generator/applications/barcode_generator)
- GPIO: Sentry Safe plugin [(by H4ckd4ddy)](https://github.com/H4ckd4ddy/flipperzero-sentry-safe-plugin)
- ESP32: WiFi Marauder companion plugin [(by 0xchocolate)](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion)
- NRF24: Sniffer & MouseJacker (with changes) [(by mothball187)](https://github.com/mothball187/flipperzero-nrf24/tree/main/mousejacker)
- Simple Clock (timer by GMMan / settings by kowalski7cc) [(Original by CompaqDisc)](https://gist.github.com/CompaqDisc/4e329c501bd03c1e801849b81f48ea61)
- UniversalRF Remix / Sub-GHz Remote [(by ESurge)](https://github.com/ESurge/flipperzero-firmware-unirfremix)[(updated and all protocol support added by darmiel & xMasterX)](https://github.com/darmiel/flipper-playlist/tree/feat/unirf-protocols)
- Spectrum Analyzer (with changes) [(by jolcese)](https://github.com/jolcese/flipperzero-firmware/tree/spectrum/applications/spectrum_analyzer) - [Ultra Narrow mode & scan channels non-consecutively](https://github.com/theY4Kman/flipperzero-firmware/commits?author=theY4Kman)

Games:
- DOOM (fixed) [(By p4nic4ttack)](https://github.com/p4nic4ttack/doom-flipper-zero/)
- Zombiez [(Reworked By DevMilanIan)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/240) [(Original By Dooskington)](https://github.com/Dooskington/flipperzero-zombiez)
- Flappy Bird [(By DroomOne)](https://github.com/DroomOne/flipperzero-firmware/tree/dev/applications/flappy_bird)
- Arkanoid (refactored by xMasterX) [(by gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins)
- Tic Tac Toe (refactored by xMasterX) [(by gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins)
- Tetris (with fixes) [(by jeffplang)](https://github.com/jeffplang/flipperzero-firmware/tree/tetris_game/applications/tetris_game)

### Other changes

- BadUSB -> Keyboard layouts [(by rien > dummy-decoy)](https://github.com/dummy-decoy/flipperzero-firmware/tree/dummy_decoy/bad_usb_keyboard_layout)
- SubGHz -> New frequency analyzer - [(by ClusterM)](https://github.com/Eng1n33r/flipperzero-firmware/pull/43)
- SubGHz -> Detect RAW feature - [(by perspecdev)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/152)
- SubGHz -> Save last used frequency and moduluation [(by derskythe)](https://github.com/Eng1n33r/flipperzero-firmware/pull/77)
- SubGHz -> Press OK in frequency analyzer to use detected frequency in Read modes [(by derskythe)](https://github.com/Eng1n33r/flipperzero-firmware/pull/77)

# Instructions
## [- How to install firmware](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/HowToInstall.md)

## [- How to build firmware](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/HowToBuild.md)

## [- BadUSB: how to add new keyboard layouts](https://github.com/dummy-decoy/flipperzero_badusb_kl)

## [- How to change Flipper name](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/CustomFlipperName.md)

### **Plugins**

## [- 🎲 Download Extra plugins for Unleashed](https://github.com/UberGuidoZ/Flipper/tree/main/Applications/Unleashed)

## [- Configure Sub-GHz Remote App](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/SubGHzRemotePlugin.md)

## [- Barcode Generator](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/BarcodeGenerator.md)

## [- Multi Converter](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/MultiConverter.md)

## [- WAV Player sample files & how to convert](https://github.com/UberGuidoZ/Flipper/tree/main/Wav_Player#readme)

## [- SubGHz playlist generator script](https://github.com/darmiel/flipper-scripts/blob/main/playlist/playlist_creator_by_chunk.py)

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
## [UberGuidoZ Playground - Large collection of files - Github](https://github.com/UberGuidoZ/Flipper)
## [Awesome Flipper Zero - Github](https://github.com/djsime1/awesome-flipperzero)
## [CAME-12bit, NICE-12bit, Linear-10bit, PT-2240 - SubGHz fixed code bruteforce](https://github.com/tobiabocchi/flipperzero-bruteforce)
## [SMC5326, UNILARM - SubGHz fixed code bruteforce](https://github.com/Hong5489/flipperzero-gate-bruteforce)

<br>
<br>

# Links

* Unofficial Discord: [discord.gg/flipperzero-unofficial](https://discord.gg/flipperzero-unofficial)
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

Also pay attention to `ReadMe.md` files inside those directories.
