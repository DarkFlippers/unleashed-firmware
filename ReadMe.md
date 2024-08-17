<h3 align="center">
<a href="https://github.com/DarkFlippers/unleashed-firmware">
<img src="https://user-images.githubusercontent.com/10697207/186202043-26947e28-b1cc-459a-8f20-ffcc7fc0c71c.png" align="center" alt="Unleashed Firmware Logo" border="0">
</a>
</h3>
<div align="center" id="badges">
	<a href="https://discord.unleashedflip.com">
		<img src="https://img.shields.io/discord/937479784148115456?style=flat-square&logo=discord&label=Discord&color=%237289DA&link=https%3A%2F%2Fdiscord.unleashedflip.com%2F" alt="Discord server"/>
	</a>
	<a href="https://t.me/flipperzero_unofficial">
		<img src="https://img.shields.io/endpoint?label=EN%20Channel&style=flat-square&url=https%3A%2F%2Fmogyo.ro%2Fquart-apis%2Ftgmembercount%3Fchat_id%3Dflipperzero_unofficial" alt="EN TG channel"/>
	</a>
	<a href="https://t.me/flipperzero_unofficial_ru">
		<img src="https://img.shields.io/endpoint?label=RU%20Channel&style=flat-square&url=https%3A%2F%2Fmogyo.ro%2Fquart-apis%2Ftgmembercount%3Fchat_id%3Dflipperzero_unofficial_ru" alt="RU TG channel"/>	
	</a>
	<a href="https://t.me/flipperzero_unofficial_ua">
		<img src="https://img.shields.io/endpoint?label=UA%20Channel&style=flat-square&url=https%3A%2F%2Fmogyo.ro%2Fquart-apis%2Ftgmembercount%3Fchat_id%3Dflipperzero_unofficial_ua" alt="UA TG channel"/>
	</a>
</div>

### Welcome to the Flipper Zero Unleashed Firmware repo! 

#### **This firmware is a fork from original (OFW) firmware** [flipperdevices/flipperzero-firmware](https://github.com/flipperdevices/flipperzero-firmware)

<br>

### Most stable custom firmware focused on new features and improvements of original firmware components, keeping compatibility with original firmware API and Apps

<br>

##### This software is for experimental purposes only and is not meant for any illegal activity/purposes. <br> We do not condone illegal activity and strongly encourage keeping transmissions to legal/valid uses allowed by law. <br> Also, this software is made without any support from Flipper Devices and is in no way related to the official team. 


<br>

## FAQ (frequently asked questions)
[Follow this link to find answers to most asked questions](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/FAQ.md)

## Dev builds (unstable) (built automatically from dev branch)
- https://dev.unleashedflip.com/
- https://t.me/kotnehleb
## Releases in Telegram
- https://t.me/unleashed_fw

