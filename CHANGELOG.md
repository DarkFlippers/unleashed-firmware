## Warning!!! Please read this before installing!!!
**This release has some unresolved issues, if any of those affects your daily usage, stay at 065 release or wait for next releases:** <br>
**Issues from this list will be fixed in next releases**
### Known NFC app regressions and issues: 
- Mifare Classic with custom UID add manually option was temporarily removed (Unleashed)
- Mifare Mini clones reading is broken (original mini working fine) (OFW)
- Mifare Classic dict attack fast skip (multiple presses on OK button) causes glitches/incorrect reading (OFW)
- EMV simple data parser was removed with protocol with refactoring (OFW)
- Mifare Classic Emulation slow response (unconfirmed) (OFW)
- Option to unlock Slix-L (NFC V) with preset or custom password was removed with refactoring (OFW)
- NFC CLI was removed with refactoring (OFW)
### Some apps that was made for old nfc stack is now not compatible with the new API and require complete remake:
**If you want to help with making this apps work again please send PR to the repo at link below**
- Current list of affected apps: https://github.com/xMasterX/all-the-plugins/tree/dev/apps_broken_by_last_refactors
- Also in app **Enhanced Sub-GHz Chat** - NFC part was temporarily removed to make app usable, NFC part of the app requires remaking it with new nfc stack <br>
**API was updated to v49.x** 
## New changes
* NFC: Added new parsers for transport cards - Umarsh, Kazan, Moscow, Metromoney(Tbilisi), and fixes for OFW parsers (by @assasinfil and @Leptopt1los) (special thanks for users who provided various dumps of those cards for research)
* NFC: Added simple key name display to UI to fix regression
* NFC: Add keys to mf_classic_dict (by @hnlcory | PR #660)
* NFC: Add Saflok and MyKey KDFs (by @noproto | PR #662)
* NFC: social_moscow parser verification collisions fix (by @Leptopt1los)
* iButton: Fix UI text - protocol name getting out of screen bounds when key name is too large, and other related issues (by @krolchonok | PR #649)
* SubGHz: Fixed feature naming in menu
* SubGHz: Added honeywell protocol [(by @htotoo)](https://github.com/Flipper-XFW/Xtreme-Firmware/commit/ceee551befa0cb8fd8514a4f8a1250fd9e0997ee)
* SubGHz: Add 303.9 Mhz to default frequency list
* SubGHz: Fix Keeloq decoding order bug (random switch to HCS101 or anmotors)
* SubGHz: Fix secplus v1 key display issue
* API: Add new get function for varitemlist (by @Willy-JL)
* Misc code cleanup
* Apps: **Bluetooth Remote / USB Keyboard & Mouse** - `Movie` and `PTT` modes by @hryamzik
* Apps: **BLE Spam app** updated to latest version (New devices support, + Menu by holding Start) (by @Willy-JL) -> (app can be found in builds ` `, `e`, `n`, `r`)
* Apps: **NFC Magic** - Gen4 Actions (option to fix card with broken config) (by @Leptopt1los and @xMasterX)
* Apps: **Check out Apps updates by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
* OFW: NFC fixes
* OFW: nfc: m1k-based Aime (non-AIC) card support 
* OFW: SubGhz: fix count bit for detect gate_tx protocol 
* OFW: Fixed a zero allocation error when reading an iso15693 nfc tag with no additional blocks. 
* OFW: Ntag21x write
* OFW: Mifare Classic nested auth support
* OFW: ST25TB poller refining + write support
* OFW: Libraries cleanup; u2f crypto rework to use mbedtls
* OFW: Add the secret door animation
* OFW: Allows you to use UCS-2 in canvas_glyph_width
* OFW: Mifare Classic fixes
* OFW: NFC: Felica UID emulation
* OFW: 64k does not enough
* OFW: fbt: improvements
* OFW: Various Fixes for 0.95
* OFW: Add Mastercode SubGHz Protocol
* OFW: Do not remove file when renaming to itself
* OFW: Fix iButton crash on missing file
* OFW: NFC API improvements
* OFW: MF Ultralight no pwd polling adjustment
* OFW: Fix limited_credit_value having wrong value in mf_desfire_file_settings_parse
* OFW: Infrared remote button index support
* OFW: Fix NFC unit tests
* OFW: fix: invariant format of log time data
* OFW: fbt: dist improvements
* OFW: Fix crash when exiting write mode
* OFW: Dolphin: Extreme butthurt loop fix
* OFW: **Furi, FuriHal: remove FreeRTOS headers leaks**
* OFW: fbt: source collection improvements
* OFW: Rename menu items related to dummy mode and sound
* OFW: fbt: SD card resource handling speedup
* OFW: **Furi: cleanup crash use**
* OFW: Allow for larger Infrared remotes 
* OFW: **fbt: reworked assets & resources handling**
* OFW: Storage: speedup write_chunk cli command
* OFW: fix crash after st25tb save
* OFW: Fix crash when reading files > 64B
* OFW: NFC RC fixes
* OFW: Fix MF DESFire record file handling
* OFW: **NFC refactoring** (new NFC stack) -> some apps still require very big changes to make them work with new system - see apps that was temporarily removed from this release here: https://github.com/xMasterX/all-the-plugins/tree/dev/apps_broken_by_last_refactors
* OFW: fbt: glob & git improvements
* OFW: FastFAP: human readable error log 

----

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper (official link)](https://flipperzero.one/update)

## Please support development of the project
|Service|Remark|Link/Wallet|
|-|-|-|
|**Patreon**||https://patreon.com/mmxdev|
|**Boosty**|patreon alternative|https://boosty.to/mmxdev|
|cloudtips|only RU payments accepted|https://pay.cloudtips.ru/p/7b3e9d65|
|YooMoney|only RU payments accepted|https://yoomoney.ru/fundraise/XA49mgQLPA0.221209|
|USDT|(TRC20)|`TSXcitMSnWXUFqiUfEXrTVpVewXy2cYhrs`|
|BCH||`qquxfyzntuqufy2dx0hrfr4sndp0tucvky4sw8qyu3`|
|ETH|(BSC/ERC20-Tokens)|`darkflippers.eth` (or `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`)|
|BTC||`bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`|
|DOGE||`D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`|
|LTC||`ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`|
|XMR|(Monero)| `41xUz92suUu1u5Mu4qkrcs52gtfpu9rnZRdBpCJ244KRHf6xXSvVFevdf2cnjS7RAeYr5hn9MsEfxKoFDRSctFjG5fv1Mhn`|
|TON||`EQCOqcnYkvzOZUV_9bPE_8oTbOrOF03MnF-VcJyjisTZmpGf`|

#### Thanks to our sponsors who supported project in the past and special thanks to sponsors who supports us on regular basis:
ClaraCrazy, Pathfinder [Count Zero cDc], callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
and all other great people who supported our project and me (xMasterX), thanks to you all!


## **Recommended update option - Web Updater**

### What `n`, `r`, `e`, ` `, `c` means? What I need to download if I don't want to use Web updater?
What build I should download and what this name means - `flipper-z-f7-update-(version)(n / r / e / c).tgz` ? <br>
`flipper-z` = for Flipper Zero device<br>
`f7` = Hardware version - same for all flipper zero devices<br>
`update` = Update package, contains updater, all assets (plugins, IR libs, etc.), and firmware itself<br>
`(version)` = Firmware version<br>
| Designation | 3 Custom Animation | [Base Apps](https://github.com/xMasterX/all-the-plugins#default-pack) | [Extra Apps](https://github.com/xMasterX/all-the-plugins#extra-pack) | ⚠️RGB mode* |
|-----|:---:|:---:|:---:|:---:|
| ` ` | ✅ | ✅ |  |  |
| `c` | ✅ |  |  |  |
| `n` |  | ✅ |  |  |
| `e` | ✅ | ✅ | ✅ |  |
| `r` | ✅ | ✅ | ✅ | ⚠️ |

⚠️This is [hardware mod](https://github.com/quen0n/flipperzero-firmware-rgb#readme), works only on modded flippers! do not install on non modded device!

Firmware Self-update package (update from microSD) - `flipper-z-f7-update-(version).tgz` for mobile app / qFlipper / web<br>
Archive of `scripts` folder (contains scripts for FW/plugins development) - `flipper-z-any-scripts-(version).tgz`<br>
SDK files for plugins development and uFBT - `flipper-z-f7-sdk-(version).zip`



