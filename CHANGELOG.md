### New changes
* Remove `apps` folder on your microSD before installing this release to avoid issues!
* SubGHz: **Custom buttons for Nice Flor S / Somfy Telis (+Programming mode)** - now you can use arrow buttons to send signal with different button code
* SubGHz: Somfy Telis -> Add manually (create new remote, now with programming button (Prog / 0x8) you can write it into receiver)
* SubGHz: BFT Mitto -> Add manually (create new remote, now with programming button (0xF) you can write it into receiver)
* SubGHz: Nice One -> Add manually (programming is possible using regular button)
* SubGHz: More precise settings for debug counter increase value
* Plugins -> MouseJacker: Features, Fixes and improvements (by @MatthisC | PR #366)
* Plugins -> HC-SR04: Improve accuracy by measuring microseconds (by @clashlab | PR #367)
* OFW: BadUSB UI fixes
* OFW: Plugins: move to designated categories -> **We moved some plugins to new categories too**
* OFW: Drivers: remove excessive check in bq25896 and make PVS happy
* OFW: FuriHal, Power, UnitTests: fix, rename battery charging voltage limit API -> **Breaking API change, api was changed from 14.x to 15.x** 
**(this will make your manually copied plugins not work, update them in same way you installed them, or delete `apps` folder and then install firmware, if you using extra pack builds (with `e` in version) all apps in _Extra will be updated automatically)**

* OFW: Fix incorrect type choise condition in image compressor
* OFW: Updater: handle storage errors when removing files, fix folder remove routine, prevent unused services from starting
* OFW: Unify power info, power debug, and device_info into one info command
* OFW: SD Cache: moved to diskio layer, invalidation in case of error
* OFW: Picopass: factory key support, minor code cleanup

#### [🎲 Download latest extra apps pack](https://download-directory.github.io/?url=https://github.com/xMasterX/unleashed-extra-pack/tree/main/apps)

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


