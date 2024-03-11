## New changes
* NFC: EMV Fixes and imporvements (by @wosk & @Leptopt1los | PR #702)
* NFC: Parsers refactoring (by @Leptopt1los)
* NFC: Kazan parser improved - token parse option added (by @Leptopt1los)
* NFC: Update ndef parser, mf classic dict changes (by @Willy-JL)
* Infrared: Update universal remote assets (by @amec0e | PR #718 #719)
* SubGHz: Add 430.50 mhz (by @MizumasuShoichi | PR #721)
* SubGHz: Magellan Event Code Update (by @wooferguy | PR #713)
* SubGHz: Reduce subghz add manually scene flash size (by @Willy-JL)
* SubGHz: Fix led blink on decode raw > signal info (by @Willy-JL)
* HID App: apply fix for ms teams on macos (by @cpressland)
* HID App: merge official fw hid app keyboard changes
* Expansion `is_connected` API to check for VGM (by @HaxSam)
* New JavaScript Modules `UsbDisk`,`badusb.quit()`,`SubGHz`,`Submenu`,`BleBeacon`,`Keyboard`,`Math` (by @Willy-JL, @Spooks4576, @Sil333033)
* Apps: **Check out Apps updates by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
* OFW: Fix troika 4K keys
* OFW: Archive: Fix item focus after aborting the Delete operation
* OFW: Troyka parser improvements (by UL Team)
* OFW: NFC: Fix washcity plugin verify function being to greedy
* OFW: Parser for Santiago, Chile BIP transit card
* OFW: WiFi board: fixed update script on Windows (unfortunately also Mac and Linux)
* OFW: Gui: reset canvas orientation and frame when entering direct draw mode
* OFW: FBT/uFBT: Enable C++20/GNU23 in VSCode IntelliSense
* OFW: Toolchain fixes
* OFW: Quote $FBT_TOOLCHAIN_PATH to avoid splitting
* OFW: ble: profile rework
* OFW: lfrfid/em4100: added support for different bit rates (16clk was added back into UL, still not reading properly)
* OFW: T5577 lib: write with mask function added
* OFW: Archive: fixed Apps tab ext filter
* OFW: FuriHalRtc refactor: new datetime lib (by UL Team)
* OFW: bit_lib and nfc_util refactor (by UL Team)
* OFW: Gui text box: fix formatted string memory reservation
* OFW: JS debug disabled, archive and file browser fixes
* OFW: VSCode integration fixes for new toolchain
* OFW: FIX ISO15693 emulation
* OFW: JS serial module renamed, uart channel selection
* OFW: mjs: minor fixes
* OFW: **JavaScript runner**
* OFW: Fixed MyKey check LockID
* OFW: Check universal remote files before loading
* OFW: NFC: fix retry scene navigation logic 
* OFW: Expansion module service improvements
* OFW: New toolchain with gcc 12 (+ aarch64 support!)
* OFW: HID app: keyboard modifiers fix
* OFW: CLI: cat command crash workaround
* OFW: NFC: Custom UID entry when adding manually
* OFW: Added NFC plugin; Some parser
* OFW: **Slix disable privacy** (Unlock SLIX-L)
* OFW: NFC: Add support for Gallagher access control (MIFARE Classic only)
* OFW: furi/core/timer: resolve timer handle use-after-free post deletion
* OFW: FuriHal: various GPIO improvements
* OFW: GUI: canvas commit callback has been moved to canvas. Direct Draw apps can now be streamed via RPC.
* OFW: nfc app: fix incorrect protocol detection in save scene (by UL Team)
* OFW: NFC: MFC Unlock with Dictionary
* OFW: ITSO Parser (UK)
* OFW: NFC: fix application opening from browser
* OFW: Rework more info scene for Ultralight cards
* OFW: NFC UI refactor 
* OFW: Add an NFC parser for the San Francisco Bay Area "Clipper" transit card.
* OFW: Fix nfc_protocol_support_scene_save_name_on_event crash
* OFW: NFC: Display unread Mifare Classic bytes as question marks 
* OFW: Troika layout fixes
* OFW: NFC: MF Classic parsers read() fix (dictionary attack skip)
<br><br>
#### Known NFC post-refactor regressions list: 
- Mifare Mini clones reading is broken (original mini working fine) (OFW)
- NFC CLI was removed with refactoring (OFW) (will be back soon)
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



