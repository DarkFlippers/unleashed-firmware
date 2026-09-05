## Main changes
- Current API: 88.6
- SubGHz: **Add Superrollo (GW60) roller-shutter protocol** (KeeLoq HCS361) (67bit rolling code, with CRC) (with add manually support) (PR #1068 | by @rollorentner)
- SubGHz: **Read no longer adds a copy of the last received signal a few seconds after it arrived** - the duplicate filter now measures the gap between the signals themselves instead of the time they reached the app, so the last repeat of a burst is recognised as a repeat no matter how late the receiver reports it
- Desktop: **Second page in the up-button menu** - press left or right there for screen brightness, volume and vibro
- Desktop: **Main menu styles** - Settings -> Desktop -> Menu Style picks between Default, PS4, Wii, DSi, C64, Compact, CoverFlow, Vertical and Grid, each shipped as its own plugin (by @apfxtech | PR #1119 & PR #1126) - most of the layouts are ported from [Momentum Firmware](https://github.com/Next-Flip/Momentum-Firmware), original designs by their authors: Wii, DSi, PS4 and Vertical by @WillyJL, C64 by @Sil333033, Compact by @MatthewKuKanich, CoverFlow by @CodyTolene & @956MB, Grid by @apfxtech
- Infrared: **Save a signal straight from the Universal Remote** - pause the brute force and press down to keep the signal that just worked, as a new remote or appended to an existing one
- NFC: **Save recovered MIFARE Classic keys to the user dictionary** - new "Save Keys to Dictionary" action on a read or saved card, so keys found by any attack (including the per-UID dictionary used for static-encrypted-nonce cards) become available to future reads and to NFC Magic; keys the system or user dictionary already holds are skipped (by @mishamyte | PR #1118 | Closes #1117)
- Apps: Build tag (**4sep2026**) - **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
## Other changes
- GUI: File browser keeps only the file name of each listed entry instead of its whole path, and rebuilds the full path from the folder it is showing when one is picked - a folder of long names no longer costs a few KB of RAM to display (ported from [Momentum Firmware](https://github.com/Next-Flip/Momentum-Firmware) | by @WillyJL)
- NFC: Type 4 Tag - selecting a file by EF id now sends P1 = 0x00 rather than 0x02, so cards that reject the "select EF under the current DF" mode can be read (ported from [Momentum Firmware](https://github.com/Next-Flip/Momentum-Firmware) | by @WillyJL)
- GUI: Keys pressed while an application is showing a loading animation are no longer delivered to the screen that replaces it - they were queued and replayed once the event loop began, where a stray Left or Right could change a setting the user never saw (by @mishamyte | PR #1125 | Closes #1124)
- GUI: Archive and Desktop settings put a loading animation up while their first screen is built, instead of leaving the menu they were opened from on display; any app can do the same with the new view_dispatcher_show_loading(), which owns the view so it costs no view id and nothing to free (by @mishamyte | PR #1125 | Closes #1124)
- Desktop: Settings -> Desktop now shows a loading screen instead of the menu it was opened from while it starts - the menu style plugins are scanned behind it, and once per run instead of on every return to the settings list (by @mishamyte | PR #1123 | Fixes #1122)
- Desktop: Main menu styles - Left and Right now move between the columns of the C64 and Compact layouts whenever the other column has anything in it; on the stock menu the bottom half of Compact's left column did nothing at all (by @mishamyte | PR #1121 | Fixes #1120)
- Desktop: Main menu styles - the Vertical layout no longer streams a sideways screen to qFlipper and the mobile app, a style plugin that fails to load or ships an incomplete vtable is now reported instead of silently ignored, a style file that has gone missing no longer reads as "Default" in Settings, and the style plugin ABI is documented (by @mishamyte | PR #1121 | Fixes #1120)
- NFC: Adding a key to a user dictionary no longer rewrites the whole file - it is appended instead of inserted at the end, which also speeds up MFKey32 writing back a batch of recovered keys (by @mishamyte | PR #1118)
- SubGHz: Fixed a one-past-the-end write when building a transmission (just in case) - the final level duration was stored without a bounds check (by @MNeroba | PR #1105)
- SubGHz: The free/stop/yield/reset/hash/serialize handlers that were byte-identical across 57 protocols now share one implementation instead of 346 copies, freeing ~6 KB of flash (thanks @apfxtech !)
- SubGHz & Storage: A further ~4.8 KB of flash freed - the alloc, deserialize and remaining serialize bodies still duplicated across SubGHz protocols, and the twelve Storage API calls that differed only in the command they send, now share one implementation each (by @mishamyte | PR #1116)
- NFC: FeliCa - a saved dump claiming more blocks than the card can hold is now rejected on load, instead of being read past the end of the block array (by @MNeroba | PR #1106)
- HID: Mouse Jiggler (Stealth) - movement is now generated within the signed 8-bit range that HID mouse reports carry, instead of a +-1000 value that was truncated before it was sent (by @MNeroba | PR #1111)
- Expansion: Fixed an off-by-one that accepted FuriHalSerialIdMax itself as a serial id when setting an expansion module callback (by @MNeroba | PR #1108)
- NFC: stop the CUID dictionary pass ending one key index early
- NFC: keep the CUID dictionary pass end marker out of the sector counter (by @mishamyte | PR #1115 | Fixes #1114)
- OFW: Fix typo in BT HAL source
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
