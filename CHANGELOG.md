## Main changes
- Current API: 87.8
* SubGHz: Add support for **42+ Keeloq based systems** (with partial Add Manually support) (see [Full list](/documentation/SubGHzSupportedSystems.md)) (by @zero-mega, @xMasterX, ARF Team)
* SubGHz: Add **Allstar Firefly 318ALD31K** protocol (18 bits, Static) (PR #989 | by @jlaughter)
* SubGHz: Add **Nord ICE** protocol (33 bits, Static)
* SubGHz: **Better support for CAME Atomo** type remotes (TOPD4REN) (decode + button codes) (thx to Roman for raw recordings)
* SubGHz: Add **CAME TOP44FGN** support in CAME TWEE protocol
* SubGHz: Add all 0x0s and all 0xFs KeeLoq MF codes for normal and simple learning
* SubGHz: **Fix CAME TWEE repeats count for button click**
* SubGHz: Improve Nice FLO decoding (thx to Roman for raw recordings)
* NFC: Add **ISO15693-3 and SLIX write-back support** (PR #984 | by @DoniyorI)
* NFC: **Fix "MIR" and other EMV cards crash on Read** (by @Dmitry422)
* NFC: Add **Mifare Ultralight C Write Support** (by @haw8411)
* NFC: Add **new parsers SZPPK, SKPPK and SevPPK**, upgrade Plantain parser, fix TwoCities parser (PR #981 | by @mxcdoam)
* NFC: Fix CLI with NTAG4xx and Type 4 Tag support (by @WillyJL)
* OFW PR 4362: NFC: **Fix BusFault** in Write to Initial Card (by @akrylysov)
* OFW PR 4369: NFC: Fix stack buffer overflows in MFUL FAST_READ and DESFire file settings parsers (by @qp-x-qp)
* Apps: Build tag (**5may2026**) - **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
## Other changes
* UI: Add `canvas_get_buffer`, `canvas_get_buffer_size` to public API
* HID App: PTT improvements (PR #996 | by @hryamzik)
* UI: Wording changes to be more clear (PR #994 | by @SkeletonMan03)
* SubGHz: Fix duplicate code in RAW protocol
* SubGHz: Fix small logic error in KingGatesStylo4k protocol (found by @X-Stuff)
* OFW PR 4364: JS: Address utf8 support when uploading JavaScript application (by @bekindpleaserewind)
* Display: Extend lcd contrast range to full ST756x 6-bit range (by @ShaTie) (reduced to -10 to +18 to avoid fully unreadable state)
<br><br>
#### Known NFC post-refactor regressions list: 
- Mifare Mini clones reading is broken (original mini working fine) (OFW)

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



