### New changes
* API: Due to OFW changes API was updated to 14.x, extra pack was updated you can install it separatly or use build with extra pack included
* SubGHz: **Fixed bug in SubGHz HAL, now CC1101 shutdowns properly, (also this fixed unstable TX)**
* SubGHz: **Alutech AT-4N** encoder (support for sending signals) (by @assasinfil | PR #322)
* SubGHz: **KingGates Stylo 4k** encoder (support for sending signals) (by @assasinfil | PR #321)
* SubGHz: Added Debug Pin setting for protocol development and fixed debug (and sound) with internal module
* SubGHz: GUI Fixes
* SubGHz: Removed bugged Detect RAW feature, now its replaced with BinRAW
* Plugins: SubGHz Bruteforcer - Added support for Linear Delta-3 310MHz
* Plugins: Fix CTRL-SHIFT in mousejacker (by @notmarek | PR #316)
* Infrared: Update universal remote assets (by @amec0e) (PR #318)
* NFC: Add Mifare Classic keys (by @Bon-Jorik (scaff.walker) | PR #324)
* Misc: Playing games now affect Flipper's level
* OFW: Fixed typo in nfc_magic_scene_wrong_card.c 
* OFW: SubGhz: fix cc1101_read_fifo func
* OFW: feat: add missing `const` qualifiers
* OFW: **SubGhz: add protocol BinRAW (binarization of data quantized by the minimum correlated duration)**
* OFW: Picopass: show elite key used from dictionary 
* OFW: Firmware fixes and improvements for flashing via blackmagic
* OFW: fbt: building fap_dist for compact gh build; accessor: fixed for latest ibutton changes 
* OFW: Move CSN space to revent overflow
* OFW: **SubGhz: add protocol KingGates Stylo4k**
* OFW: **SubGhz: add protocol Nice One**
* OFW: **SubGhz: add protocol Alutech at-4n**
* OFW: **SubGhz: add DOOYA protocol**
* OFW: **SubGhz: add protocol "Linear Delta-3"**
* OFW: **SubGhz: Fix Raw write, add short duration filter setting**
* OFW: Update Missing SD Card icon from PR 2373
* OFW: SCons: do not include backup files in build
* OFW: Fix minor UI inconsistencies and bugs
* OFW: Allow use of any suitable pin for 1-Wire devices
* OFW: **SD over SPI improvements**
* OFW: Multitarget support for fbt (includes support for non released yet flipper hardware)
* OFW: Pin Reset
* OFW: nfc: Add mifare classic value block commands
* OFW: battery info temperature shown in C or F based on settings
* OFW: Script that can find programmer and flash firmware via it.
* OFW: **SPI Mem Manager C port** (You can use flipper as programmer to flash supported chips)

#### [🎲 Download latest extra apps pack](https://download-directory.github.io/?url=https://github.com/xMasterX/unleashed-extra-pack/tree/main/apps)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper (official link)](https://flipperzero.one/update)

## Please support development of the project
* Boosty: https://boosty.to/mmxdev
* Ko-Fi: https://ko-fi.com/masterx
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

### Thanks to our sponsors:
callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
and all other great people who supported our project and me (xMasterX), thanks to you all!

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

**Recommended option - Web Updater**

What means `n` or `e` in - `flipper-z-f7-update-(version)(n / e).tgz` ? - `n` means this build comes without our custom animations, only official flipper animations, 
`e` means build has extra apps pack preinstalled

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for mobile app / qFlipper


