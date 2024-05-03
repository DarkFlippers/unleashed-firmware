## New changes
* LFRFID: **Electra intercom protocol support** (Romania) (by @Leptopt1los | PR #750)
* NFC: Temp fix for `iso14443_4_layer_decode_block` crash
* NFC: CharlieCard parser (by @zacharyweiss)
* SubGHz: FAAC RC XT - add 0xB button code on arrow buttons for programming mode
* SubGHz: Add Manually - Sommer FM fixes
* SubGHz: Enabled tx-rx state on unused gpio pin by default (**external amp option was removed and is enabled by default now**)
* SubGHz: **Status output !TX/RX on the GDO2 CC1101 pin** (by @quen0n | PR #742)
* SubGHz: Reworked saved settings (by @xMasterX and @Willy-JL)
* Desktop: Fixes for animation unload (by @Willy-JL)
* iButton: Updated DS1420 for latest ibutton changes
* Misc: Allow no prefix usage of name_generator_make_detailed_datetime
* Misc: Allow setting view dispatcher callbacks to NULL
* Misc: Added `void` due to `-Wstrict-prototypes`
* Misc: Some code cleanup and proper log levels in nfc parsers
* Infrared: Allow external apps to use infrared settings (by @Willy-JL)
* JS & HAL: Various fixes and FURI_HAL_RANDOM_MAX define added (by @Willy-JL)
* JS: **BadUSB layout support** (by @Willy-JL)
* JS: New Modules `widget`, `vgm` and path globals (by @jamisonderek)
* Apps: Enhance Random Interval and Movement Functionality in HID Mouse Jiggler for Improved Stealth and Human-Like Behavior (by @gushmazuko | PR #746)
* Apps: NFC Magic - **Gen2 writing support, Gen4 NTAG password and PACK fixes** (by @Astrrra)
* Apps: MFKey - **fixed crashes**, add more free ram (by @noproto & @Willy-JL) 
* Apps: **Check out Apps updates by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
* OFW PR 3616: NFC: Mf Desfire fix reading big files (by gornekich)
* OFW: iButton: fix crash when deleting some keys
* OFW: Desktop: cleanup error popups
* OFW: Troika parser visual fixes 
* OFW: Fix the retry/exit confirmation prompts in iButton
* OFW: nfc app: add legacy keys for plantain cards
* OFW: GUI: Fix array out of bounds in menu exit 
* OFW: add support for S(WTX) request in iso14443_4a_poller
* OFW: Mosgortrans parser output fixes
* OFW: BLE: Add GapPairingNone support 
* OFW: iButton new UI 
* OFW: FuriHal: add ADC API
* OFW: Mf Desfire multiple file rights support
* OFW: **Felica poller** (NFC-F)
* OFW: Desktop/Loader: Unload animations before loading FAPs
* OFW: JS Documentation
* OFW: **Update radio stack to v1.19.0**
* OFW: **Move crypto1 to helpers, add it to the public API**
* OFW: Explain RNG differences, add FURI_HAL_RANDOM_MAX
* OFW: Furi: Add "out of memory" and "malloc(0)" crash messages
* OFW: IR: Fix crash on duty_cycle=1
* OFW: **Desktop: ensure that animation is unloaded before app start (fixes some out of memory crashes)**
* OFW: Hide unlock with reader for MFU-C 
* OFW: fbt: fixed missing FBT_FAP_DEBUG_ELF_ROOT to dist env
* OFW: fbt: added -Wstrict-prototypes for main firmware
* OFW: Mifare Ultralight naming fix 
* OFW: IR: Remember OTG state
* OFW: Bad USB: fix crash when selecting a keyboard layout
* OFW: L1_Mods animation update : adding VGM visual 
* OFW: RFID Improvements 
* OFW: Fixed plugins and UI 
* OFW: **NFC: Fix mf desfire detect**
* OFW: infrared_transmit.h was missing `#pragma once`
* OFW: Show the wrong PIN Attempt count on the login screen
* OFW: SavedStruct: Introduce saved_struct_get_metadata
* OFW: JS CLI command
* OFW: Add ChromeOS Bad USB demo
* OFW: **Configurable Infrared TX output** (previous UL version is replaced with OFW version, new features added "AutoDetect" and saving settings)
* OFW: BadUSB: BLE, media keys, Fn/Globe key commands
* OFW: NFC: Slix privacy password reveal ->(was included in previous UL release) and **Desfire detect fix**
* OFW: github: additional pre-upload checks for doxygen workflow
* OFW: NFC UI fixes
* OFW: Gui: unicode support, new canvas API
* OFW: **Api Symbols: replace asserts with checks**
<br><br>
#### Known NFC post-refactor regressions list: 
- Mifare Mini clones reading is broken (original mini working fine) (OFW)
- NFC CLI was removed with refactoring (OFW) (will be back soon)
- Mifare Nested not ported to latest API yet, `unlshd-065` is the latest version on old NFC API that works with "nested app"

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
|ETH|(BSC/ERC20-Tokens)|`darkflippers.eth` (or `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`)|
|BTC||`bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`|
|SOL|(Solana/Tokens)|`DSgwouAEgu8iP5yr7EHHDqMNYWZxAqXWsTEeqCAXGLj8`|
|DOGE||`D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`|
|LTC||`ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`|
|BCH||`qquxfyzntuqufy2dx0hrfr4sndp0tucvky4sw8qyu3`|
|XMR|(Monero)| `41xUz92suUu1u5Mu4qkrcs52gtfpu9rnZRdBpCJ244KRHf6xXSvVFevdf2cnjS7RAeYr5hn9MsEfxKoFDRSctFjG5fv1Mhn`|
|TON||`UQCOqcnYkvzOZUV_9bPE_8oTbOrOF03MnF-VcJyjisTZmsxa`|

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



