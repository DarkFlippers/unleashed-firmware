## New changes
* SubGHz: Support for Ebyte E07 module power amp switch (works with TehRabbitt's Flux Capacitor Board) (by @Sil333033) (PR #559 by @Z3BRO) -> Remade by @xMasterX -> Driver code fixed and reworked by @gid9798
* Infrared: Update universal remote assets (by @amec0e | PR #570)
* Infrared: Update universal AC asset (by @Leptopt1los | PR #569)
* Plugins: Add * in NFC Maker keyboard (hold `.`)
* Plugins: Update TOTP (Authenticator) [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator)
* Plugins: Update ESP32: WiFi Marauder companion plugin [(by 0xchocolate)](https://github.com/0xchocolate/flipperzero-wifi-marauder)
* Plugins: Update ESP32-CAM -> Camera Suite [(by CodyTolene)](https://github.com/CodyTolene/Flipper-Zero-Camera-Suite) -> (PR #562 by @CodyTolene)
* OFW PR 2949: IR: buttons move feature rework (by nminaylov)
* OFW PR 2941: FDX-B temperature now uses system units (by Astrrra)
* OFW: fbtenv: add additional environ variable to control execution flow
* OFW: NFC CLI: Fix multiple apdu commands from not working when one of them gives an empty response
* OFW: NFC: Fix MFC key invalidation
* OFW: Rename Applications to Apps 
* OFW: Fix about screen
* OFW: change FuriThreadPriorityIsr to 31 (configMAX_PRIORITIES-1)
* OFW: External apps icounter
* OFW: Overly missed feature: Infrared: move button (change button order in a remote)
* OFW: Move U2F path to ext
* OFW: New RTC flags in device info
* OFW: Backlight notification fix
* OFW: Fix fbtenv restore

----

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

#### Thanks to our sponsors:
callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
and all other great people who supported our project and me (xMasterX), thanks to you all!


## **Recommended update option - Web Updater**

### What `n`, `r`, `e` means? What I need to download if I don't want to use Web updater?
What build I should download and what this name means - `flipper-z-f7-update-(version)(n / r / e).tgz` ? <br>
`flipper-z` = for Flipper Zero device<br>
`f7` = Hardware version - same for all flipper zero devices<br>
`update` = Update package, contains updater, all assets (plugins, IR libs, etc.), and firmware itself<br>
`(version)` = Firmware version<br>
`n` = this build comes without our custom animations (we have only 3 of them), only official flipper animations<br>
`e` = build has 🎲 [extra apps pack](https://github.com/xMasterX/all-the-plugins) preinstalled<br>
`r` = RGB patch (+ extra apps) for flippers with rgb backlight mod (this is hardware mod!) (Works only on modded flippers!) (do not install on non modded device!)

Firmware Self-update package (update from microSD) - `flipper-z-f7-update-(version).tgz` for mobile app / qFlipper / web<br>
Archive of `scripts` folder (contains scripts for FW/plugins development) - `flipper-z-any-scripts-(version).tgz`<br>
SDK files for plugins development and uFBT - `flipper-z-f7-sdk-(version).zip`



