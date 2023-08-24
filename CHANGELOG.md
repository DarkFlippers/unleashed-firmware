## New changes
* !!! **Warning! After installing, Desktop settings (Favoutite apps, PIN Code, AutoLock time..) will be resetted to default due to settings changes, Please set your PIN code, Favourite apps again in Settings->Desktop** !!!
* Desktop: **New way to set favourite apps and fully configurable dummy mode** (now you can set up to 4 favourite apps!) (port of OFW PR 2972 by nminaylov) (by @gid9798 | PR #578)
* Desktop: Fix lock timer after rebooting (by @gid9798 | PR #578)
* Infrared: Updated universal assets (by @amec0e | PR #581)
* Core: Added proper error message on out of memory crash (by @Willy-JL)
* SubGHz: Fix FAAC SLH add manually issues and fix sending signals with unknown seed
* SubGHz: Temporarily reverted changes from OFW PR 2984: SubGhz: fix todo (by Skorpionm) - Fixes Enhanced Sub-GHz Chat app and various issues related to receiving signals that was found in 061 release

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



