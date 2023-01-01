### New changes
* CI/CD: Builds with extra apps pack included, see new option below
* SubGHz: Allow manual creation of Nice Flor S, Nice Smilo
* LF RFID: Allow removing password from T5577 to make it operational (PR #225 | by @TQMatvey & Tobirg (grvpvl))
* Plugins -> Barcode Generator: Save last used barcode, and load on app start
* Plugins -> Barcode Generator: Add EAN-13 support (PR #234 | by @gid9798)
* Plugins -> Arkanoid: Fix bug with unability to continue game after first level
* Plugins -> Morse Code: Fixed crashes, fixed known issues (PR #249 | by @gid9798)
* Plugins -> USB Keyboard & Mouse: Added ESCAPE key (PR #243 | by @huuck)
* Plugins -> USB Keyboard & Mouse: Added Delete key
* Plugins -> SubGHz Bruteforcer: Add holtek HT12X protocol support
* Plugins -> POCSAG Pager: Fix UI bug
* Plugins -> Wifi Marauder: Enable +5volt on plugin start, to allow usage with custom boards
* Plugins: Add 2048 Game (PR #233 | by @eugene-kirzhanov)
* Plugins: Update TOTP, UniTemp, Wifi marauder to latest commits from their repo's
* Infrared: Universal remote assets updates (by @amec0e | PRs #246 #242)
* Readme fixes (PR #247 | by @Kavitate)
* New Year update slideshow (by @Svaarich)
* Show OTP flipper region on debug and about screens
* OFW: Add float_tools to SDK api
* OFW: Gui: change data ownership model in submenu, own text by default 
* OFW: Gui: Direct Draw API
* OFW: Mifare dictionary attack performance improvements.
* OFW: SubGhz: CAME 12 bit encoder fix guard time
* OFW: Fix MFC bruteforce progress bar 
* OFW: Docs and readme's updates
* OFW: Picopass read bug fixes
* OFW: OpenOCD scripts
* OFW: New MFC Bruteforce animation
* OFW: File browser: Empty folder label
* OFW: SubGhz: add Holtek_ht12x protocol
* OFW: USB/BLE HID Remote icon fix
* OFW: Fix quoted error for macOS bad-usb
* OFW: Modules: locking view model
* OFW: Fix PVS-Studio warnings 
* OFW: Fix unit tests
* OFW: WeatherStation: fix incorrect history index increment
* OFW: File format docs: RFID, iButton, BadUSB
* OFW: SubGhz: fix start navigation
* OFW: iButton: fixed notification on successful read 
* OFW: Archive browser delete fix
* OFW: Fixes: correct scrolling text
* OFW: SubGhz: fix Hormann HSM
* OFW: Rework BLE key storage
* OFW: Gui: scrollable long file names in FileBrowser and Archive Browser
* OFW: Untangle NFC_APP_FOLDER from nfc_device
* OFW: WS: add choice fahrenheit/celsius (can be set from flipper settings -> system)

#### [🎲 Download latest extra apps pack](https://download-directory.github.io/?url=https://github.com/xMasterX/unleashed-extra-pack/tree/main/apps)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper (official link)](https://flipperzero.one/update)

## Please support development of the project
* Boosty: https://boosty.to/mmxdev
* Ko-Fi: https://ko-fi.com/masterx
* destream (100 EUR min): https://destream.net/live/MMX/donate
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


