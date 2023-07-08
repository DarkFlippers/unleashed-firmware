## New changes
* !!! **Warning! After installing, Desktop settings (Favoutite apps, PIN Code, AutoLock time..) will be resetted to default due to settings changes, Please set your PIN code, Favourite apps again in Settings->Desktop** !!!
* If you have copied any apps manually into `apps` folder - remove `apps` folder or that specific apps you copied on your microSD before installing this release to avoid issues due to OFW API version update! If you using regular builds or extra pack builds (e) without your manually added apps, all included apps will be installed automatically, no extra actions needed!
-----
* SubGHz: **Keeloq: Centurion Nova read and emulate support (+ add manually)**
* SubGHz: FAAC SLH - UI Fixes, Fix sending signals with no seed
* SubGHz: Code cleanup, proper handling of protocols ignore options (by @gid9798 | PR #516)
* SubGHz: Various UI fixes (by @wosk | PR #527)
* NFC: Fixed issue #532 (Mifare classic user dict - delete removes more than selected key)
* Infrared: Updated universal remote assets (by @amec0e | PR #529)
* Plugins: **Use correct categories (folders) for all plugins (extra pack too)**
* Plugins: Various fixes for uFBT (by @hedger)
* Plugins: Added **NFC Maker** plugin (make tags with URLs, Wifi and other things) [(by Willy-JL)](https://github.com/ClaraCrazy/Flipper-Xtreme/tree/dev/applications/external/nfc_maker) 
* Plugins: Added JetPack Joyride [(by timstrasser)](https://github.com/timstrasser)
* Plugins: Moved Barcode Generator [(by Kingal1337)](https://github.com/Kingal1337/flipper-barcode-generator) from extra pack into base fw, old barcode generator was removed
* Plugins: Updated ESP32: WiFi Marauder companion plugin [(by 0xchocolate)](https://github.com/0xchocolate/flipperzero-wifi-marauder)
* Plugins: Updated i2c Tools [(by NaejEL)](https://github.com/NaejEL/flipperzero-i2ctools)
* Settings: Change LED and volume settings by 5% steps (by @cokyrain)
* BLE: BadBT fixes and furi_hal_bt cleanup (by @Willy-JL)
* WIP OFW PR 2825: **NFC: Improved MFC emulation on some readers (by AloneLiberty)**
* OFW PR 2829: Decode only supported Oregon 3 sensor (by @wosk)
* OFW PR: Update OFW PR 2782
* OFW: SubGhz: add "SubGhz test" external application and the ability to work "SubGhz" as an external application
* OFW: API: explicitly add math.h
* OFW: NFC: Mf Ultralight emulation optimization
* OFW: Furi_Power: fix furi_hal_power_enable_otg 
* OFW: FuriHal: allow nulling null isr
* OFW: FuriHal, Infrared, Protobuf: various fixes and improvements 
* OFW: Picopass fix ice
* OFW: Desktop settings: show icon and name for external applications
* OFW: Furi,FuriHal: various improvements
* OFW: Debug apps: speaker, uart_echo with baudrate
* OFW: Add Mitsubishi MSZ-AP25VGK universal ac remote
* OFW: Fix roll-over in file browser and archive
* OFW: Fix fr-FR-mac keylayout
* OFW: NFC/RFID detector app
* OFW: Fast FAP Loader
* OFW: LF-RFID debug: make it work
* OFW: Fix M*LIB usage
* OFW: fix: make `dialog_file_browser_set_basic_options` initialize all fields
* OFW: Scroll acceleration
* OFW: Loader refaptoring: second encounter 

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



