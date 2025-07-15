## Main changes
- Current API: 86.0
* SubGHz: **Roger (static 28 bit) with add manually support** (by @xMasterX & @mishamyte)
* SubGHz: **V2 Phoenix full support** (button switch, add manually, counter decrypt/encrypt) (by @xMasterX & @RocketGod-git, original code by @Skorpionm)
* SubGHz: **Keeloq: Add support for - Motorline (with add manually support), Rosh, Pecinin, Rossi, Merlin, Steelmate** (by @xMasterX & @RocketGod-git)
* SubGHz: **Nero Radio static parse** and display more data
* SubGHz: Reduce less popular freqs in default hopper preset, **make it faster**
* SubGHz: **Marantec protocol implement CRC verification display and Add manually support** (by @xMasterX & @li0ard, original code by @Skorpionm)
* SubGHz: **Keeloq: Comunello - add manually support**
* iButton: **TM01x Dallas write support** (PR #899 | by @Leptopt1los)
* SubGHz: Rename and **extend Alarms, Sensors, Cars ignore options** (Alarms: Hollarm, GangQi | Cars: Kia, Starline, ScherKhan | Sensors: Magellan, Honeywell, Honeywell WDB (doorbells), Legrand (doorbells), Feron (RGB lights))
* SubGHz: V2 Phoenix show counter value (upd: see above, now decrypted)
* SubGHz: **Add Keeloq IronLogic (aka IL100) smart clone remote copiers support** (thanks to Vitaly for RAWs)
* SubGHz: **Fix CAME 24bit decoder**
* SubGHz: Add 462.750 MHz & 868.46 MHz to default subghz freqs list
* SubGHz: **Tune Holtek HT12x to decode Holtek only** and not conflict with came 12bit
* SubGHz: Fix Rename scene bug, that was replacing file name with random name when Rename is opened then closed then opened again
* Display: Backlight option "always on" and RGB bug removed (PR #900 | by @Dmitry422)
* NFC: Ultralight C - Attempt of authentication with default key (PR #898 | by @mishamyte)
* System: Loader - Fix misplaced ApplicationBeforeLoad events (PR #905 | by @WillyJL)
* OFW PR 4210: Infrared: Add text scroll to remote buttons (by @956MB)
* NFC:
  - **NFC Type 4 support + many other improvements** (by @WillyJL)
    - New Type 4 Tag (NDEF on NTAG4xx / MIFARE DESFire) protocol, full support
    - New NTAG4xx (NTAG413 DNA / NTAG424 DNA) protocol, only detection and basic info support
    - NDEF parsing plugin supports Type 4 Tag protocol
    - Show more version info for MIFARE Plus cards
    - Improve detection/verification of MIFARE DESFire and MIFARE Plus SE
    - Improve navigation for MIFARE Classic Update from / Write to Initial Card
    - Refactor Write code for MIFARE Ultralight/Classic in NFC app helpers
    - Cleanup event handling in NFC app
    - NFC app uses a bit less RAM because of previous 2 points
    - Refactor NXP Native Commands to share between protocols (used by MIFARE DESFire, MIFARE Plus, NTAG4xx)
    - MIFARE DESFire poller API can now switch between native and ISO7816-wrapped commands
    - Expand ISO14443-4A API with listener (emulation) support for sending responses to reader (except I-block chaining)
    - Exposed some APIs for apps to use that were meant to be public:
      - ISO14443-3A listener (emulation)
      - ISO15693-3 device (data), poller (reading), listener (emulation)
    - Cleanup/reorder protocol definitions for tidyness
  - Ventra ULEV1 parser (by @hazardousvoltage)
  - CSC Service Works parser (by @zinongli)
  - Philips Sonicare parser (by @Sil333033)
  - SmartRider parser (by @jaylikesbunda)
* Apps: **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
## Other changes
* BadUSB: Fix modifier keys with HOLD/RELEASE commands (by @WillyJL)
* Docs: Update doorhan programming instructions (by @li0ard)
* FuriHalSerial: Fix RXFNE interrupt hang, aka freezing with UART output when Expansion Modules are enabled (by @WillyJL)
* Expansion: add is_connected api (by @HaxSam & @WillyJL)
* RFID 125khz: Fix strange bug with LCD backlight going off after doing "Write"
* GUI: Added `submenu_remove_item()` to API, was needed for NFC Type 4 related changes (by @WillyJL)
* SubGHz: Fix possible frequency analyzer deadlock when holding Ok (by @WillyJL)
* RFID 125khz: Add DEZ10 representation to EM410X (by @realcatgirly)
* OFW PR 4205: fix sample durations when using external CC1101 (by @Aerosnail)
* OFW PR 4206: Stop JS PWM on exit (by @portasynthinca3)
* OFW PR 4212: Fixed inverted logic condition in subghz chat cli (by @GameLord2011)
* NFC: Fix clipper date timestamp (PR #903 | by @luu176)
* Desktop: DEBUG - fix desktop anim switch override by favourite apps
* CLI: Various fixes (by @WillyJL)
* BadUSB: Fix key combos main keys being case sensitive (by @WillyJL)
* System: log level none after update
* Docs: Some updates on subghz remotes programming
<br><br>
#### Known NFC post-refactor regressions list: 
- Mifare Mini clones reading is broken (original mini working fine) (OFW)
- NFC CLI was removed with refactoring (OFW) (will be back soon)

----

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper (official link)](https://flipperzero.one/update)

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


#### Thanks to our sponsors who supported project in the past and special thanks to sponsors who supports us on regular basis:
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



