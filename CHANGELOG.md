## Main changes
- Current API: 88.27
- Apps: Build tag (**27nov2026p3**) - **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
- SubGHz: **New protocol - BelKorn, 118 bits, rolling code with no encryption, 36 bit serial, with custom buttons** - (Korn, BelGate, Furniteh and other rebrands; the frame carries no manufacturer field, so they all read as `BelKorn`)
- SubGHz: **Add Manually: `BelKorn`**
- SubGHz: **The read list names the system a multi-system protocol decoded, instead of the protocol - CAME, Nice FloR-S, Faac SLH and KeeLoq**
- SubGHz: **DEA Mio sends back the discriminator the original remote sent, instead of always setting the overflow bits**
- SubGHz: **DEA Mio asks for the discriminator in Advanced Add Manually and shows it as `Ds:`**
- SubGHz: **Advanced Add Manually asks for TE where the protocol carries one - Princeton, ZKTeco and FAAC TM**
- SubGHz: **Advanced Add Manually asks for the guard time after TE, where the protocol carries one - Princeton and ZKTeco**
- SubGHz: **Add Manually has one entry per system, with the frequency and modulation picked in the next screen**
- SubGHz: **The band picker lists the system's own frequencies first and every other one below, so an odd installation can still be crafted**
- SubGHz: **`More` on a saved RAW file now has an `Edit` entry that opens it straight in Sub-GHz RAW Edit and returns to Sub-GHz when you leave the editor** (by @Lechnio | PR #1221)
- Apps: **Sub-GHz RAW Edit ships with the firmware now** - it is no longer part of the extra pack (by [Lechnio](https://github.com/Lechnio/SubGHz-RAW-Edit))
- Apps: **New app - SIM800, drives a SIM800 GSM module over the GPIO UART: voice calls, SMS, a phonebook of numbers and SMS presets, and the module's network, inbox, USSD, DTMF, audio and power commands** - `Apps -> GPIO -> SIM800`, see [wiring and powering](/documentation/SIM800.md)
- Archive: **New `Phone` tab for the SIM800 phonebook - a `.phone` entry opens straight in the app**
- BadUSB: **New `DUCKY_LANG` script command - a script picks its own keyboard layout from `badusb/assets/layouts/<name>.kl`, instead of every script running on the one layout set in the app** (the native `.kl` map is ASCII only, and a missing or invalid name falls back to the built-in en-US) (by @ANTONiO-eagle | PR #1224 | Closes #1218)
- SubGHz: **New KeeLoq manufacturer - FAAC FIX, with `KL: FAAC FIX` in Add Manually and custom button support** (thanks @Anat01y)
- SubGHz: **Add Manually: `Alutech AT4L`** (`Alutech AT-4N` protocol with `0x1Dxxxxxx` serial prefix)
- SubGHz: **Alutech AT-4N CounterMode 1 & 2 now transmit full smart clone packs** - Mode 1 uses a 3-press cycle (`0x0000` x4, `0x0001` x4, then `0xFFFE` x4 + `0xFFFF` x4 in one press for MCSW-3.3M); Mode 2 sends `0x0000..0x0005` (14 frames) in a single press while preserving the file counter
- JS: **New `usbdisk.createImage(path, size)` method to create empty disk images**
## Other changes
- OFW: fix: resolve PVS-Studio warnings reported on dev
- OFW: Fix BLE key storage callback lifetime across radio restarts
- OFW: Fix BadUSB `MOUSESCROLL`/`MOUSEMOVE` reading past end of line without arguments
- OFW: Storage CLI: stop `storage write` and `read_chunks` when the host closes the session
- NFC: **Fix third-party NFC apps reading the wrong NTAG/Ultralight pages (Weebo "Wrong password" on every amiibo) - the Ultralight AES fields no longer sit in the middle of `MfUltralightData`**
- SubGHz: **Fix the read list beep going missing with `Delete old signals` on and memory full**
- SubGHz: **A signal turned away by a full read list now buzzes with `Delete old signals` on too**
- Remote Analyzer: **BelGate dropped from the app, it is a firmware Sub-GHz protocol now**
- Remote Analyzer: **`How to use` lists the protocols the app decodes on its own**
- SubGHz: **Advanced Add Manually key field is now as long as the protocol's bits, instead of padding it with `00` to 8 bytes**
- SubGHz: **Fix Add Manually failing on FAAC TM - only Princeton carries a `Guard_time`, so writing one into every TE protocol broke it**
- SubGHz: **`subghz rx`, `tx` and `tx_from_file` over the CLI work on the internal radio again, instead of stopping at `Device 0 failed to start`** (by @GodKimba | PR #1216 | Fixes #1215)
- SubGHz: **Feron frames are verified against their checksum, instead of any 32 bits that fit the timings decoding as Feron** (by @Lechnio | PR #1219)
- SubGHz: **A settings file from another firmware no longer hides whole groups of protocols in Read - only the bits the `Ignore` options own are kept from a saved `IgnoreFilter`** (by @Lechnio | PR #1220)
- RFID: **Fix Gallagher writes to EM4305 being shifted by one bit, so the written tag did not read back** (by @Firat17aze | PR #1217)
- SubGHz: **`Signal settings` and `Radio settings` ship as embedded `.fal` plugins, so their scenes are only resident while the page is open** (by @Lechnio | PR #1222)
- SubGHz: **Two more frequencies in the default list - 317.0 and 336.1 MHz**
- SubGHz: **Fix `Edit button` in Signal Settings not saving the new button to the file for static protocols**
- JS: **Fix `subghz.isExternal()` reporting internal radio when an external CC1101 is connected**
- GUI: **Fix TextInput writing past its buffer when drawing the cursor and mishandling 256+ character strings** (by @jeje77131 | PR #1239)
- SubGHz: **Serialize receive history between the worker and GUI, and keep the open signal from being dropped when `Delete old signals` is on** (by @jeje77131 | PR #1240)
- GPS, Network: **Unsubscribing now waits for a running callback to finish** (by @jeje77131 | PR #1241)
- SubGHz: **Fix renaming a saved file with a 64+ character name dropping its `.sub` extension** (by @jeje77131 | PR #1242)
- Furi: **`realloc()` copies only the old block when growing, and `calloc()` checks for overflow** (by @jeje77131 | PR #1243)
- SIM800: **Fix SIM800C wake/baud/profile-store timeouts and persist audio settings; add embedded Cyrillic `.fal` viewer for SMS, history, and USSD with UCS2 decoding, cached chunked inbox, multipart SMS read/delete/history, delete confirmation, and incoming SMS alerts**
- SubGHz: **Fix KeeLoq/KeeloqHR false positives on ~250us 66-bit remotes by tightening sync-gap bounds and validating bit period**
<br><br>

----

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Unleashed FW Web Installer](https://web.unleashedflip.com)

## Please support development of the project

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

#### Thanks to our sponsors who supported project in the past and special thanks to sponsors who supports us on regular basis

@mishamyte, ClaraCrazy, Pathfinder [Count Zero cDc], callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
and all other great people who supported our project and me (xMasterX), thanks to you all!

## **Recommended update option - Web Updater**

### What `e`, ` `, `c` means? What I need to download if I don't want to use Web updater?

What build I should download and what this name means - `flipper-z-f7-update-(version)(e / c).tgz` ? <br>
`flipper-z` = for Flipper Zero device<br>
`f7` = Hardware version - same for all flipper zero devices<br>
`update` = Update package, contains updater, all assets (plugins, IR libs, etc.), and firmware itself<br>
`(version)` = Firmware version<br>

| Designation | [Base Apps](https://github.com/xMasterX/all-the-plugins#default-pack) | [Extra Apps](https://github.com/xMasterX/all-the-plugins#extra-pack) |
|-----|:---:|:---:|
| ` ` | ✅ |  |
| `c` |  |  |
| `e` | ✅ | ✅ |

**To enable RGB Backlight support go into LCD & Notifications settings**

⚠️RGB backlight [hardware mod](https://github.com/quen0n/flipperzero-firmware-rgb#readme), works only on modded flippers! do not enable on non modded device!

Firmware Self-update package (update from microSD) - `flipper-z-f7-update-(version).tgz` for mobile app / qFlipper / web<br>
Archive of `scripts` folder (contains scripts for FW/plugins development) - `flipper-z-any-scripts-(version).tgz`<br>
SDK files for plugins development and uFBT - `flipper-z-f7-sdk-(version).zip`
