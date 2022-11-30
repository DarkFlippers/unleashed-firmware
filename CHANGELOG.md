### New changes
* Extra [apps pack](https://download-directory.github.io/?url=https://github.com/xMasterX/unleashed-extra-pack/tree/main/apps) update required! Please download using link below, API version changed to 8.x (details below)
* SubGHz: Fix counter can go higher than 16bits in protocols - Keeloq, SL, Came Atomo, Nice Flor S
* SubGHz -> Plugin: WS - Add protocol Auriol HG0601A (by @LY2NEO) [(Details)](https://github.com/DarkFlippers/unleashed-firmware/issues/184)
* NFC -> PR: Assets - mf classic dict update - Possible keys for Omsk transport cards (by @vadrozh | PR #181)
* BadUSB -> PR: Keyboard layouts: Slovenian (si) / Croatian (hr), Bosnian (ba) (Latin, Bosnia and Herzegovina) (by @StellarStoic | PR #187)
* Plugins: Update Temperature Sensor Plugin - HTU2xD, SHT2x, SI702x, SI700x, SI701x, AM2320 [(by Mywk)](https://github.com/Mywk/FlipperTemperatureSensor)
* Plugins: Update HEX Viewer [(by QtRoS)](https://github.com/QtRoS/flipper-zero-hex-viewer)
* OFW: SubGhz: fix RAW "Send never ends
* OFW: Allow "Detect reader" for unsaved card 
* OFW: Blocking USB driver API 
* OFW: Unified Info API, App Error, Data Xchange (breaking change in API, API version was bumped to 8.x - OFW)
* OFW: Improve file name filtering
* OFW: SubGhz: fix duration pricenton protocol
* OFW: Gui: better navigation in file browser dialog
* OFW: Nfc: NTAG password auto capture (and other password-related changes) 
* OFW: NFC: Accept non-parsed apps in Mifare DESFire.
* OFW: NFC: Fix MIFARE DESfire info action to open app menu

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

