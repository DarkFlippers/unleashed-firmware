## Main changes
- SubGHz:
    - Add new protocol - legrand 18bit (by @user890104)
    - OFW: Princeton protocol add custom guard time support
- NFC:
    - Fix Mifare DESFire reading (revert of buffer check workaround for rare emv cases)
    - OFW: Mifare Plus detection support 
    - OFW: Felica emulation
    - OFW: Write to ultralight cards is now possible (no UID writing)
    - OFW: Fixed infinite loop in dictionary attack scene
* LF RFID: OFW: Added Support for Securakey Protocol
* JS: `adc` support in `gpio` module (by @jamisonderek)
* BadUSB: Add Finnish keyboard layout (by @nicou | PR #761)
* Apps: **Check out more Apps updates and fixes by following** [this link](https://github.com/xMasterX/all-the-plugins/commits/dev)
## Other changes
* Misc: Revert USB CDC changes to fix usb serial
* Misc: Fix usage of deprecated `icon_get_data`
* NFC: Fix typo in parsers
* Apps: Fix `input_callback` and `timer_callback` usage of non `void` argument as input
* OFW: Cli: top 
* OFW: Desktop lockup fix, GUI improvements
* OFW: Loader: fix crash on "locked via cli loader"
* OFW: SubGhz: fix navigation GUI
* OFW: Furi: event loop
* OFW: Code Cleanup: unused includes, useless checks, unused variables, etc...
* OFW: SubGhz: fix gui "No transition to the "Saved" menu when deleting a SubGHz RAW file"
* OFW: RPC: Add TarExtract command, some small fixes
* OFW: Use static synchronisation primitives
* OFW: cleanup of various warnings from clangd
* OFW: Add initial ISO7816 support
* OFW: fbt, vscode: tweaks for cdb generation for clangd
* OFW: Updater: fix inability to update with bigger updater.bin
* OFW: Furi: wrap message queue in container, prepare it for epoll. Accessor: disable expansion service on start.
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



