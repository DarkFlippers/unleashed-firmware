### New changes
* !!! **Warning! After installing, Desktop settings (Favoutite apps, PIN Code, AutoLock time..) will be resetted to default due to settings changes, Please set your PIN code, Favourite apps again in Settings->Desktop** !!!
* If you have copied any apps manually into `apps` folder - remove `apps` folder or that specific apps you copied on your microSD before installing this release to avoid issues due to OFW API version update! If you using regular builds or extra pack builds (e) without your manually added apps, all included apps will be installed automatically, no extra actions needed!
-----
* Desktop: **Show clock on main screen** (Enable in Settings->Desktop->Show Clock) (by @gid9798 | PR #484)
* SubGHz Remote: New plugin - Configurator (Remote Maker) - Now you can create and edit map files on flipper! (by @gid9798 | PR #487)
* SubGHz Remote: Full refactoring, app was re-made from scratch (by @gid9798)
* Archive: Fix rename, show error message to user
* API: Cleanup, mini refactoring of some apps (+6k of free flash space)
* LFRFID: Debug: Allow PSK RAW emulation in gui
* SubGHz: Security+ 2.0 -> add extra custom button `0x78` - Fixes issue #469
* SubGHz: Various fixes (by @gid9798)
* SubGHz: Fix counter settings in debug
* SubGHz: Move dangerous_settings check (by @gid9798 | PR #475)
* Misc: Move NFC plugins into NFC folder
* Misc: Name changer code moved to proper place, load after system startup + extra checks
* Plugins: Merge tiktok and ytshorts remote into one (by @Willy-JL)
* Plugins: NMEA GPS UART - stability fix
* Plugins: Port XFW keyboard with extra symbols to WiFi Marauder instead of using UART Term keyboard (thanks to @Willy-JL)
* Plugins: Moved from extra pack to main FW: Mifare Nested [(by AloneLiberty)](https://github.com/AloneLiberty/FlipperNested) - Works with PC and python app `FlipperNested`
* Plugins: Update TOTP (Authenticator) [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator) (+ Add option to set custom fonts)
* Plugins: Update NMEA GPS UART [(by ezod)](https://github.com/ezod/flipperzero-gps) (GLL support)
* Plugins: Update WiFi Marauder [(by 0xchocolate)](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion)
* OFW PR 2680: RFID - Add support for Nexkey/Nexwatch (by @mauimauer)
* OFW: nfc: Mifare Ultralight C detection
* OFW: api: added toolbox/api_lock.h
* OFW: NFC: Add support for Gen4 "ultimate card" in Magic app
* OFW: desktop: Refactor favorites settings and allow app browser in selection
* OFW: Infrared: respect carrier frequency and duty cycle settings -> **Breaking API change, API version was changed from 26.x to 27.x** 
* OFW: Desktop,Rpc: desktop status subscription
* OFW: Storage, common_rename: check that old path is exists
* OFW: Services: remove deallocator for persistent services
* OFW: Storage: common_rename is now POSIX compliant
* OFW: Removed user-specific data from tar artifacts
* OFW: fbt: Fix tar uid overflow when packaging
* OFW: fbt: Use union for old py (Fix builds if using older python versions)
* OFW: USB HID report timeout

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


