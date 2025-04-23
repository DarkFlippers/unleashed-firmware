## Main changes
- Current API: 86.0
**WARNING! After install of this version your Desktop (fav apps) and LCD & Notifications settings will be reset to default values, please configure them again after this update!** (this is required due to big updates on that parts and config struct changes)
* SubGHz: Add **Feron** protocol (static 32 bit) **full support** (by @xMasterX)
* SubGHz: Add **Revers RB2 / RB2M Protocol** (static 64 bit) **full support** with add manually (by @xMasterX)
* SubGHz: **Fix Hollarm protocol with more verification**
* SubGHz: **Fix GangQi protocol** (by @DoberBit and @mishamyte (who spent 2 weeks on this))
* SubGHz: **Came Atomo button hold simulation with full cycle** simulation (to allow proper pairing with receiver)
* SubGHz: Add **Prastel (42bit static code)** support (OFW PR 4178 by @pmazzini)
* Desktop: **Add support for Favorite App - Ok Long** (Warning! Old favourites apps list will be reset!) (PR #886 | by @DrEverr)
* Display: **LCD Color Inversion** (Settings - LCD and Notifications - LCD Inversion) (PR #887 #893 | by @Dmitry422)
* Display: **Night Shift Feature** (dimming backlight in selected time interval) (PR #885 #896 | by @Dmitry422)
* Display: **Сombining RGB Backlight mod** (by @quen0n) and original backlight support **in one firmware** (+ Rainbow/Wave effect (based on @Willy-JL idea)) (PR #877 #881 #890 | by @Dmitry422) - (**To enable RGB Backlight support go into Notifications settings**)
* NFC: Use default UL/UL-C pwd/key as default value for key input (PR #891 | by @mishamyte)
* OFW: LFRFID - **EM4305 support**
* OFW: **Universal IR signal selection**
* OFW: **BadUSB: Mouse control**
* OFW: **Pinning of settings options**
* OFW: NFC app now can launch MFKey32
* OFW: BadUSB arbitrary key combinations
* OFW PR 4136: BadUSB: Full USB/BLE parameter customization, UI improvements, and more (by @Willy-JL)
* OFW: NFC - Added naming for DESFire cards + fix MF3ICD40 cards unable to be read
* Apps: Add **FindMyFlipper to system apps and allow autostart** on system boot [app by @MatthewKuKanich](https://github.com/MatthewKuKanich/FindMyFlipper) and autoloader by @Willy-JL - to use app please check how to add keys in [app repo](https://github.com/MatthewKuKanich/FindMyFlipper)
* README Update: Enhanced Visuals & Navigation (PR #871 #872 | by @m-xim)
* Docs: Update FAQ.md (PR #865 | by @mi-lrn)
* Input: **Vibro on Button press option** (PR #867 | by @Dmitry422)
* Power: **Option to limit battery charging** (suppress charging on selected charge level) (PR #867 | by @Dmitry422) (idea and example by @oltenxyz)
* Apps: **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
## Other changes
* SubGHz: Move hardcoded extra modulations to user config - uncomment them in setting_user.example and remove .example from filename
* SubGHz: Various bugfixes and experimental options (rolling counter overflow) (by @xMasterX)
* Anims: Disable winter anims
* NFC: mfclassic poller fix early key reuse in dictionary attack state machine (by @noproto)
* OFW: RC fixes
* OFW: Desktop: Fix freeze on boot if PIN set
* OFW PR 4189: USB-UART bridge fix (by @portasynthinca3)
* OFW: FBT: Fix for Python 3.13
* OFW: sdk: bump API to force re-upload for the catalog 
* OFW: SDK: Fix missing RECORD_CLI define
* OFW: Fix NULL dereference in CLI completions
* OFW PR 4181: vcp, cli: Handle Tx/Rx events before Connect/Disconnect + extra fixes (by @portasynthinca3)
* OFW: BLE: Slightly increase mfg_data size
* OFW: fbt: Deterministic STARTUP order & additional checks
* OFW: JS: Update and fix docs, fix Number.toString() with decimals
* OFW: New JS value destructuring
* OFW: Docs: Fix doxygen references from PR 4168
* OFW: BLE advertising improvements
* OFW: **New CLI architecture**
* OFW: **CLI autocomplete and other sugar**
* OFW: CLI commands in fals and threads
* OFW: cli: fixed `free_blocks` command
* OFW: docs: badusb arbitrary modkey chains
* OFW: Separate cli_shell into toolbox
* OFW: Move JS modules to new arg parser
* OFW: Application chaining
* OFW: Fix DWARF dead code elimination and linking 
* OFW: NFC: Fix crash on ISO15693-3 save when memory is empty or cannot be read
* OFW: Reduced ieee754 parser size
* OFW: Added Doom animation (by @doomwastaken)
* OFW PR 4133: add nfc apdu cli command back (by @leommxj)
* OFW: NFC: Support DESFire Transaction MAC file type (by @Willy-JL)
* OFW: NFC: Fix NDEF parser for MIFARE Classic (by @Willy-JL)
* OFW: GUI: Fix widget text scroll with 256+ lines (by @Willy-JL)
* OFW: Infrared: Fix universals sending (by @Willy-JL)
* OFW: HID Ble: increased stack and improvements (by @doomwastaken)
* OFW: Stricter constness for const data (by @hedger)
* OFW PR 4017: Alarm improvements: Snooze, timeouts, and dismissing from the locked state (by @Astrrra)
* OFW: fix: flipper detected before it was rebooted
* OFW: NFC: FeliCa Protocol Expose Read Block API and Allow Specifying Service
* OFW: LFRFID: Fix Detection Conflict Between Securakey and Noralsy Format (by @zinongli)
* OFW: Stdio API improvements
* OFW: GUI: Widget view extra options for JS 
* OFW: Update heap implementation
* OFW: Updated Button Panel
* OFW: UART framing mode selection
* OFW: gpio: clear irq status before calling user handler
* OFW: Fix 5V on GPIO
* OFW: Fixed repeat in subghz tx_from_file command 
* OFW: LFRFID: Noralsy Format/Brand
* OFW: Faster di card reading
* OFW: vscode: disabled auto-update for clangd since correct version is in the toolchain
* OFW: Furi, USB, BLE, Debug: various bug fixes and improvements
* OFW: EventLoop unsubscribe fix
* OFW: nfc: Enable MFUL sync poller to be provided with passwords
* OFW: ST25TB poller mode check
* OFW: JS features & bugfixes (SDK 0.2) **Existing Widget JS module was removed and replaced with new ofw gui/widget module, old apps using widget may be incompatible now!**
* OFW: Infrared: increase max carrier limit
* OFW: Ensure that `furi_record_create` is passed a non-NULL data pointer
* OFW: Update mbedtls & expose AES
* OFW: Add the Showtime animation
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



