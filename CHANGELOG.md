### New changes
* SubGHz: New protocols support: CAME Space, Stilmatic / Schellenberg
* SubGHz: Add Manually - new protocols -> Beninca, Sommer(FSK), IronLogic, DTM Neo, Gibidi, Elmes Electronic (Elmes Poland), CAME Space
* SubGHz: Security+ 2.0 -> Ability to send custom buttons codes (0x80, 0x81, 0xE2)
* SubGHz: Remember last external module setting and power setting, so if you turn off flipper it will remember last external module settings (only for subghz app)
* SubGHz: Fix issues when external module is not found but plugins tries to use it, now they will fallback to internal in that case
* SubGHz: Fixed external CC1101 module power issues, added more checks, fixed issues when launching subghz favourites
* SubGHz: Removed 330MHz from default freq hopper to make it faster, to change freq hopper settings and remove/add your freqs see -> [Instruction](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzSettings.md)
* SubGHz: Small UI text fixes
* GUI: Fix submenu elements text size, now we can fit more symbols
* Plugins: Added delay and retries to avoid power issues in plugins -> WiFi Scanner, ESP8266 Deauther
* Plugins: Update for WiFi Marauder companion -> `sniff` saves pcaps in `YourFlippersMicroSD/apps_data/marauder/` -> Only with custom marauder build (It is necessary to uncomment "#define WRITE_PACKETS_SERIAL" in configs.h (in marauder fw) and compile the firmware for the wifi board.) Or download precompiled build -> [Download esp32_marauder_ver_flipper_sd_serial.bin](https://github.com/justcallmekoko/ESP32Marauder/releases/latest) - [(by tcpassos)](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion/pull/7)
* Plugins: Removed `cdefines` from external plugins as it was not used in any way
* Plugins: Updated **TOTP (Authenticator)** [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator) (fixed issue #379)
* Plugins: Update for SubGHz Bruteforcer, added - Holtek HT12X 12bit AM 433.920MHz/315MHz/868MHz/915MHz (TE: 433us)
* OFW: iButton: Add support for Dallas DS1971
* OFW: fbt: explicitly set dist suffix length, not depending on environment settings
* OFW: NFC -> Skip the read when the card is not present
* OFW: NFC -> Mark keys as not found when they couldn't auth successfully
* OFW: Storage -> Require the trailing slash for root paths
* OFW: gh: use shallow clones whenever possible
* OFW: Add new nfc apdu cli command 
* OFW: Picopass standard KDF dictionary 
* OFW: Nfc: fixes for latest PVS-studio 7.23
* OFW: Dolphin: new spring animation, weight adjust, drop winter animation.
* OFW: fbt, faploader: minimal app module implementation -> **All plugins now should have** `apptype=FlipperAppType.EXTERNAL`
* OFW: Fbt: fix broken resource deployment
* OFW: NFC: Fix 0 block write possibility in Mifare Classic emulation
* OFW: BadUSB: updated parser and added stringln, hold and release
* OFW: Upside down / left handed orientation support 
* OFW: Embed assets in elf file
* OFW: Dumb mode menu freeze fix

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


