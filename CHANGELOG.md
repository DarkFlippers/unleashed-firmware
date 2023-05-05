### New changes 
* If you have copied any apps manually into `apps` folder - remove `apps` folder or that specific apps you copied on your microSD before installing this release to avoid issues!
* Plugins: RFID and iButton Fuzzer remove excessive free's (thanks @Willy-JL)
* Plugins: Use COUNT_OF in mouse jiggler
* Plugins: Added Numpad keyboard to HID app (by @clipboard1 | PR #452)
* About: Don't show 00 on about screens
* SubGHz: Combine FuriString allocs and other small changes
* Infrared: Updated universal remote assets (by @amec0e | PR #454)
* Update slideshow: Replace QR code with good old link
* OFW: Various Furi/FuriHal bug fixes and improvements -> **Breaking API change, api was changed from 24.x to 26.x** 
* OFW: Loader refactoring, part 1 -> **Breaking API change, api was changed from 23.x to 24.x** **(this will make your manually copied plugins not work, update them in same way you installed them, or delete `apps` folder and then install firmware, if you using extra pack builds (with `e` in version) all apps in _Extra will be updated automatically)**
* OFW: Dolphin builder in ufbt; minor ufbt/fbt improvements
* OFW: Added API version to device info 
* OFW: Gui: relax some asserts in view
* OFW: Move gauge calibration to separate header, add f18 calibration
* OFW: Fix TERMINFO on Linux systems 

#### [🎲 Download latest extra apps pack](https://github.com/xMasterX/all-the-plugins/archive/refs/heads/main.zip)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper (official link)](https://flipperzero.one/update)

## Please support development of the project
* **Boosty** (patreon alternative): https://boosty.to/mmxdev
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


