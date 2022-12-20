<h3 align="center">
<a href="https://github.com/DarkFlippers/unleashed-firmware">
<img src="https://user-images.githubusercontent.com/10697207/186202043-26947e28-b1cc-459a-8f20-ffcc7fc0c71c.png" align="center" alt="fzCUSTOM" border="0">
</a>
</h3>

### Welcome to Flipper Zero Unleashed Firmware repo! 

**This firmware is a fork from** [flipperdevices/flipperzero-firmware](https://github.com/flipperdevices/flipperzero-firmware)

<br>

Our goal is to make any features possible in this device without any limitations! 

Please help us implement emulation for all subghz dynamic (rolling code) protocols!

<br>

### This software is for experimental purposes only and is not meant for any illegal activity/purposes. <br> We do not condone illegal activity and strongly encourage keeping transmissions to legal/valid uses allowed by law. <br> Also this software is made without any support from Flipper Devices and in no way related to official devs. 

<br>
Our Discord Community:
<br>
<a href="https://discord.unleashedflip.com"><img src="https://discordapp.com/api/guilds/937479784148115456/widget.png?style=banner4" alt="Unofficial Discord Community" target="_blank"></a>

<br>
<br>
<br>

## Dev builds
- https://dev.unleashedflip.com/
- https://t.me/kotnehleb
## Releases in Telegram
- https://t.me/unleashed_fw

# What's changed
* SubGHz regional TX restrictions removed
* SubGHz frequency range can be extended in settings file (Warning: It can damage flipper's hardware)
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
* SubGHz -> Long press OK button in SubGHz Frequency analyzer to switch to Read menu 
* Other small fixes and changes throughout
* See other changes in changelog and in readme below

Also check changelog in releases for latest updates!

### Current modified and new SubGHz protocols list:
- Keeloq [Not ALL systems supported for decode or emulation yet!] - [Supported manufacturers list](https://0bin.net/paste/VwR2lNJY#WH9vnPgvcp7w6zVKucFCuNREKAcOij8KsJ6vqLfMn3b)
- Keeloq: HCS101
- Keeloq: AN-Motors
- Keeloq: JCM Tech
- Keeloq: MHouse
- Keeloq: Nice Smilo
- Keeloq: DTM Neo
- Keeloq: FAAC RC,XT
- Keeloq: Mutancode
- Keeloq: Normstahl
- CAME Atomo
- Nice Flor S
- FAAC SLH (Spa) [External seed calculation required (For info contact me in Discord: Nano#8998)] 
- BFT Mitto [External seed calculation required (For info contact me in Discord: Nano#8998)] 
- Security+ v1 & v2
- Star Line

## Please support development of the project
* Boosty: https://boosty.to/mmxdev
* Ko-Fi: https://ko-fi.com/masterx
* destream (100 EUR min): https://destream.net/live/MMX/donate
* cloudtips (only RU payments accepted): https://pay.cloudtips.ru/p/7b3e9d65
* YooMoney (only RU payments accepted): https://yoomoney.ru/fundraise/XA49mgQLPA0.221209
* USDT(TRC20): `TSXcitMSnWXUFqiUfEXrTVpVewXy2cYhrs`
* BCH: `qquxfyzntuqufy2dx0hrfr4sndp0tucvky4sw8qyu3`
* ETH/BSC/ERC20-Tokens: `darkflippers.eth` (or `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`)
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`
* XMR (Monero): `41xUz92suUu1u5Mu4qkrcs52gtfpu9rnZRdBpCJ244KRHf6xXSvVFevdf2cnjS7RAeYr5hn9MsEfxKoFDRSctFjG5fv1Mhn`
* TON: `EQCOqcnYkvzOZUV_9bPE_8oTbOrOF03MnF-VcJyjisTZmpGf`

### Community apps included:

- RFID Fuzzer plugin [(by Ganapati & @xMasterX)](https://github.com/DarkFlippers/unleashed-firmware/pull/54) & New protocols by @mvanzanten
- Sub-GHz bruteforce plugin [(by @derskythe & xMasterX)](https://github.com/derskythe/flipperzero-subbrute) [(original by Ganapati & xMasterX)](https://github.com/DarkFlippers/unleashed-firmware/pull/57)
- Sub-GHz playlist plugin [(by darmiel)](https://github.com/DarkFlippers/unleashed-firmware/pull/62)
- ESP8266 Deauther plugin [(by SequoiaSan)](https://github.com/SequoiaSan/FlipperZero-Wifi-ESP8266-Deauther-Module)
- WiFi Scanner plugin [(by SequoiaSan)](https://github.com/SequoiaSan/FlipperZero-WiFi-Scanner_Module)
- MultiConverter plugin [(by theisolinearchip)](https://github.com/theisolinearchip/flipperzero_stuff)
- USB Keyboard plugin [(by huuck)](https://github.com/huuck/FlipperZeroUSBKeyboard)
- WAV player plugin (fixed) [(OFW: DrZlo13)](https://github.com/flipperdevices/flipperzero-firmware/tree/zlo/wav-player)
- Barcode generator plugin [(original by McAzzaMan)](https://github.com/McAzzaMan/flipperzero-firmware/tree/UPC-A_Barcode_Generator/applications/barcode_generator) - [EAN-8 and refactoring](https://github.com/DarkFlippers/unleashed-firmware/pull/154) by @msvsergey
- GPIO: Sentry Safe plugin [(by H4ckd4ddy)](https://github.com/H4ckd4ddy/flipperzero-sentry-safe-plugin)
- ESP32: WiFi Marauder companion plugin [(by 0xchocolate)](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion)
- NRF24: Sniffer & MouseJacker (with changes) [(by mothball187)](https://github.com/mothball187/flipperzero-nrf24/tree/main/mousejacker)
- Simple Clock (timer by GMMan / settings by kowalski7cc) [(original by CompaqDisc)](https://gist.github.com/CompaqDisc/4e329c501bd03c1e801849b81f48ea61)
- UniversalRF Remix / Sub-GHz Remote [(by @darmiel & @xMasterX)](https://github.com/darmiel/flipper-playlist/tree/feat/unirf-protocols) (original by @ESurge)
- Spectrum Analyzer (with changes) [(by jolcese)](https://github.com/jolcese/flipperzero-firmware/tree/spectrum/applications/spectrum_analyzer) - [Ultra Narrow mode & scan channels non-consecutively](https://github.com/theY4Kman/flipperzero-firmware/commits?author=theY4Kman)
- Metronome [(by panki27)](https://github.com/panki27/Metronome)
- DTMF Dolphin [(by litui)](https://github.com/litui/dtmf_dolphin)
- **TOTP (Authenticator)** [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator)
- GPS [(by ezod)](https://github.com/ezod/flipperzero-gps) works with module `NMEA 0183` via UART (13TX, 14RX, GND pins on Flipper)
- i2c Tools [(by NaejEL)](https://github.com/NaejEL/flipperzero-i2ctools) - C0 -> SCL / C1 -> SDA / GND -> GND | 3v3 logic levels only!
- Temperature Sensor Plugin - HTU2xD, SHT2x, SI702x, SI700x, SI701x, AM2320 [(by Mywk)](https://github.com/Mywk/FlipperTemperatureSensor) - [How to Connect](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/applications/plugins/htu21d_temp_sensor/Readme.md)
- HC-SR04 Distance sensor - Ported and modified by @xMasterX [(original by Sanqui)](https://github.com/Sanqui/flipperzero-firmware/tree/hc_sr04) - How to connect -> (5V -> VCC) / (GND -> GND) / (13|TX -> Trig) / (14|RX -> Echo)
- Morse Code [(by wh00hw)](https://github.com/wh00hw/MorseCodeFAP)
- (UniTemp) Temp sensor reader [(by quen0n)](https://github.com/quen0n/unitemp-flipperzero)
- BH1750 - Lightmeter [(by oleksiikutuzov)](https://github.com/oleksiikutuzov/flipperzero-lightmeter)
- iButton Fuzzer [(by xMasterX)](https://github.com/xMasterX/ibutton-fuzzer)
- HEX Viewer [(by QtRoS)](https://github.com/QtRoS/flipper-zero-hex-viewer)
- POCSAG Pager [(by xMasterX & Shmuma)](https://github.com/xMasterX/flipper-pager)

Games:
- DOOM (fixed) [(by p4nic4ttack)](https://github.com/p4nic4ttack/doom-flipper-zero/)
- Zombiez [(Reworked By DevMilanIan)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/240) [(Original By Dooskington)](https://github.com/Dooskington/flipperzero-zombiez)
- Flappy Bird [(by DroomOne)](https://github.com/DroomOne/flipperzero-firmware/tree/dev/applications/flappy_bird)
- Arkanoid (refactored by xMasterX) [(by gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins)
- Tic Tac Toe (refactored by xMasterX) [(by gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins)
- Tetris (with fixes) [(by jeffplang)](https://github.com/jeffplang/flipperzero-firmware/tree/tetris_game/applications/tetris_game)
- Minesweeper [(by panki27)](https://github.com/panki27/minesweeper)
- Heap Defence (aka Stack Attack) - Ported to latest firmware by @xMasterX - [(original by wquinoa & Vedmein)](https://github.com/Vedmein/flipperzero-firmware/tree/hd/svisto-perdelki)
- Game15 [(by x27)](https://github.com/x27/flipperzero-game15)
- Solitaire [(by teeebor)](https://github.com/teeebor/flipper_games)
- BlackJack [(by teeebor)](https://github.com/teeebor/flipper_games)

### Other changes

- BadUSB -> Keyboard layouts [(by rien > dummy-decoy)](https://github.com/dummy-decoy/flipperzero-firmware/tree/dummy_decoy/bad_usb_keyboard_layout)
- SubGHz -> New frequency analyzer - [(by ClusterM)](https://github.com/DarkFlippers/unleashed-firmware/pull/43)
- SubGHz -> Detect RAW feature - [(by perspecdev)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/152)
- SubGHz -> Save last used frequency [(by derskythe)](https://github.com/DarkFlippers/unleashed-firmware/pull/77)
- SubGHz -> Press OK in frequency analyzer to use detected frequency in Read modes [(by derskythe)](https://github.com/DarkFlippers/unleashed-firmware/pull/77)
- SubGHz -> Long press OK button in SubGHz Frequency analyzer to switch to Read menu [(by derskythe)](https://github.com/DarkFlippers/unleashed-firmware/pull/79)
- Lock device with pin(or regular lock if pin not set) by holding UP button on main screen [(by an4tur0r)](https://github.com/DarkFlippers/unleashed-firmware/pull/107)

# Instructions
## [- How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

## [- How to build firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToBuild.md)

## [- BadUSB: how to add new keyboard layouts](https://github.com/dummy-decoy/flipperzero_badusb_kl)

## [- How to change Flipper name](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/CustomFlipperName.md)

### **Plugins**

## [- 🎲 Download Extra plugins for Unleashed](https://github.com/xMasterX/unleashed-extra-pack)

## [- Configure Sub-GHz Remote App](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemotePlugin.md)

## [- TOTP (Authenticator) config description](https://github.com/akopachov/flipper-zero_authenticator/blob/master/docs/conf-file_description.md)

## [- Barcode Generator](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/BarcodeGenerator.md)

## [- Multi Converter](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/MultiConverter.md)

## [- WAV Player sample files & how to convert](https://github.com/UberGuidoZ/Flipper/tree/main/Wav_Player#readme)

## [- SubGHz playlist generator script](https://github.com/darmiel/flipper-scripts/blob/main/playlist/playlist_creator_by_chunk.py)

### **Plugins that works with external hardware**

## [- How to use: Temperature Sensor Plugin - HTU21D / SI7021](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/applications/plugins/htu21d_temp_sensor/Readme.md)

## [- How to use: DHT11/22 Temp. Sensor Monitor](https://github.com/quen0n/FipperZero-DHT-Monitor#readme)

## [- How to use: AM2320/AM2321 Temp. Sensor plugin](https://github.com/xMasterX/AM2320_Flipper_Plugin)

## [- How to use: [NMEA] GPS](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/applications/plugins/gps_nmea_uart/README.md)

## [- How to use: i2c Tools](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/applications/plugins/flipper_i2ctools/README.md)

## [- How to use: [NRF24] plugins](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/NRF24.md)

## [- How to use: [WiFi] Scanner](https://github.com/SequoiaSan/FlipperZero-WiFi-Scanner_Module#readme)

## [- How to use: [ESP8266] Deauther](https://github.com/SequoiaSan/FlipperZero-Wifi-ESP8266-Deauther-Module#readme)

## [- How to use: [ESP32] WiFi Marauder](https://github.com/UberGuidoZ/Flipper/tree/main/Wifi_DevBoard)

## [- [WiFi] Scanner - Web Flasher for module firmware](https://sequoiasan.github.io/FlipperZero-WiFi-Scanner_Module/)

## [- [ESP8266] Deauther - Web Flasher for module firmware](https://sequoiasan.github.io/FlipperZero-Wifi-ESP8266-Deauther-Module/)

## [- Windows: How to Upload .bin to ESP32/ESP8266](https://github.com/SequoiaSan/Guide-How-To-Upload-bin-to-ESP8266-ESP32)

## [- How to use: [GPIO] SentrySafe plugin](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SentrySafe.md)

### **SubGHz**

## [- Transmission is blocked? - How to extend SubGHz frequency range](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/DangerousSettings.md)

## [- How to add extra SubGHz frequencies](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzSettings.md)

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

* Unofficial Discord: [discord.unleashedflip.com](https://discord.unleashedflip.com)
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
