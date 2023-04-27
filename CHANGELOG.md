### New changes 
* Power + BLE: DeepSleep + required ble stack upgrade added back, all known issues was fixed in OFW, no issues was found during our tests
* Desktop: Allow locking without pin using Up menu on desktop (Short click on `Lock` = Without PIN / Long = With PIN)
* RFID: Add confirmation message before running `Clear T5577 Password` 
* RFID: Add more user friendly RAW emulation via UI [(by Dan Caprita)](https://forum.flipperzero.one/t/electra-intercom/6368/43)
* SubGHz: Fixed `Frequency Analyzer` issues, fixed `Read` mode issues
* SubGHz: Fix NFC crash when using external CC1101 radio module
* SubGHz: Fix multiple external CC1101 radio module issues, (int callbacks, SPI handlers init/reinit)
* SubGHz: Using scene manager function in add manually (by @gid9798 | PR #437)
* Plugins: ESP32: WiFi Marauder - add icon for log files in logs browser
* Plugins: Update **ESP32: WiFi Marauder companion** plugin [(by 0xchocolate)](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion) merged [PR by @tcpassos](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion/pull/11)
* Plugins: Update **TOTP (Authenticator)** [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator)
* Plugins: Fix RFID Fuzzer and iButton Fuzzer crashes
* Plugins: iButton Fuzzer default keys update (by @team-orangeBlue)
* Infrared: Updated infrared assets (by @amec0e | PR #441)
* Docs: Update **How To Install** images (by @krolchonok | PR #436)
* OFW PR 2620: NFC: Fix reading Mifare Classic cards with unusual access conditions and fix emulation of unknown keys (by Astrrra)
* OFW PR 2616: Picopass: remove spaces in CSN (by bettse)
* OFW PR 2604: WS: add protocol "Wendox W6726" (by Skorpionm)
* OFW PR 2607: BadUSB: command parser fix (by nminaylov)
* OFW: Keep HSI16 working in stop mode.
* OFW: FuriHal: use proper divider for core2 when transition to sleep, remove extra stop mode transition checks, cleanup code. Furi: proper assert and check messages.
* OFW: Don't reboot on crash in debug builds
* OFW: cubewb: downgraded to v1.15.0 

#### [🎲 Download latest extra apps pack](https://github.com/xMasterX/all-the-plugins/archive/refs/heads/main.zip)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper (official link)](https://flipperzero.one/update)

## Please support development of the project
* **Boosty** (patreon alternative): https://boosty.to/mmxdev
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


