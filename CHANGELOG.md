## New changes
* Apps: **Mifare Nested - ported to latest API** using old nfc lib (by @xMasterX) (original app made by @AloneLiberty) (+ mem management fix by @Willy-JL) - [Python app running on PC is required](https://github.com/AloneLiberty/FlipperNestedRecovery)
* LFRFID: **Electra fix** non-initialized encoded epilogue on render (by @Leptopt1los)
* JS: Move examples to subfolder `js_examples`
* Apps: HID app improvements and fixes<br>
`- Move new mouse jiggler into mouse jiggler stealth and bring back previous version of mouse jiggler too`<br>
`- Set stealth jiggler max time default value to 4 min and min value to 1 min`<br>
`- Merge OFW changes`<br>
`- More OFW merge fixes` (by @Willy-JL | PR #753)<br>
* Apps: **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
* OFW (TLSF branch): SubGHz: fix memory corrupt in read raw view
* OFW: **NFC App: fix changing UID**
* OFW: Replaced obsolete-format delay
* OFW: **Archive: fix condition race on exit**
* OFW: Text Box: fix displaying text with end text focus
* OFW: FuriHal: add flash ops stats, workaround bug in SHCI_C2_SetSystemClock
* OFW: Icons: compression fixes & larger dimension support
* OFW: **Text Box rework**
* OFW: Fix calling both `view_free_model()` and `view_free()`
* OFW: JS: Add textbox module
* OFW: JS: Add math module
* OFW: **NFC: add Slix capabilities**
* OFW: Settings refactor fixes
* OFW: JS: Add submenu module
* OFW: **Skylanders plugin**
* OFW: Settings menu refactoring 
* OFW: NFC: Mf Desfire fix reading big files 
* OFW: Infrared: Add Toshiba RAS-2518D 
* OFW: **vscode: config fixes**
* OFW: Ble: new connection parameters negotiation scheme
* OFW: FuriHal: move version init to early stage 
* OFW: Add support for R_ARM_REL32 relocations.
* OFW: Remove unused DolphinWait_61x59 icon
* OFW: Add the Akira animation
* OFW: **Desktop: fix crash on autolock after restart in locked state**
<br><br>
#### Known NFC post-refactor regressions list: 
- Mifare Mini clones reading is broken (original mini working fine) (OFW)
- NFC CLI was removed with refactoring (OFW) (will be back soon)

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
@mishamyte, ClaraCrazy, Pathfinder [Count Zero cDc], callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
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



