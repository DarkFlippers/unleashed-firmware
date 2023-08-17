## New changes
* NFC: Add manually MF Classic with custom UID (by @gid9798 | PR #571)
* Infrared: Fix crash when frequency is outside of supported range, allow sending broken files by setting frequency to nearest border
* Infrared: Updated AC universal asset (Legion LE-F30RH-IN added and other changes) (by @Leptopt1los | PR #577)
* SubGHz: Fix Somfy Telis wrong frequency in add manually
* SubGHz: Fix frequency out of bounds check causing crash/freeze
* RGB Patch: Fix colors
* Builds: New build (clean) `c` - comes only with main apps, use in case you don't want preinstalled apps pack
* Plugins: Moved into one repo - https://github.com/xMasterX/all-the-plugins
* OFW PR 2984: (only buffer changes was merged, other will be later) SubGhz: fix todo (by Skorpionm)
* OFW PR 2980: Properly reset the NFC device data (by Astrrra)
* OFW: SubGhz: add timeout to subghz_hal_async_tx_test_run 
* OFW: Improve vscode clangd experience
* OFW: Add the Sad song animation
* OFW: ufbt: fixed FAP_SRC_DIR
* OFW: UI: Clock on Desktop -> Refactoring of our desktop clock
* OFW: uFBT: devboard_flash to update WiFi devboard 
* OFW: FBT: devboard_flash to update WiFi devboard
* OFW: Scripts: OB recovery
* OFW: Expose additional functions of the crypto engine to user -> **Breaking API change 34.x -> 35.x**
* OFW: External apps removed -> In our case - moved into extra plugins repo to separate plugins and firmware
* OFW: BadUSB: qFlipper install script for MacOS
* OFW: Add compressor.h to the SDK
* OFW: fbt: Fix building using path with space
* OFW: RPC: md5 in storage list
* OFW: Fixes 2957 - subghz decode_raw
* OFW: FDX-B temperature in system units
* OFW: Infrared: buttons move feature rework

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

### What `n`, `r`, `e`, ` `, `c` means? What I need to download if I don't want to use Web updater?
What build I should download and what this name means - `flipper-z-f7-update-(version)(n / r / e / c).tgz` ? <br>
`flipper-z` = for Flipper Zero device<br>
`f7` = Hardware version - same for all flipper zero devices<br>
`update` = Update package, contains updater, all assets (plugins, IR libs, etc.), and firmware itself<br>
`(version)` = Firmware version<br>
` ` = this build comes with 3 custom animations, and default apps preinstalled<br>
`c` = this build comes with 3 custom animations, and only main apps (Clean build)<br>
`n` = this build comes without our custom animations (we have only 3 of them), only official flipper animations<br>
`e` = build has 🎲 [extra apps pack](https://github.com/xMasterX/all-the-plugins) preinstalled<br>
`r` = RGB patch (+ extra apps) for flippers with rgb backlight mod (this is hardware mod!) (Works only on modded flippers!) (do not install on non modded device!)

Firmware Self-update package (update from microSD) - `flipper-z-f7-update-(version).tgz` for mobile app / qFlipper / web<br>
Archive of `scripts` folder (contains scripts for FW/plugins development) - `flipper-z-any-scripts-(version).tgz`<br>
SDK files for plugins development and uFBT - `flipper-z-f7-sdk-(version).zip`



