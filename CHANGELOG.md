### New changes
* If you have copied apps into `apps` folder - remove `apps` folder on your microSD before installing this release to avoid issues!
* SubGHz: Default custom buttons layout for non standard remotes (for example your remote has broken buttons and transmit only 0xC, now you can use other buttons)
* SubGHz: Fix issues with external module 5v power (now all works automatically, enabling +5v manually not required) (**Only for modules that work with 5v->3.3v converter!!!!!**)
* SubGHz: Option to disable automatic 5v power for external modules - (5v is enabled by default, if you are using module without converter you can set this option to OFF)
* SubGHz: Fix and update subghz protocols to use new error system
* SubGHz: Fix default frequency being overwritten bug (Add manually fixes)
* SubGHz: Fix 464Mhz and (390MHz for external module only) was showing up in Frequency analyzer all the time due to noise
* iButton: Fix ibutton app - add manually - duplicate names
* Plugins: NFC Magic fix - reinit nfc at app start
* Plugins: Update **Unitemp - Temperature sensors reader** (DHT11/22, DS18B20, BMP280, HTU21x and more) [(by quen0n)](https://github.com/quen0n/unitemp-flipperzero)
* Plugins: Update **SWD Probe** [(by g3gg0)](https://github.com/g3gg0/flipper-swd_probe)
* Plugins: Massive plugins refactoring - not full refactoring, only small issues is fixed and moved all plugins to furi mutex instead of valuemutex
* Plugins: Many small issues was found and fixed due mutex upgrade
* Plugins: `Extra pack` updated and fixed (valuemutex to furi_mutex upgrade)
* Plugins: SubGHz playlist - rewind (skip or play previous file) [(by alvarotorijano)](https://github.com/alvarotorijano/playListMod/blob/main/playlistMod.c)
* Plugins: Properly rename unirf remix to subghz remote - And automatically migrate user files to new folder (unirf -> subghz_remote)
* Plugins: Fix unirf freeze (protocol deserialize status ok) (by @Willy-JL | PR #375)
* Plugins: Blackjack game: fix bug counting more than one ace (by @403-Fruit | PR #374)
* Plugins: Update POCSAG Pager app to new error system
* Plugins: Update iButton Fuzzer to new iButton system
* Infrared: Update universal remote assets (by @amec0e | PR #378)
* OFW: PicoPass: auth cleanup
* OFW: More UI fixes and improvements
* OFW: NFC: Support reading Mifare Classic key B from sector trailer, reading sector with B key where A key can't read block,  Nfc Magic app not using NFC folder by default (in file select) 
* OFW: Remove ValueMutex -> **Breaking API change, api was changed from 17.x to 18.x** 
* OFW: Support reseting iCx cards
* OFW: Fixed picopass load save file overrun
* OFW: Fix SD card CID parsing
* OFW: Archive browser: update path on dir leave 
* OFW: SubGhz: better and more verbose error handling in protocols, stricter CAME validation -> **Breaking API change, api was changed from 16.x to 17.x** 
* OFW: iButton system and app refactoring (+new protocols) -> **Breaking API change, api was changed from 15.x to 16.x** 
**(this will make your manually copied plugins not work, update them in same way you installed them, or delete `apps` folder and then install firmware, if you using extra pack builds (with `e` in version) all apps in _Extra will be updated automatically)**

* OFW: New pin reset splashscreen
* OFW: Getter for application data path

#### [🎲 Download latest extra apps pack](https://github.com/xMasterX/unleashed-extra-pack/archive/refs/heads/main.zip)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper (official link)](https://flipperzero.one/update)

## Please support development of the project
* Boosty: https://boosty.to/mmxdev
* Ko-Fi: https://ko-fi.com/masterx
* cloudtips (only RU payments accepted): https://pay.cloudtips.ru/p/7b3e9d65
* YooMoney (only RU payments accepted): https://yoomoney.ru/fundraise/XA49mgQLPA0.221209
* USDT(TRC20): `TSXcitMSnWXUFqiUfEXrTVpVewXy2cYhrs`
* BCH: `qquxfyzntuqufy2dx0hrfr4sndp0tucvky4sw8qyu3`
* ETH/BSC/ERC20-Tokens: `darkflippers.eth` (or `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`)
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`
* XMR (Monero): `41xUz92suUu1u5Mu4qkrcs52gtfpu9rnZRdBpCJ244KRHf6xXSvVFevdf2cnjS7RAeYr5hn9MsEfxKoFDRSctFjG5fv1Mhn`
* TON: `EQCOqcnYkvzOZUV_9bPE_8oTbOrOF03MnF-VcJyjisTZmpGf`

### Thanks to our sponsors:
callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
and all other great people who supported our project and me (xMasterX), thanks to you all!

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

**Recommended option - Web Updater**

What means `n` or `e` in - `flipper-z-f7-update-(version)(n / e).tgz` ? - `n` means this build comes without our custom animations, only official flipper animations, 
`e` means build has extra apps pack preinstalled

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for mobile app / qFlipper


