## Main changes
- SubGHz:
    - **Novoferm** remotes full support
    - Fix Decode scene in RAW files
    - Add manually -> Add Sommer FM238 option for cases when default option doesn't work (named as Sommer fm2)
    - Remove broken preset modulation
    - Normstahl, Sommer, MHouse, Aprimatic -> Fixes for button codes and more in Add manually
    - Custom button improvements for MHouse, Novoferm, Nice Smilo
    - Hormann EcoStar -> Add manually support, and custom button support
    - Hormann HSM 44bit static -> Button code decoding fix
    - Choose RSSI threshold for Hopping mode (by @Willy-JL)
- NFC: 
    - OFW: Ultralight C authentication with des key
    - EMV Transactions less nested, hide if unavailable (by @Willy-JL | PR #771)
    - Update Mifare Classic default keys dict with new keys from proxmark3 repo and UberGuidoZ repo
- LF RFID: 
    - Update T5577 password list (by @korden32 | PR #774)
    - Add DEZ 8 display form for EM4100 (by @korden32 | PR #776 & (#777 by @mishamyte))
- JS: 
    - Refactor widget and keyboard modules, fix crash (by @Willy-JL | PR #770)
    - SubGHz module fixes and improvements (by @Willy-JL)
* OFW: Infrared: check for negative timings
* OFW: Fix iButton/LFRFID Add Manually results being discarded
* OFW: Event Loop Timers
* OFW: Updater: resource compression
* Apps: **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
## Other changes
* OFW: HID/BLE Keyboard UI refactoring
* OFW: CCID: Add CCIDWorker
* OFW: Disabled ISR runtime stats collection for updater builds
* OFW: VSCode fixes: .gitignore & clangd
* OFW: ufbt: synced .clang-format rules with main
* OFW: Code formatting update 
* OFW: scripts: runfap: fixed starting apps with spaces in path
* OFW: toolchain: v38. clangd as default language server
* OFW: NFC: ISO15693 Render Typo Fix
* OFW: tar archive: fix double free
* OFW: ufbt: added ARGS to commandline parser
* OFW: lib: sconscript todo cleanup
* OFW: Intruder animation
* OFW: Desktop: allow to close blocking bad sd animation
* OFW: Updater: reset various debug flags on production build flash (was done in same way in UL before)
* OFW: Fix PVS Warnings
* OFW: CCID: Improve request and response data handling
* OFW: Furi: count ISR time. Cli: show ISR time in top.
* OFW: toolchain: v37
* OFW: NFC: Cache plugin name not full path, saves some RAM (by @Willy-JL)
* OFW: copro: bumped to 1.20.0
* OFW: input_srv: Put input state data on the stack of the service
* OFW: Coalesce some allocations
* OFW: updater: slightly smaller image
* OFW: Updater: Fix double dir cleanup
* OFW: cli: storage: minor subcommand lookup refactor
* OFW: LFRFID Securakey: Add Support for RKKTH Plain Text Format
* OFW: NFC: Add mf_classic_set_sector_trailer_read function
* OFW: Separate editing and renaming in iButton and LFRFID
* OFW: New js modules documentation added 
* OFW: Update link to mfkey32
* OFW: NFC: Desfire Renderer Minor Debug 
* OFW: RPC: Fix input lockup on disconnect
* OFW: Thread Signals
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

### What `r`, `e`, ` `, `c` means? What I need to download if I don't want to use Web updater?
What build I should download and what this name means - `flipper-z-f7-update-(version)(r / e / c).tgz` ? <br>
`flipper-z` = for Flipper Zero device<br>
`f7` = Hardware version - same for all flipper zero devices<br>
`update` = Update package, contains updater, all assets (plugins, IR libs, etc.), and firmware itself<br>
`(version)` = Firmware version<br>
| Designation | [Base Apps](https://github.com/xMasterX/all-the-plugins#default-pack) | [Extra Apps](https://github.com/xMasterX/all-the-plugins#extra-pack) | ⚠️RGB mode* |
|-----|:---:|:---:|:---:|
| ` ` | ✅ |  |  |
| `c` |  |  |  |
| `e` | ✅ | ✅ |  |
| `r` | ✅ | ✅ | ⚠️ |

⚠️This is [hardware mod](https://github.com/quen0n/flipperzero-firmware-rgb#readme), works only on modded flippers! do not install on non modded device!

Firmware Self-update package (update from microSD) - `flipper-z-f7-update-(version).tgz` for mobile app / qFlipper / web<br>
Archive of `scripts` folder (contains scripts for FW/plugins development) - `flipper-z-any-scripts-(version).tgz`<br>
SDK files for plugins development and uFBT - `flipper-z-f7-sdk-(version).zip`



