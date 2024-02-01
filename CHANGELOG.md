## New changes
* NFC: **Fixed stuck Saved success screen**
* NFC: **Fixed crash when reading mifare classic tag then going to add manually menu and adding NFC-A tag**
* NFC: Fixed EMV txs render
* NFC/LFRFID: Don't Stop emulation after 5 mins to avoid antenna damage if debug is ON (by @Leptopt1los)
* LFRFID: Fixed T5577 custom password input (by @Leptopt1los)
* OFW PR 3410: lfrfid/em4100: added support for different bit rates - by @Mrkvak (RF/32 full support, RF/16 support without reading (16clk removed for now))
* OFW PR 3412: Fixed MyKey LockID - by @zProAle
<br>----<br>
**Changes from 070 release:**<br>
* NFC: **EMV parser** added (by @Leptopt1los and @wosk | PR #700)
* NFC: Metromoney parser balance fix (by @Leptopt1los | PR #699)
* NFC/LFRFID: Stop emulation after 5 mins to avoid antenna damage (by @Leptopt1los)
* Archive: Fix two filebrowser bugs
* SubGHz: **Programming mode for Dea Mio** (right arrow button)
* SubGHz: **Keeloq fix emulation for multiple systems and extend add manually support** for 2 of them (Dea Mio, Genius Bravo, GSN, Normstahl)
* SubGHz: Fixed hopper state when entering Read via Freq analyzer
* SubGHz: Raw erase fixes (by @Willy-JL)
* SubGHz: Subghz save files with receive time (by @Willy-JL)
* NFC: Fix NFC V dumps with v3 (pre refactor saves) crashing at info page
* NFC: Zolotaya Korona Online parser added (by @Leptopt1los)
* NFC: Add NFC **NDEF parser** (by @Willy-JL)
* LF RFID: **Write T5577 with random and custom password** added (clear password via Extra actions) (by @Leptopt1los)
* SubGHz: Update honeywell protocol (by @Willy-JL)
* System: More contrast values for replacement displays (up to +8 or -8)
* USB/BLE HID: Add macOS Music app volume control
* Apps: **Check out Apps updates by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
* OFW PR 3384: NFC: Display unread Mifare Classic bytes as question marks - by @TollyH
* OFW PR 3396: NFC: **fix application opening from browser** - by @RebornedBrain (+ fix for leftover issues)
* OFW PR 3382: NFC UI refactor - by @RebornedBrain
* OFW PR 3391: Rework more info scene for Ultralight cards - by @RebornedBrain
* OFW PR 3401: it-IT-mac layout - by @nminaylov
* OFW: Fix expansion protocol crash when fed lots of garbage
* OFW: 0.98.0-rc various fixes
* OFW: RFID CLI: better usage
* OFW: **Mf DESFire fixes**
* OFW: NFC UI refactor
* OFW: **Expansion module protocol** (+ expansion settings read and store in ram by @Willy-JL) 
* OFW: Bugfix: Strip last parity bit from decoded FDX-B data
* OFW: FuriHal: interrupt priorities and documentation
* OFW: FuriHal: **UART refactoring**
* OFW: SubGhz: add `subghz tx_from_file` CLI cmd, major TX flow refactoring, various improvements and bug fixes
* OFW: Furi_hal_rtc: new function
* OFW: NFC UI refactor 
* OFW: assets: checking limits on image size; ufbt: cdb target
* OFW: NFC: system dict skip when user dict is skipped fix (replaces our fix)
* OFW: FuriHal: fix start duration furi_hal_subghz_async_tx
* OFW: NFC: parsers minor cleanup
* OFW: NFC Ntag success write freeze when not saved card
* OFW: ufbt: fixed generated project paths on Windows 
<br><br>
#### Known NFC post-refactor regressions list: 
- Mifare Mini clones reading is broken (original mini working fine) (OFW)
- Option to unlock Slix-L (NFC V) with preset or custom password was removed with refactoring (OFW)
- NFC CLI was removed with refactoring (OFW)
- Current list of affected apps: https://github.com/xMasterX/all-the-plugins/tree/dev/apps_broken_by_last_refactors
- Also in app **Enhanced Sub-GHz Chat** - NFC part was temporarily removed to make app usable, NFC part of the app requires remaking it with new nfc stack

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