# What's changed
- **Sub-GHz** *lib & hal*
	- Regional TX restrictions removed
	- Extra Sub-GHz frequencies added
	- Frequency range can be extended in settings file (Warning: It can damage Flipper's hardware)
	- Many rolling code [protocols](https://github.com/DarkFlippers/unleashed-firmware#current-modified-and-new-sub-ghz-protocols-list) now have the ability to save & send captured signals
	- FAAC SLH (Spa) & BFT Mitto (keeloq secure with seed) manual creation
	- External CC1101 module support [(by quen0n)](https://github.com/DarkFlippers/unleashed-firmware/pull/307)
- **Sub-GHz** *Main App*
	- Save last used settings [(by derskythe)](https://github.com/DarkFlippers/unleashed-firmware/pull/77)
	- New frequency analyzer [(by ClusterM)](https://github.com/DarkFlippers/unleashed-firmware/pull/43)
	- Press OK in frequency analyzer to use detected frequency in Read modes [(by derskythe)](https://github.com/DarkFlippers/unleashed-firmware/pull/77)
	- Long press OK button in Sub-GHz Frequency analyzer to switch to Read menu [(by derskythe)](https://github.com/DarkFlippers/unleashed-firmware/pull/79)
	- New option to use timestamps + protocol name when you saving file, instead of random name or timestamp only - Enable in `Radio Settings -> Protocol Names = ON`
	- Read mode UI improvements (shows time when signal was received) (by @wosk)
	- External CC1101 module support (Hardware SPI used)
	- External CC1101 module amplifier control (or LED control) support (enabled by default)
	- **Hold right in received signal list to delete selected signal**
	- **Custom buttons for Keeloq / Alutech AT4N / Nice Flor S / Somfy Telis / Security+ 2.0 / CAME Atomo** - now you can use arrow buttons to send signal with different button code
	-   `Add manually` menu extended with new protocols
	- FAAC SLH, BFT Mitto / Somfy Telis / Nice Flor S / CAME Atomo, etc.. manual creation with programming new remote into receiver (use button 0xF for BFT Mitto, 0x8 (Prog) on Somfy Telis, (right arrow button for other protocols))
	- Debug mode counter increase settings (+1 -> +5, +10, default: +1)
	- Debug PIN output settings for protocol development
	
- **Sub-GHz apps** *by unleashed team*
	- Sub-GHz Bruteforce - static code brute-force plugin | 
		- Time delay (between signals) setting (hold Up in main screen(says Up to Save)) + configure repeats in protocols list by pressing right button on selected protocol
        - Load your own file and select bytes you want to bruteforce or use preconfigured options in protocols list
	- Sub-GHz Remote - remote control for 5 sub-ghz files | bind one file for each button
		- use the built-in constructor or make config file by following this [instruction](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemotePlugin.md)
- **Infrared**
	- Recompiled IR TV Universal Remote for ALL buttons
	- Universal remotes for Projectors, Fans, A/Cs and Audio(soundbars, etc.) -> Also always updated and verified by our team
	- Infrared -> `RCA` Protocol
	- Infrared -> External IR modules support (with autodetect by OFW)
- **NFC/RFID/iButton**
	* LFRFID and iButton Fuzzer plugins
	* Add DEZ 8 display form for EM4100 (by @korden32)
	* Extra Mifare Classic keys in system dict
	* EMV Protocol + Public data parser (by @Leptopt1los and @wosk)
	* NFC `Add manually` -> Mifare Classic with custom UID
	* NFC parsers: Umarsh, Zolotaya Korona, Kazan, Metromoney, Moscow Social Card, Troika (reworked) and [many others](https://github.com/DarkFlippers/unleashed-firmware/tree/dev/applications/main/nfc/plugins/supported_cards) (by @Leptopt1los and @assasinfil)
- **Quality of life & other features**
	- Customizable Flipper name **Update! Now can be changed in Settings->Desktop** (by @xMasterX and @Willy-JL)
	- Text Input UI element -> Cursor feature (by @Willy-JL)
	- Byte Input Mini editor -> **Press UP** multiple times until the nibble editor appears (by @gid9798)
	- Clock on Desktop -> `Settings -> Desktop -> Show Clock` (by @gid9798)
	- Battery percentage display with different styles `Settings -> Desktop -> Battery View`
	- More games in Dummy Mode -> click or hold any of arrow buttons
	- Lock device with pin(or regular lock if pin not set) by holding UP button on main screen [(by an4tur0r)](https://github.com/DarkFlippers/unleashed-firmware/pull/107)
	- **BadKB** plugin [(by Willy-JL, ClaraCrazy, XFW contributors)](https://github.com/Flipper-XFW/Xtreme-Firmware/tree/dev/applications/main/bad_kb) - (See in Applications->Tools) - (aka BadUSB via Bluetooth)
	- BadUSB -> Keyboard layouts [(by rien > dummy-decoy)](https://github.com/dummy-decoy/flipperzero-firmware/tree/dummy_decoy/bad_usb_keyboard_layout)
	- Custom community plugins and games added + all known working apps can be downloaded in extra pack in every release
	- Other small fixes and changes throughout
	- See other changes in readme below

Also check the [changelog in releases](https://github.com/DarkFlippers/unleashed-firmware/releases) for latest updates!

### Current modified and new Sub-GHz protocols list:
Thanks to Official team (to their SubGHz Developer, Skorp) for implementing support (decoder + encoder / or decode only) for these protocols in OFW.

Keeloq [Not ALL systems supported for decode or emulation!] - [Supported manufacturers list](https://pastes.io/raw/unuj9bhe4m)

Encoders or emulation (+ programming mode) support made by @xMasterX:
- Nero Radio 57bit (+ 56bit support)
- CAME 12bit/24bit encoder fixes (Fixes are now merged in OFW)
- Keeloq: Dea Mio, Genius Bravo, GSN, HCS101, AN-Motors, JCM Tech, MHouse, Nice Smilo, DTM Neo, FAAC RC,XT, Mutancode, Normstahl, Beninca + Allmatic, Stilmatic, CAME Space, Aprimatic (model TR and similar), Centurion Nova (thanks Carlos !), Hormann EcoStar, Novoferm

Protocols support made by Skorp (original implementation) and @xMasterX (current version):
- CAME Atomo -> Update! check out new [instructions](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemoteProg.md)
- Nice Flor S -> How to create new remote - [instructions](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemoteProg.md)
- FAAC SLH (Spa) -> Update!!! (Programming mode!) Check out new [instructions](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemoteProg.md)
- Keeloq: BFT Mitto -> Update! Check out new [instructions](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemoteProg.md)
- Star Line
- Security+ v1 & v2

Encoders made by @assasinfil and @xMasterX:
- Somfy Telis -> How to create new remote - [instructions](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemoteProg.md)
- Somfy Keytis
- KingGates Stylo 4k
- Alutech AT-4N -> How to create new remote - [instructions](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemoteProg.md)
- Nice ON2E (Nice One) -> How to create new remote - [instructions](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemoteProg.md)

## Please support development of the project
The majority of this project is developed and maintained by me, @xMasterX.
Our team is small and the guys are working on this project as much as they can solely based on the enthusiasm they have for this project and the community.
- @Leptopt1los - NFC, RFID, Plugins, and many other things
- @gid9798 - SubGHz, Plugins, many other things - currently offline :( 
- @assasinfil - SubGHz protocols, NFC parsers
- @Svaarich - UI design and animations
- @amec0e - Infrared assets
- Community moderators in Telegram, Discord, and Reddit
- And of course our GitHub community. Your PRs are a very important part of this firmware and open-source development.

The amount of work done on this project is huge and we need your support, no matter how large or small. Even if you just say, "Thank you Unleashed firmware developers!" somewhere. Doing so will help us continue our work and will help drive us to make the firmware better every time.
Also, regarding our releases, every build has and always will be free and open-source. There will be no paywall releases or closed-source apps within the firmware. As long as I am working on this project it will never happen.
You can support us by using links or addresses below:
|Service|Remark|QR Code|Link/Wallet|
|-|-|-|-|
|**Patreon**||<div align="center"><a href="https://github.com/user-attachments/assets/a88a90a5-28c3-40b4-864a-0c0b79494a42"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|https://patreon.com/mmxdev|
|**Boosty**|patreon alternative|<div align="center"><a href="https://github.com/user-attachments/assets/893c0760-f738-42c1-acaa-916019a7bdf8"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|https://boosty.to/mmxdev|
|cloudtips|only RU payments accepted|<div align="center"><a href="https://github.com/user-attachments/assets/5de31d6a-ef24-4d30-bd8e-c06af815332a"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|https://pay.cloudtips.ru/p/7b3e9d65|
|YooMoney|only RU payments accepted|<div align="center"><a href="https://github.com/user-attachments/assets/33454f79-074b-4349-b453-f94fdadc3c68"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|https://yoomoney.ru/fundraise/XA49mgQLPA0.221209|
|USDT|(TRC20)|<div align="center"><a href="https://github.com/user-attachments/assets/0500498d-18ed-412d-a1a4-8a66d0b6f057"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|`TSXcitMSnWXUFqiUfEXrTVpVewXy2cYhrs`|
|ETH|(BSC/ERC20-Tokens)|<div align="center"><a href="https://github.com/user-attachments/assets/0f323e98-c524-4f41-abb2-f4f1cec83ab6"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|`darkflippers.eth` (or `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`)|
|BTC||<div align="center"><a href="https://github.com/user-attachments/assets/5a904d45-947e-4b92-9f0f-7fbaaa7b37f8"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|`bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`|
|SOL|(Solana/Tokens)|<div align="center"><a href="https://github.com/user-attachments/assets/ab33c5e0-dd59-497b-9c91-ceb89c36b34d"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|`DSgwouAEgu8iP5yr7EHHDqMNYWZxAqXWsTEeqCAXGLj8`|
|DOGE||<div align="center"><a href="https://github.com/user-attachments/assets/2937edd0-5c85-4465-a444-14d4edb481c0"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|`D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`|
|LTC||<div align="center"><a href="https://github.com/user-attachments/assets/441985fe-f028-4400-83c1-c215760c1e74"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|`ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`|
|BCH||<div align="center"><a href="https://github.com/user-attachments/assets/7f365976-19a3-4777-b17e-4bfba5f69eff"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|`qquxfyzntuqufy2dx0hrfr4sndp0tucvky4sw8qyu3`|
|XMR|(Monero)|<div align="center"><a href="https://github.com/user-attachments/assets/96186c06-61e7-4b4d-b716-6eaf1779bfd8"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|`41xUz92suUu1u5Mu4qkrcs52gtfpu9rnZRdBpCJ244KRHf6xXSvVFevdf2cnjS7RAeYr5hn9MsEfxKoFDRSctFjG5fv1Mhn`|
|TON||<div align="center"><a href="https://github.com/user-attachments/assets/92a57e57-7462-42b7-a342-6f22c6e600c1"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div>|`UQCOqcnYkvzOZUV_9bPE_8oTbOrOF03MnF-VcJyjisTZmsxa`|

## Community apps included

### [🎲 Download Extra plugins for Unleashed](https://github.com/xMasterX/all-the-plugins/releases/latest)
### [List of Extra pack](https://github.com/xMasterX/all-the-plugins/tree/dev#extra-pack) | [List of Base *(Default)* pack](https://github.com/xMasterX/all-the-plugins/tree/dev#default-pack)

See full list and sources here: [xMasterX/all-the-plugins](https://github.com/xMasterX/all-the-plugins/tree/dev)

### Official Flipper Zero Apps Catalog [web version](https://lab.flipper.net/apps) or mobile app

# Instructions
## First look at official docs [docs.flipper.net](https://docs.flipper.net/)
## [How to install](/documentation/HowToInstall.md) - [versions info](/CHANGELOG.md#recommended-update-option---web-updater): `r`,` `,`e`...
## Firmware & Development

### - **Developer Documentation** - [developer.flipper.net](https://developer.flipper.net/flipperzero/doxygen)

### - **[How to build](/documentation/HowToBuild.md#how-to-build-by-yourself) | [Project-structure](#project-structure)**

### - **CLion IDE** - How to setup workspace for flipper firmware development [by Savely Krasovsky](https://krasovs.ky/2022/11/01/flipper-zero-clion.html)

### - **"Hello world!"** - plugin tutorial [English<sub> by DroomOne</sub> ](https://github.com/DroomOne/Flipper-Plugin-Tutorial) | [Russian<sub> by Pavel Yakovlev</sub>](https://yakovlev.me/hello-flipper-zero/)

### - [How to write your own app](https://flipper.atmanos.com/docs/overview/intro). Docs by atmanos **⚠️outdated API**

## Firmware & main Apps feature

### - System: [How to change Flipper name](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/CustomFlipperName.md)

### - BadUSB: [How to add new keyboard layouts](https://github.com/dummy-decoy/flipperzero_badusb_kl)

### - Infrared: [How to make captures to add them into Universal IR remotes](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/InfraredCaptures.md)

## **Sub-GHz**

### - [How to use Flipper as new remote (Nice FlorS, BFT Mitto, Somfy Telis, Aprimatic, AN-Motors, etc..)](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemoteProg.md)

### - External Radio: [How to connect CC1101 module](https://github.com/quen0n/flipperzero-ext-cc1101)

### - Transmission is blocked? [How to extend Sub-GHz frequency range](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/DangerousSettings.md)

### - [How to add extra Sub-GHz frequencies](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzSettings.md)

### - [~~Configure Sub-GHz Remote App~~](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemotePlugin.md) Not recommended, please use embedded configurator

## **Plugins**

### - TOTP (Authenticator): [config description](https://github.com/akopachov/flipper-zero_authenticator/blob/master/docs/conf-file_description.md)

### - Barcode Generator: [How to use](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/BarcodeGenerator.md)

### - Multi Converter: [How to use](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/MultiConverter.md)

### - WAV Player: [sample files & how to convert](https://github.com/UberGuidoZ/Flipper/tree/main/Wav_Player#readme)

### - Sub-GHz playlist: [generator script](https://github.com/darmiel/flipper-scripts/blob/main/playlist/playlist_creator_by_chunk.py)

## **Plugins that works with external hardware** [GPIO]

### - Unitemp - Temperature sensors reader: [How to use & supported sensors](https://github.com/quen0n/unitemp-flipperzero#readme)

### - [NMEA] GPS: [How to use](https://github.com/xMasterX/all-the-plugins/blob/dev/base_pack/gps_nmea_uart/README.md)

### - i2c Tools [How to use](https://github.com/xMasterX/all-the-plugins/blob/dev/base_pack/flipper_i2ctools/README.md)

### - [NRF24] plugins: [How to use](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/NRF24.md)


### - [WiFi] Scanner: [How to use](https://github.com/SequoiaSan/FlipperZero-WiFi-Scanner_Module#readme) | [Web Flasher](https://sequoiasan.github.io/FlipperZero-WiFi-Scanner_Module/)

### - [ESP8266] Deauther: [How to use](https://github.com/SequoiaSan/FlipperZero-Wifi-ESP8266-Deauther-Module#readme) | [Web Flasher](https://sequoiasan.github.io/FlipperZero-Wifi-ESP8266-Deauther-Module/)

### - [ESP32] WiFi Marauder: [How to use](https://github.com/UberGuidoZ/Flipper/tree/main/Wifi_DevBoard)<sub> docs by UberGuidoZ</sub> | [Marauder repo](https://github.com/justcallmekoko/ESP32Marauder)

### - [ESP32-CAM] Camera Suite: [How to use](https://github.com/CodyTolene/Flipper-Zero-Camera-Suite)

### - How to Upload `.bin` to ESP32/ESP8266: [Windows](https://github.com/SequoiaSan/Guide-How-To-Upload-bin-to-ESP8266-ESP32) | [FAP "ESP flasher"](https://github.com/0xchocolate/flipperzero-esp-flasher)

### - [GPIO] SentrySafe plugin: [How to use](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SentrySafe.md)

<br>
<br>

# Where I can find IR, Sub-GHz, ... files, DBs, and other stuff?
## [UberGuidoZ Playground - Large collection of files - Github](https://github.com/UberGuidoZ/Flipper)
## [Awesome Flipper Zero - Github](https://github.com/djsime1/awesome-flipperzero)

<br>
<br>

# Links

* Official Docs: [docs.flipper.net](https://docs.flipper.net/)
* Official Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)

* Update! Developer Documentation [developer.flipper.net](https://developer.flipper.net/flipperzero/doxygen)

# Project structure

- `applications`    - Applications and services used in firmware
- `assets`          - Assets used by applications and services
- `furi`            - Furi Core: OS-level primitives and helpers
- `debug`           - Debug tool: GDB-plugins, SVD-file and etc
- `documentation`   - Documentation generation system configs and input files
- `firmware`        - Firmware source code
- `lib`             - Our and 3rd party libraries, drivers and etc...
- `site_scons`      - Build helpers
- `scripts`         - Supplementary scripts and python libraries home

Also, pay attention to the `ReadMe.md` files inside those directories.
