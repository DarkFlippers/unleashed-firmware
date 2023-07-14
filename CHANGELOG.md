## New changes
* !!! **Warning! After installing, Desktop settings (Favoutite apps, PIN Code, AutoLock time..) will be resetted to default due to settings changes, Please set your PIN code, Favourite apps again in Settings->Desktop** !!!
* This next text applies to you only **If you have copied any apps manually** into `apps` folder and don't know that you need to update those apps manually in same way you added them - remove `apps` folder or that specific apps you copied on your microSD before installing this release to avoid issues due to OFW API version update! **If you using regular builds or extra pack builds (e) without your manually added apps, all included apps will be installed automatically, no extra actions needed!**
-----
* SubGHz: Port latest OFW external radio driver, fix issues (now you can make drivers for other radio chips) (by @gid9798 | PR #539 #536 #535 #534)
* GUI module: SubMenu fix vertical orientation (by @gid9798 | PR #543)
* Apps: After merge fixes (by @gid9798 | PR #537)
* Docs: Update docs for debug build and update vscode example, please remove `debug_pack` target if you had it in your workflow
* Infrared: Updated universal remote assets (by @amec0e | PR #544)
* Plugins: Added Camera Suite GPIO application for the ESP32-CAM module. [(by CodyTolene)](https://github.com/CodyTolene/Flipper-Zero-Camera-Suite) (PR #541)
* Plugins: Updated ESP32: WiFi Marauder companion plugin [(by 0xchocolate)](https://github.com/0xchocolate/flipperzero-wifi-marauder)
* Plugins: Updated **NFC Maker** plugin (make tags with URLs, Wifi and other things) [(by Willy-JL)](https://github.com/ClaraCrazy/Flipper-Xtreme/tree/dev/applications/external/nfc_maker) 
* Plugins: Updated **Mifare Nested** [(by AloneLiberty)](https://github.com/AloneLiberty/FlipperNested)
* Plugins: Updated Lightmeter [(by oleksiikutuzov)](https://github.com/oleksiikutuzov/flipperzero-lightmeter)
* Plugins: Updated **Multi (RFID/iButton) Fuzzer** [(by @gid9798)](https://github.com/DarkFlippers/Multi_Fuzzer)
* OFW: Loader: good looking error messages
* OFW: Desktop,Cli: add uptime info
* OFW: Archive and file browser fixes
* OFW: Loader: exit animation fix
* OFW: SubGhz: fix check connect cc1101_ext 
* OFW: NFC: Improved MFC emulation on some readers
* OFW: Dolphin: add new animation
* OFW: Update toolchain to v23
* OFW: More descriptive error messages for the log command
* OFW: **External menu apps** -> now we have ton of free internal space
* OFW: Device Info update

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



