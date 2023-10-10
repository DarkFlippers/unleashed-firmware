## New changes
**Summary: BLE Core2 (Copro) crashes should be fixed with this update, Apple BLE Spam app and other apps updated to the latest versions, NFC file sort crashes has been fixed, other fixes and improvements see below:**
* SubGHz: Nice Flor S - added custom button code 0x3
* NFC: Fixes out of memory crash if we open folder with more than 300 files in it
* LF RFID: Fixed logic in `t5577_write_with_pass` (by @baugp | PR #612)
* Infrared: Updated universal assets (by @amec0e | PR #607 #619)
* **Apple BLE Spam app** updated to latest version (by @Willy-JL) -> (app can be found in builds ` `, `e`, `n`, `r`)
* OFW: Fix spelling across some project files
* OFW: CCID: Support PC To Reader Transfer Block data
* OFW: Firmware: bigger thread name storage. Notification app: better BacklightEnforce edge cases handling. 
* OFW: Lib: update stm32wb_copro to 1.17.3 release
* OFW: FuriHal ble: length fix for fw version prop 
* OFW: add documentation SubGhz Bin_RAW file format
* OFW: fbt: glob improvements
* OFW: HEX input UI improvements
* OFW: Ble: fix null-ptr dereference in bt_change_profile
* OFW: Add the coding in the shell animation
* OFW: FuriHal,BleGlue: prevent sleep while HCI command executed, proper bt api rpc locking. Fixes random system lockups.
* OFW: fbt: reworked tool path handling
* OFW: Gui: handle view port lockup and notify developer about it
* OFW: Added `fal_embedded` parameter for PLUGIN apps
* OFW: Fix multiline aligned text going out of bounds (again)
* OFW: Add Initial CCID support 
* OFW: Add confirmation before exiting USB-UART
* OFW: Add extended I2C HAL functions -> **Breaking API change 38.x -> 39.x**
* OFW: New clock switch schema, **fixes random core2 crashes**

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

#### Thanks to our sponsors:
callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
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
| `r` | ✅ | ✅ | ✅ | ✅ |

⚠️This is [hardware mod](https://github.com/quen0n/flipperzero-firmware-rgb#readme), works only on modded flippers! do not install on non modded device!

Firmware Self-update package (update from microSD) - `flipper-z-f7-update-(version).tgz` for mobile app / qFlipper / web<br>
Archive of `scripts` folder (contains scripts for FW/plugins development) - `flipper-z-any-scripts-(version).tgz`<br>
SDK files for plugins development and uFBT - `flipper-z-f7-sdk-(version).zip`



