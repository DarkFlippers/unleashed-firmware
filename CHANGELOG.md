## Warning!!! Please read this before installing!!!
**This release has some unresolved issues, if any of those affects your daily usage, stay at 065 release or wait for next releases:** <br>
**Issues from this list will be fixed in next releases**
### Known NFC app regressions and issues: 
- Mifare Classic with custom UID add manually option was temporarily removed (Unleashed)
- Mifare Mini clones reading is broken (original mini working fine) (OFW)
- Mifare Classic dict attack fast skip (multiple presses on OK button) causes glitches/incorrect reading (OFW)
- EMV simple data parser was removed with protocol with refactoring (OFW)
- Option to unlock Slix-L (NFC V) with preset or custom password was removed with refactoring (OFW)
- NFC CLI was removed with refactoring (OFW)
### Some apps that was made for old nfc stack is now not compatible with the new API and require complete remake:
**If you want to help with making this apps work again please send PR to the repo at link below**
- Current list of affected apps: https://github.com/xMasterX/all-the-plugins/tree/dev/apps_broken_by_last_refactors
- Also in app **Enhanced Sub-GHz Chat** - NFC part was temporarily removed to make app usable, NFC part of the app requires remaking it with new nfc stack <br>
**API was updated to v50.x** 
## New changes
* IR: Updated infrared assets (by @amec0e | PR #677)
* NFC: Fix Saflok edge case 0.5% of UIDs got wrong result (by @noproto | PR #668)
* NFC: Zolotaya Korona transport card parser added (by @Leptopt1los)
* NFC: Parsers cleanup for new api (by @Leptopt1los)
* SubGHz: Temp fix for subghz keyboard lock display issue (furi_timer is not working properly)
* SubGHz: Added new option to delete old signals on full memory
* SubGHz: Faac rc/xt add manually (unverified)
* SubGHz: Better subghz history element removal (by @Willy-JL)
* SubGHz: Fix key display newline issue in came atomo
* Apps: **Check out Apps updates by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
* OFW: USART Bridge: added support for software control of DE/RE pins
* OFW: ufbt: changed toolchain environment invocation; updated .gitignore for app template
* OFW: Keys Dict: fix PVS warnings
* OFW: NfcDict Refactoring
* OFW: Add AC's Carrier 42QG5A580SC and AUX YKR-H/006E
* OFW: NFC Plugins loading rework
* OFW: MFC emulation fix
* OFW: nfc_util: little endian bytes2num functions added
* OFW: Add MyKey parser
* OFW: Update CLI MOTD
* OFW: NFC NTAG and ISO14443-3b reading fix
* OFW: FuriHal: RTC register reset API. New factory reset routine that wipes all RTC backup registers content.
* OFW: FuriHal: various GPIO improvements
* OFW: SubGhz: changed the name of the button when sending RAW to SubGHz

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



