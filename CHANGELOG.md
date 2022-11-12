### New changes
* API now 99% compatible with official firmware, that means all apps built on OFW can be used on unleashed!
* Also extra apps pack was updated, download latest by using link below
* Archive: Show loading popup on delete
* Docs -> PR: Fix link to "TOTP (Authenticator) config description" (by @pbek | PR #157)
* Reorder main menu - Applications now first item, clock moved 2 items up
* API: Add `value_index` to API symbols
* API: Furi Region Mocks, fix protocol dict funcs was disabled in API
* New animation L3_FlipperMustache_128x64 by @Svaarich
* Fix FlipperCity animation by @Svaarich
* CI/CD: Builds without custom animations now included in releases
* SubGHz: Fix magellan display issue
* SubGHz: Fix wrong error message in history 
* SubGHz: Add frequencies 434.075, 434.390
* SubGHz: Frequency analyzer: Add counter, GUI fixes, allow Ok button - When signal is present (when frequency window shows black background)
* SubGHz: Frequency analyzer: move -+ in freq analyzer, swap up & down button
* SubGHz Remote: Cleanup code in unirf, fix issue #153
* Plugins: Remove `srand` calls
* Plugins: Fix DHT Monitor icon
* Plugins: RFID Fuzzer - Fix random crashes and improve stability
* Plugins: RFID Fuzzer - allow holding left right to change delay faster (hold TD button to add +10 or -10 to time delay)
* Plugins: Morse code cleanup text by pressing back
* Plugins: TOTP Update - "BadUSB" type key into pc mode [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator)
* Plugins: Update i2c Tools [(by NaejEL)](https://github.com/NaejEL/flipperzero-i2ctools)
* Plugins -> PR: Barcode generator: refactoring, ux improvements, implement EAN-8. (by @msvsergey | PR #154)
* Plugins -> PR: Fix HC-SR04 plugin naming (by @krolchonok | PR #161)
* Plugins: Added BH1750 - Lightmeter [(by oleksiikutuzov)](https://github.com/oleksiikutuzov/flipperzero-lightmeter)
* Plugins: Added iButton Fuzzer [(by xMasterX)](https://github.com/xMasterX/ibutton-fuzzer)
* OFW: BadUSB and Archive fixes
* OFW: iButton: Fix header "Saved!" message stays on other screens + proper popups reset
* OFW: Bug fixes and improvements: Furi, Input, CLI
* OFW: SubGhz: properly handle storage loss 
* OFW: NFC - Force card types in extra actions
* OFW: (docs): bad path for furi core
* OFW: RPC: increase stack size, fix stack overflow 
* OFW: fbt: 'target' field for apps; lib debugging support 
* OFW: NFC: fix crash on MFC read
* OFW: Furi: show thread allocation balance for child threads
* OFW: Add Acurite 609TXC protocol to weather station
* OFW: DAP-Link: show error if usb is locked
* OFW: fbt: compile_db fixes
* OFW: Infrared: add Kaseikyo IR protocol
* OFW: WS: fix show negative temperature
* OFW: fbt: fix for launch_app
* OFW: Code cleanup: srand, PVS warnings
* OFW: fbt: fixes for ufbt pt3 

#### [🎲 Download latest extra apps pack](https://download-directory.github.io/?url=https://github.com/xMasterX/unleashed-extra-pack/tree/main/apps)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper 1.2.1 (allows .tgz installation) (official link)](https://update.flipperzero.one/builds/qFlipper/1.2.1/)

## Please support development of the project
* Boosty: https://boosty.to/mmxdev
* destream (100 EUR min): https://destream.net/live/MMX/donate
* USDT(TRC20): `TSXcitMSnWXUFqiUfEXrTVpVewXy2cYhrs`
* BCH: `qquxfyzntuqufy2dx0hrfr4sndp0tucvky4sw8qyu3`
* ETH/BSC/ERC20-Tokens: `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for iOS mobile app / qFlipper

Update using qFlipper (1.2.0+) is now possible with `.tgz` update package! Also you can use Web Updater or self-update package.

