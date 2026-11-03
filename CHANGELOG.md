## Main changes
- Current API: 88.23
- SubGHz: **Nice O-Code remotes that send the longer 72bit frame now work, as `Nice O-Code One 433MHz` in Add Manually**
- SubGHz: **Nice O-Code 112bit frames carrying the receiver certificate are decoded and sent, as `Nice O-Code+Crt 433MHz`**
- SubGHz: **Nice O-Code shows the certificate as the receiver's own value, masked with each remote's serial only on air**
- SubGHz: **Nice O-Code sends the k byte an O-Code remote really uses, not the Nice One one**
- SubGHz: **Nice O-Code changes the certificate frame index from press to press, as a real remote does**
- SubGHz: **Nice O-Code sets the end-of-press marker on the last two parcels, as a real remote does**
- SubGHz: **Nice O-Code counts like a real remote, and `Cnt` is the plain press count everywhere**
- SubGHz: **Fix a received Nice O-Code frame being saved as if its installer code were FFFF**
- SubGHz: **Fix a Nice FLO remote that runs out of spec also being read as an Ansonic remote**
- Apps: **`Nice O-Code` can decode a saved RAW `.sub` file and shows the certificate beside the installer code**
- Apps: **`Nice O-Code` - `New remote` mints a serial the gate is certain to enrol**
- Apps: **`Seed Capturer` can decode a saved RAW `.sub` file and uses an external CC1101 module**
- Apps: **`Seed Capturer` supports BFT Mitto remotes on 433.92 MHz, the same way as Erreka**
- SubGHz: **Nero Radio rolling remotes are now decoded and can be replayed as is**
- SubGHz: **Dooya remotes support custom buttons** (by @Lechnio | Momentum PR 597)
- SubGHz Remote: **The sending button is highlighted and the last pressed button's details are shown**
- SubGHz Remote: **Hold a button for endless TX, one click sends the full signal**
- SubGHz Remote: **RAW is sent in full with one click, and repeats while the button is held**
- SubGHz Remote: **Fix rolling code and custom button resets after TX, and the counter moving twice per press**
- Apps: **`SubGHz WarDriving` - RX restarts after TX, and sending a received dynamic signal no longer overwrites a saved file**
- SubGHz: **KeeLoq `CounterMode: 7` now opens a Doorhan on the first press**
- SubGHz: **The read list is no longer capped at 55 signals, and Hold Right deletes one**
- SubGHz: **`Delete old signals` makes room by memory now, for a send as well as a read**
- SubGHz: **Send no longer crashes on a full list**
- SubGHz: **Fix a duplicated entry in the read list after deleting a signal**
- SubGHz: **10 KB of RAM freed while BinRAW is off**
- Apps: **`SubGHz WarDriving` no longer crashes on receive with BinRAW on**
- Apps: **`SubGHz WarDriving` autosaves off the decoder thread, so signals arriving during a save are not missed**
- Apps: **`SubGHz WarDriving` - `Remove duplicates` left duplicates behind and could delete the wrong signal**
- RFID: **Better FDX-B (animal chips / implants) reading, also in Extra Actions -> `Read FDX-B (Animals)`**
- RFID: **New T5577 app - read a T5577, save it, write it back** (by @mishamyte | PR #1179)
- RFID: **New Test Tag in the T5577 app** (by @mishamyte | PR #1179)
- RFID: **The firmware can read a single T5577 block** (by @mishamyte | PR #1179)
- RFID: **Wipe Tag and Clear Password check the tag instead of assuming, and the password list lives on the card** (by @mishamyte | PR #1179)
- RFID: **Fix `Write and set password` zeroing a tag's config instead of protecting it** (by @mishamyte | PR #1179 | Fixes #1180)
- RFID: **Write and set password no longer hangs with the tag left locked** (by @mishamyte | PR #1179)
- RFID: **T5577 dumps save to `/ext/lfrfid` and open from the Saved list** (by @mishamyte | PR #1188 | Closes #1187)
- RFID: **T5577 app - leaving a dump returns to the list it was picked from** (by @mishamyte | PR #1188)
- RFID: **Fix the T5577 app moving a dump when renaming it** (by @mishamyte | PR #1188)
- RFID: **New `Sniff Password` in the T5577 app - recover a password by listening to a reader or cloner write a tag** (by @mishamyte | PR #1192)
- RFID: **T5577 passwords you find are kept in a dictionary of your own** (by @mishamyte | PR #1192)
- RFID: **Fix the T5577 app sending a password to a tag that never asked for one** (by @mishamyte | PR #1192)
- RFID: **Fix the T5577 app ignoring Back while it looks for a tag** (by @mishamyte | PR #1192)
- RFID: **T5577 `Sniff Password` says `Too much noise` instead of reporting silence when the coil itself chatters** (by @mishamyte | PR #1202)
- RFID: **T5577 app - a Test Tag backup is deleted once the tag is proven restored, and warns before overwriting one that survives** (by @mishamyte | PR #1179)
- SubGHz: **Fix Marantec24 and Linear Delta 3 not decoding a signal the Flipper itself sends** (by @mishamyte | PR #1207 | Closes #1205)
- SubGHz: **Fix Holtek HT12X reporting a key off one frame when the receiver had just been reset** (by @mishamyte | PR #1207)
- Apps: Build tag (**2nov2026p2**) - **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
## Other changes
- SubGHz: **CAME TWEE no longer reads past the end of its decode table on a parcel counter of F**
- SubGHz: **Fix encoder buffers too small for the keys Came Atomo, Somfy Keytis, Somfy Telis, Power Smart and Dooya accept**
- Apps: **`SubGHz WarDriving` - `Delete old signals` can no longer empty the whole list, and Send refuses instead of crashing**
- Apps: **Fix the loading spinner flashing when you close an app opened from `Apps`** (by @mishamyte | PR #1195 | Fixes #1178)
- OFW: **Typos fixed in on-screen text**
- OFW: **FatFs: apply upstream R0.12c patches, fix 0-byte files in fragmented exFAT directories**
- OFW: **NFC: reject Ultralight READ/WRITE starting at the page count**
- API: **Export `faac_slh_reset_prog_mode` for apps**
- API: **Export the T5577 block read/write API for apps (`t5577.h`, `t5577_file.h`)** (by @mishamyte | PR #1179)
- iButton: **Fix write success text going off screen**
- RFID: **Move the Writing screen text up so long file names fit**
- GUI: **A screen that is covered no longer repaints the one on top of it** (by @mishamyte | PR #1198 | Closes #1196)
- GUI: **Delete screens (SubGHz, NFC, RFID, iButton, Archive) fit a long file name**
- Archive: **Stability pass over the file menu, deleting during a load, favourites and Show**
- RFID: **T5577 app - `Saved` opens straight away, and a dump that will not open says why** (by @mishamyte | PR #1189)
- RFID: **Fix the missing-T5577-app message pointing at the apps catalogue** (by @mishamyte | PR #1189)
- Archive: **Fix the rename check missing an existing file whose extension is six characters long** (by @mishamyte | PR #1188)
- RFID: **~1 KB of flash freed by sharing the duplicated LF write_data bodies** (by @mishamyte | PR #1183)
- Dev: **Fix the infrared unit test checking a universal remote database that does not exist** (by @mishamyte | PR #1199)
- Dev: **Fix the unit test firmware not building** (by @mishamyte | PR #1199 | Closes #1185)
- Dev: **A unit test run that tested nothing no longer reports `Status: PASSED`** (by @mishamyte | PR #1206)
- Dev: **CI builds the unit test firmware** (by @mishamyte | PR #1206)
- Dev: **The NFC supported-card parsers have unit tests** (by @mishamyte | PR #1204)
- Dev: **Unit test encoder round-trips feed the decoder what the radio would send** (by @mishamyte | PR #1207)
- Core: **~7 KB of flash freed by parsing numbers without newlib's big-number machinery** (by @mishamyte | PR #1186 | Closes #1184)
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
