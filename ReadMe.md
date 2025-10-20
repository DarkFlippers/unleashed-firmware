<h3 align="center">
    <a href="https://github.com/DarkFlippers/unleashed-firmware">
        <img src="https://github.com/user-attachments/assets/466c40d5-f6a1-444d-a235-d9026f7cd0ff" align="center" alt="Unleashed Firmware Logo" border="0">  
    </a>
</h3>

[![English Telegram Chat](https://img.shields.io/endpoint?color=neon&style=flat&url=https%3A%2F%2Ftg.sumanjay.workers.dev%2Fflipperzero_unofficial)](https://t.me/flipperzero_unofficial)
[![Russian Telegram Chat](https://img.shields.io/endpoint?color=neon&style=flat&url=https%3A%2F%2Ftg.sumanjay.workers.dev%2Fflipperzero_unofficial_ru)](https://t.me/flipperzero_unofficial_ru)
[![Ukraine Telegram Chat](https://img.shields.io/endpoint?color=neon&style=flat&url=https%3A%2F%2Ftg.sumanjay.workers.dev%2Fflipperzero_unofficial_ua)](https://t.me/flipperzero_unofficial_ua)
[![Discord Server](https://img.shields.io/discord/937479784148115456?style=flat&logo=discord&label=Discord&color=%237289DA&link=https%3A%2F%2Fdiscord.unleashedflip.com%2F)](https://discord.unleashedflip.com)

# Flipper Zero Unleashed Firmware
This firmware is a fork of the original (OFW) version of [flipperdevices/flipperzero-firmware](https://github.com/flipperdevices/flipperzero-firmware) and represents the **most stable** custom build, incorporating **new features** and **improvements** to the original components while remaining **fully compatible** with the API and applications of the original firmware.

> [!WARNING]
> This software is intended solely for experimental purposes and is not meant for any illegal activities.
> We do not condone unlawful behavior and strongly encourage you to use it only within the bounds of the law.
>
> This project is developed independently and is not affiliated with Flipper Devices.
>
> Also be aware, DarkFlippers/unleashed-firmware is the only official page of the project, there is no paid, premium or closed source versions and if someone contacts you and say they are from our team and try to offer something like that - they are scammers, block that users ASAP

<br/>

## 🚀 Usage

Before getting started:
- **Review the Official Documentation:** [docs.flipper.net](https://docs.flipper.net)

- **Installation Guide & Version Info:**  
  How to install the firmware by following the [Installation Guide](/documentation/HowToInstall.md) and check the [version information](/CHANGELOG.md#recommended-update-option---web-updater) (`e`, ` `, `c`)

- **FAQ:**  
  Find answers to common questions in the [FAQ](/documentation/FAQ.md)


## 📦 Releases

### Release builds (stable)
- <img src="https://github.com/badges/shields/assets/10694593/c5c5acc3-f434-4a8d-a834-6d94a7ffb45a" alt="Telegram" title="Telegram" width="14" height="14"> Telegram: <a href="https://t.me/unleashed_fw">t.me/unleashed_fw</a>
- <a href="https://github.com/DarkFlippers/unleashed-firmware/releases"> <img src="https://cdn.simpleicons.org/github/black/white" alt="GitHub" title="" width="14" height="14"> GitHub Releases</a>

### Dev builds (unstable)
> [!NOTE]
> Built automatically from dev branch

- Web site: [dev.unleashedflip.com](https://dev.unleashedflip.com)
- <img src="https://github.com/badges/shields/assets/10694593/c5c5acc3-f434-4a8d-a834-6d94a7ffb45a" alt="Telegram" title="Telegram" width="14" height="14"> Telegram: <a href="https://t.me/kotnehleb">t.me/kotnehleb</a>


## 🆕 What's New

> <details>
> <summary><strong>Sub‑GHz Library & HAL</strong></summary>
> <br/>
>
> - Many new protocols added
> - Regional TX restrictions removed
> - Extra Sub-GHz frequencies added
> - Frequency range can be extended in settings file _(warning: It can damage Flipper's hardware)_
> - Many rolling code [protocols](#current-modified-and-new-sub-ghz-protocols-list) now have the ability to save & send captured signals  
> - FAAC SLH (Spa) & BFT Mitto (keeloq secure with seed) manual creation
> - External CC1101 module support [(by quen0n)](https://github.com/DarkFlippers/unleashed-firmware/pull/307)
> </details>

> <details>
> <summary><strong>Sub‑GHz Main App</strong></summary>
> <br/>
> 
> - Save last used settings [(by derskythe)](https://github.com/DarkFlippers/unleashed-firmware/pull/77)
> - New frequency analyzer [(by ClusterM)](https://github.com/DarkFlippers/unleashed-firmware/pull/43)
> - Press OK in frequency analyzer to use detected frequency in Read modes [(by derskythe)](https://github.com/DarkFlippers/unleashed-firmware/pull/77)  
> - Long press OK button in Sub-GHz Frequency analyzer to switch to Read menu [(by derskythe)](https://github.com/DarkFlippers/unleashed-firmware/pull/79)  
> - New option to use timestamps + protocol name when you saving file, instead of random name or timestamp only - Enable in `Radio Settings -> Protocol Names = ON`  
> - Read mode UI improvements (shows time when signal was received) (by @wosk)
> - External CC1101 module support (Hardware SPI used)
> - External CC1101 module amplifier control (or LED control) support (enabled by default)
> - **Hold right in received signal list to delete selected signal**
>- **Custom buttons for Keeloq / Alutech AT4N / Nice Flor S / Somfy Telis / Security+ 2.0 / CAME Atomo** - now you can use arrow buttons to send signal with different button code  
> - `Add manually` menu extended with new protocols
> - FAAC SLH, BFT Mitto / Somfy Telis / Nice Flor S / CAME Atomo, etc. manual creation with programming new remote into receiver (use button 0xF for BFT Mitto, 0x8 (Prog) on Somfy Telis, (right arrow button for other protocols))  
> - Debug mode counter increase settings (+1 → +5, +10, default: +1)
> - Debug PIN output settings for protocol development
> - Ignore options - Alarms: Hollarm, GangQi | Cars: Kia, Starline, ScherKhan | Sensors: Magellan, Honeywell Sec, Honeywell WDB (doorbells), Legrand (doorbells), Feron (RGB lights)
> </details>

> <details>
> <summary><strong>Sub‑GHz Apps (by Unleashed Team)</strong></summary>
> <br/>
> 
> - Sub-GHz Bruteforce - static code brute-force plugin
> - Time delay (between signals) setting (hold Up in main screen (says Up to Save)) + configure repeats in protocols list by pressing right button on selected protocol  
> - Load your own file and select bytes you want to bruteforce or use preconfigured options in protocols list  
> - Sub-GHz Remote - remote control for 5 sub-ghz files | bind one file for each button  
> - use the built-in constructor or make config file by following this [instruction](/documentation/SubGHzRemotePlugin.md)  
> </details>

> <details>
> <summary><strong>Infrared (IR)</strong></summary>
> <br/>
> 
> - Recompiled IR TV Universal Remote for ALL buttons
> - Universal remotes for Projectors, Fans, A/Cs and Audio(soundbars, etc.) → Also always updated and verified by our team  
> - Infrared → `RCA` Protocol
> - Infrared → External IR modules support (with autodetect by OFW)
> </details>

> <details>
> <summary><strong>NFC/RFID/iButton</strong></summary>
> <br/>
> 
> - LFRFID and iButton Fuzzer plugins
> - Add DEZ 8 display form for EM4100 (by @korden32)
> - Extra Mifare Classic keys in system dict
> - EMV Protocol + Public data parser (by @Leptopt1los and @wosk)
> - NFC `Add manually` → Mifare Classic with custom UID
> - NFC parsers: Umarsh, Zolotaya Korona, Kazan, Metromoney, Moscow Social Card, Troika (reworked) and [many others](https://github.com/DarkFlippers/unleashed-firmware/tree/dev/applications/main/nfc/plugins/supported_cards) (by @Leptopt1los and @assasinfil)
> </details>

> <details>
> <summary><strong>Quality of Life & Other Features</strong></summary>
> <br/>
> 
> - Customizable Flipper name **Update! Now can be changed in Settings → Desktop** (by @xMasterX and @Willy-JL)
> - Text Input UI element → Cursor feature (by @Willy-JL)
> - Byte Input Mini editor → **Press UP** multiple times until the nibble editor appears (by @gid9798)
> - Clock on Desktop `Settings -> Desktop -> Show Clock` (by @gid9798)
> - Battery percentage display with different styles `Settings -> Desktop -> Battery View`
> - More games in Dummy Mode → click or hold any of arrow buttons
> - Lock device with pin (or regular lock if pin not set) by holding UP button on main screen [(by an4tur0r)](https://github.com/DarkFlippers/unleashed-firmware/pull/107)
> - **BadKB** (BadUSB) [(by Willy-JL, ClaraCrazy, XFW contributors)](https://github.com/Flipper-XFW/Xtreme-Firmware/tree/dev/applications/main/bad_kb) - (Integrated into BadUSB app now!) - (aka BadUSB via Bluetooth)
> - BadUSB → Keyboard layouts [(by rien > dummy-decoy)](https://github.com/dummy-decoy/flipperzero-firmware/tree/dummy_decoy/bad_usb_keyboard_layout)
> - Custom community plugins and games added + all known working apps can be downloaded in extra pack in every release
> - Other small fixes and changes throughout
> - See other changes in readme below
> </details>

Also check the [changelog in releases](https://github.com/DarkFlippers/unleashed-firmware/releases) for latest updates!

### Current modified and new Sub-GHz protocols list:
Thanks to Official team (to their SubGHz Developer, Skorp) for implementing support (decoder + encoder / or decode only) for these protocols in OFW.  

> [!NOTE]
> Not all Keeloq systems are supported for decoding or emulation!
> <details>
> <summary><strong>Supported Keeloq manufacturers include</strong></summary>
> <br/>
>
> | Column 1          | Column 2     | Column 3         | Column 4          | Column 5               |
> |-------------------|--------------|------------------|-------------------|------------------------|
> | Alligator         | Comunello    | GSN              | Magic_4           | SL_A2-A4               |
> | Alligator_S-275   | Dea_Mio      | Guard_RF-311A    | Mongoose          | SL_A6-A9/Tomahawk_9010 |
> | APS-1100_APS-2550 | DTM_Neo      | Harpoon          | Mutanco_Mutancode | SL_B6,B9_dop           |
> | Aprimatic         | DoorHan      | IronLogic        | NICE_MHOUSE       | Sommer(fsk476)         |
> | Beninca           | EcoStar      | JCM_Tech         | NICE_Smilo        | Stilmatic              |
> | BFT               | Elmes_Poland | KEY              | Normstahl         | Teco                   |
> | Came_Space        | FAAC_RC,XT   | Kingates_Stylo4k | Pantera           | Tomahawk_TZ-9030       |
> | Cenmax            | FAAC_SLH     | KGB/Subaru       | Pantera_CLK       | Tomahawk_Z,X_3-5       |
> | Cenmax_St-5       | Faraon       | Leopard          | Pantera_XS/Jaguar | ZX-730-750-1055        |
> | Cenmax_St-7       | Genius_Bravo | Magic_1          | Partisan_RX       | IL-100(Smart)          |
> | Centurion         | Gibidi       | Magic_2          | Reff              | Merlin                 |
> | Monarch           | Jolly Motors | Magic_3          | Sheriff           | Steelmate              |
> | Motorline         | Rosh         | Pecinin          | Rossi             |                        |
> </details>
<br/>

<details>
<summary><code><strong>Decoders/Encoders or emulation (+ programming mode) support made by @xMasterX</strong></code></summary>
<br/>

- Elplast/P-11B/3BK/E.C.A (static 18 bit)
- Roger (static 28 bit) with add manually support (thanks @mishamyte)
- V2 Phoenix (Phox) (dynamic 52 bit) (thanks @RocketGod-git)
- Marantec (static 49 bit) (add manually support and CRC verify) (thanks @li0ard)
- Feron (static 32 bit)
- ReversRB2 / RB2M (static 64 bit) with add manually support
- Marantec24 (static 24 bit) with add manually support
- GangQi (static 34 bit) with button parsing and add manually support (thanks to @mishamyte for captures and testing, thanks @Skorpionm for help)
- Hollarm (static 42 bit) with button parsing and add manually support (thanks to @mishamyte for captures, thanks @Skorpionm for help)
- Hay21 (dynamic 21 bit) with button parsing
- Nero Radio 57bit (+ 56bit support)
- CAME 12bit/24bit encoder fixes (Fixes are now merged in OFW)
- Keeloq: Dea Mio, Genius Bravo, GSN, HCS101, AN-Motors, JCM Tech, MHouse, Nice Smilo, DTM Neo, FAAC RC,XT, Mutancode, Normstahl, Beninca + Allmatic, Stilmatic, CAME Space, Aprimatic (model TR and similar), Centurion Nova (thanks Carlos !), Hormann EcoStar, Novoferm, Sommer, Monarch (thanks @ashphx !), Jolly Motors (thanks @pkooiman !), IL-100(Smart) (thx Vitaly for RAWs), Motorline (with add manually support), Rosh, Pecinin, Rossi, Merlin, Steelmate (thanks @RocketGod-git)
</details>

<details>
<summary><code><strong>Protocols support made by Skorp (original implementation) and @xMasterX (current version)</strong></code></summary>
<br/>

- CAME Atomo → Update! check out new [instructions](/documentation/SubGHzRemoteProg.md)
- Nice Flor S → How to create new remote - [instructions](/documentation/SubGHzRemoteProg.md)
- FAAC SLH (Spa) → Update!!! (Programming mode!) Check out new [instructions](/documentation/SubGHzRemoteProg.md)  
- Keeloq: BFT Mitto → Update! Check out new [instructions](/documentation/SubGHzRemoteProg.md)
- Star Line
- Security+ v1 & v2
</details>

<details>
<summary><code><strong>Encoders made by @assasinfil and @xMasterX</strong></code></summary>
<br/>

- Somfy Telis → How to create new remote - [instructions](/documentation/SubGHzRemoteProg.md)
- Somfy Keytis
- KingGates Stylo 4k
- Alutech AT-4N → How to create new remote - [instructions](/documentation/SubGHzRemoteProg.md)
- Nice ON2E (Nice One) → How to create new remote - [instructions](/documentation/SubGHzRemoteProg.md)
</details>


## ❤️ Please support development of the project

The majority of this project is developed and maintained by me, @xMasterX.
Our team is small and the guys are working on this project as much as they can solely based on the enthusiasm they have for this project and the community. 
- `@mishamyte` - NFC, RFID, SubGHz and chats moderation
- `@quen0n` - Hardware, SubGHz and chats moderation
- `@Drone1950` - Reverse Engineering, telegram bot and chats moderation
- `@HackcatDev` - Support and chats moderation
- `@Leptopt1los` - NFC, RFID, Plugins, chat moderation and many other things
- `@gid9798` - SubGHz, Plugins, many other things - currently offline :( 
- `@assasinfil` - SubGHz protocols, NFC parsers, chat moderation
- `@Svaarich` - UI design and animations
- `@amec0e` - Infrared assets
- Community moderators in Telegram, Discord, and Reddit
- And of course our GitHub community. Your PRs are a very important part of this firmware and open-source development.

The amount of work done on this project is huge and we need your support, no matter how large or small. Even if you just say, "Thank you Unleashed firmware developers!" somewhere. Doing so will help us continue our work and will help drive us to make the firmware better every time.
Also, regarding our releases, every build has and always will be free and open-source. There will be no paywall releases or closed-source apps within the firmware. As long as I am working on this project it will never happen.  
You can support us by using links or addresses below:

| Service                                                                                                                                                                                        | Remark                    | QR Code                                                                                                                                                                                                                             | Link/Wallet                                                                                       |
|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------|
| <img src="https://cdn.simpleicons.org/patreon/dark/white" alt="Patreon" width="14"/> **Patreon**                                                                                               |                           | <div align="center"><a href="https://github.com/user-attachments/assets/a88a90a5-28c3-40b4-864a-0c0b79494a42"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | [patreon.com/mmxdev](https://patreon.com/mmxdev)                                                  |
| <img src="https://cdn.simpleicons.org/boosty" alt="Boosty" width="14"/> **Boosty**                                                                                                             | patreon alternative       | <div align="center"><a href="https://github.com/user-attachments/assets/893c0760-f738-42c1-acaa-916019a7bdf8"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | [boosty.to/mmxdev](https://boosty.to/mmxdev)                                                      |
| <img src="https://gist.githubusercontent.com/m-xim/255a3ef36c886dec144a58864608084c/raw/71da807b4abbd1582e511c9ea30fad27f78d642a/cloudtips_icon.svg" alt="Cloudtips" width="14"/> CloudTips    | only RU payments accepted | <div align="center"><a href="https://github.com/user-attachments/assets/5de31d6a-ef24-4d30-bd8e-c06af815332a"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | [pay.cloudtips.ru/p/7b3e9d65](https://pay.cloudtips.ru/p/7b3e9d65)                                |
| <img src="https://raw.githubusercontent.com/gist/PonomareVlad/55c8708f11702b4df629ae61129a9895/raw/1657350724dab66f2ad68ea034c480a2df2a1dfd/YooMoney.svg" alt="YooMoney" width="14"/> YooMoney | only RU payments accepted | <div align="center"><a href="https://github.com/user-attachments/assets/33454f79-074b-4349-b453-f94fdadc3c68"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | [yoomoney.ru/fundraise/XA49mgQLPA0.221209](https://yoomoney.ru/fundraise/XA49mgQLPA0.221209)      |
| <img src="https://cdn.simpleicons.org/tether" alt="USDT" width="14"/> USDT                                                                                                                     | TRC20                     | <div align="center"><a href="https://github.com/user-attachments/assets/0500498d-18ed-412d-a1a4-8a66d0b6f057"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `TSXcitMSnWXUFqiUfEXrTVpVewXy2cYhrs`                                                              |
| <img src="https://cdn.simpleicons.org/ethereum" alt="ETH" width="14"/> ETH                                                                                                                     | BSC/ERC20-Tokens          | <div align="center"><a href="https://github.com/user-attachments/assets/0f323e98-c524-4f41-abb2-f4f1cec83ab6"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`                                                      |
| <img src="https://cdn.simpleicons.org/bitcoin" alt="BTC" width="14"/> BTC                                                                                                                      |                           | <div align="center"><a href="https://github.com/user-attachments/assets/5a904d45-947e-4b92-9f0f-7fbaaa7b37f8"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`                                                      |
| <img src="https://cdn.simpleicons.org/solana" alt="SOL" width="13"/> SOL                                                                                                                       | Solana/Tokens             | <div align="center"><a href="https://github.com/user-attachments/assets/ab33c5e0-dd59-497b-9c91-ceb89c36b34d"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `DSgwouAEgu8iP5yr7EHHDqMNYWZxAqXWsTEeqCAXGLj8`                                                    |
| <img src="https://cdn.simpleicons.org/dogecoin" alt="DOGE" width="14"/> DOGE                                                                                                                   |                           | <div align="center"><a href="https://github.com/user-attachments/assets/2937edd0-5c85-4465-a444-14d4edb481c0"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`                                                              |
| <img src="https://cdn.simpleicons.org/litecoin" alt="LTC" width="14"/> LTC                                                                                                                     |                           | <div align="center"><a href="https://github.com/user-attachments/assets/441985fe-f028-4400-83c1-c215760c1e74"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`                                                     |
| <img src="https://bitcoincash.org/img/green/bitcoin-cash-circle.svg" alt="BCH" width="14"/> BCH                                                                                                |                           | <div align="center"><a href="https://github.com/user-attachments/assets/7f365976-19a3-4777-b17e-4bfba5f69eff"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `qquxfyzntuqufy2dx0hrfr4sndp0tucvky4sw8qyu3`                                                      |
| <img src="https://cdn.simpleicons.org/monero" alt="XMR" width="14"/> XMR                                                                                                                       | Monero                    | <div align="center"><a href="https://github.com/user-attachments/assets/96186c06-61e7-4b4d-b716-6eaf1779bfd8"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `41xUz92suUu1u5Mu4qkrcs52gtfpu9rnZRdBpCJ244KRHf6xXSvVFevdf2cnjS7RAeYr5hn9MsEfxKoFDRSctFjG5fv1Mhn` |
| <img src="https://cdn.simpleicons.org/ton" alt="TON" width="14"/> TON                                                                                                                          |                           | <div align="center"><a href="https://github.com/user-attachments/assets/92a57e57-7462-42b7-a342-6f22c6e600c1"><img src="https://github.com/user-attachments/assets/da3a864d-d1c7-42cc-8a86-6fcaf26663ec" alt="QR image"/></a></div> | `UQCOqcnYkvzOZUV_9bPE_8oTbOrOF03MnF-VcJyjisTZmsxa`                                                |


## 📱 Community Apps

Enhance your Flipper Zero with apps and plugins created by the community:

- **Extra Plugins & Packs:**  
  Check out the latest extra plugins and plugin packs (Extra Pack and Base Pack) on [GitHub](https://github.com/xMasterX/all-the-plugins/releases/latest).

- **Source Code & Full List:**  
  Find the complete list and source code at [xMasterX/all-the-plugins](https://github.com/xMasterX/all-the-plugins/tree/dev).

- **Official Apps Catalog:**  
  Browse the official Flipper Zero Apps Catalog on the [web](https://lab.flipper.net/apps) or via the [mobile app](https://flipperzero.one/downloads).


## 📁 Where I can find IR, Sub-GHz, ... files, DBs, and other stuff?
- [UberGuidoZ Playground - Large collection of files - Github](https://github.com/UberGuidoZ/Flipper)
- [Awesome Flipper Zero - Github](https://github.com/djsime1/awesome-flipperzero)


## 📘 Instructions

### ![Tools Icon Badge] Firmware & main Apps feature

- System: [How to change Flipper name](/documentation/CustomFlipperName.md)
- BadUSB: [How to add new keyboard layouts](https://github.com/dummy-decoy/flipperzero_badusb_kl)
- Infrared: [How to make captures to add them into Universal IR remotes](/documentation/InfraredCaptures.md)  

### ![SubGhz Icon Badge] Sub-GHz

- [How to use Flipper as new remote (Nice FlorS, BFT Mitto, Somfy Telis, Aprimatic, AN-Motors, etc..)](/documentation/SubGHzRemoteProg.md)  
- External Radio: [How to connect CC1101 module](https://github.com/quen0n/flipperzero-ext-cc1101)  
- Transmission is blocked? [How to extend Sub-GHz frequency range](/documentation/DangerousSettings.md)
- [How to add extra Sub-GHz frequencies](/documentation/SubGHzSettings.md)
- [~~Configure Sub-GHz Remote App~~](/documentation/SubGHzRemotePlugin.md) ⚠️ Not recommended, please use embedded configurator

### ![Plugins Icon Badge] Plugins

- TOTP (Authenticator): [config description](https://github.com/akopachov/flipper-zero_authenticator/blob/master/docs/conf-file_description.md) 
- Barcode Generator: [How to use](/documentation/BarcodeGenerator.md)
- Multi Converter: [How to use](/documentation/MultiConverter.md)
- WAV Player: [sample files & how to convert](https://github.com/UberGuidoZ/Flipper/tree/main/Wav_Player#readme)  
- Sub-GHz playlist: [generator script](https://github.com/darmiel/flipper-scripts/blob/main/playlist/playlist_creator_by_chunk.py)

### ![GPIO Icon Badge] GPIO - Plugins that works with external hardware

- Unitemp - Temperature sensors reader: [How to use & supported sensors](https://github.com/quen0n/unitemp-flipperzero#readme)
- [NMEA] GPS: [How to use](https://github.com/xMasterX/all-the-plugins/blob/dev/base_pack/gps_nmea_uart/README.md)  
- i2c Tools [How to use](https://github.com/xMasterX/all-the-plugins/blob/dev/base_pack/flipper_i2ctools/README.md)  
- [NRF24] plugins: [How to use](/documentation/NRF24.md)  
- [WiFi] Scanner: [How to use](https://github.com/SequoiaSan/FlipperZero-WiFi-Scanner_Module#readme) | [Web Flasher](https://sequoiasan.github.io/FlipperZero-WiFi-Scanner_Module/)
- [ESP8266] Deauther: [How to use](https://github.com/SequoiaSan/FlipperZero-Wifi-ESP8266-Deauther-Module#readme) | [Web Flasher](https://sequoiasan.github.io/FlipperZero-Wifi-ESP8266-Deauther-Module/)
- [ESP32] WiFi Marauder: [How to use](https://github.com/UberGuidoZ/Flipper/tree/main/Wifi_DevBoard)<sub> docs by UberGuidoZ</sub> | [Marauder repo](https://github.com/justcallmekoko/ESP32Marauder)
- [ESP32-CAM] Camera Suite: [How to use](https://github.com/CodyTolene/Flipper-Zero-Camera-Suite)
- How to Upload `.bin` to ESP32/ESP8266: [Windows](https://github.com/SequoiaSan/Guide-How-To-Upload-bin-to-ESP8266-ESP32) | [FAP "ESP flasher"](https://github.com/0xchocolate/flipperzero-esp-flasher)
- [GPIO] SentrySafe plugin: [How to use](/documentation/SentrySafe.md)


## 👨‍💻 Firmware & Development

- **Developer Documentation** - [developer.flipper.net](https://developer.flipper.net/flipperzero/doxygen)  
- **[How to build](/documentation/HowToBuild.md#how-to-build-by-yourself) | [Project-structure](#project-structure)**
- **CLion IDE** - How to setup workspace for flipper firmware development [by Savely Krasovsky](https://krasovs.ky/2022/11/01/flipper-zero-clion.html)
- **"Hello world!"** - plugin tutorial [English<sub> by DroomOne</sub> ](https://github.com/DroomOne/Flipper-Plugin-Tutorial) | [Russian<sub> by Pavel Yakovlev</sub>](https://yakovlev.me/hello-flipper-zero)
- [How to write your own app](https://flipper.atmanos.com/docs/overview/intro).

### Project structure

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


## 🔗 Links
- **Unleashed web page:** [flipperunleashed.com](https://flipperunleashed.com)
- **Unleashed update server, direct .tgz update links for web updater or direct download:** [unleashedflip.com](https://unleashedflip.com)

- Official Docs: [docs.flipper.net](https://docs.flipper.net)
- Official Forum: [forum.flipperzero.one](https://forum.flipperzero.one)
- Update! Developer Documentation [developer.flipper.net](https://developer.flipper.net/flipperzero/doxygen)


[RFID Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(255,244,147)?style=flat&logo=fz-rfid&logoColor=black
[iButton Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(225,187,166)?style=flat&logo=fz-ibutton&logoColor=black
[SubGhz Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(165,244,190)?style=flat&logo=fz-subghz&logoColor=black

[GPIO Badge]: https://custom-icon-badges.demolab.com/badge/-GPIO-rgb(167,242,234)?style=flat&logo=fz-gpio&logoColor=black
[GPIO Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(167,242,234)?style=flat&logo=fz-gpio&logoColor=black

[Tools Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(223,241,89)?style=flat&logo=fz-tools&logoColor=black
[Media Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(223,181,255)?style=flat&logo=fz-media&logoColor=black
[BT Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(139,172,255)?style=flat&logo=fz-bluetooth&logoColor=black
[NFC Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(152,206,254)?style=flat&logo=fz-nfc&logoColor=black
[USB Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(255,190,233)?style=flat&logo=fz-badusb&logoColor=black
[IR Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(254,147,140)?style=flat&logo=fz-infrared&logoColor=black
[Games Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(255,196,134)?style=flat&logo=fz-games&logoColor=black
[Plugins Icon Badge]: https://custom-icon-badges.demolab.com/badge/-rgb(226,78,178)?style=flat&logo=fz-plugins&logoColor=black

[UFW Icon Badge]: https://img.shields.io/badge/by_UFW-%2314D411?style=flat-square
[Official Icon Badge]: https://img.shields.io/badge/by_OFW-%23FF8200?style=flat-square
[Author Icon Badge]: https://img.shields.io/badge/by_author-%23FFFF00?style=flat-square
[None Icon Badge]: https://img.shields.io/badge/None-%23FF0000?style=flat-square
