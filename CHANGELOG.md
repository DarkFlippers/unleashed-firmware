### New changes
* Fix SubGHz Bruteforcer crash and implement support for new protocols in BF existing dump (@xMasterX)
* PR: Wifi Marauder: make "add random" not overlap > symbol (by @TQMatvey | PR #141)
### Previous changes
* Fixed TOTP (Authenticator) - downgraded to 1.1.0
* Fixed i2cTools menu bug
* API version changed to 8.2 due to OFW updates, **Download extra pack update only if your unleashed version was lower than 009!!!**
* Added Infrared API for .fap's
* **OFW: Mf Classic initial write, update, detect reader (when card has not readed keys/sectors you can use it as simulation for detect reader)**
* OFW: New Keeloq magic serial type 2, 3
* Plugins: SubGHz Bruteforcer - ability to add extra repeats (by @derskythe) (**Select by pressing left/right in first start screen! not in attack screen**)
* PR: File Browser (archive): Context menu option to show file content (by @askoriy | PR #139) (aka text file viewer) !!!
* PR: Flappy Bird: Increase pilars line width to improve visibility (by @ahumeniy | PR #140)
* PR: Add Dummy Mode Game Shortcuts (run more games by clicking and holding buttons on main screen) (by @RogueMaster | PR #137)
* Added ability to change animations with holding OK button on main screen -> to enable set debug to on (Settings -> System -> Debug -> ON)
* New icons for SubGHz signals (Dynamic, Static, Raw) and better icons in infrared universal remotes (by @Svaarich)
* SubGHz Remote - fix CC1101 init
* SubGHz: Add 330Mhz in setting user and frequency analyzer
* SubGHz: Saving latest modulation is disabled due to bugs with custom modulations (fixed crashes and broken RAW recordings)
* Plugins -> fixed wrong text in wifi deauther
* Plugins -> fixed hc-sr04 gpio
* Plugins -> Reset GPIO pins to default state
* Plugins -> Updated TOTP (Authenticator) [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator)
* Plugins -> Updated GPS [(By ezod)](https://github.com/ezod/flipperzero-gps) works with module `NMEA 0183` via UART (13TX, 14RX, GND pins on Flipper)
* Plugins -> Updated i2c Tools [(By NaejEL)](https://github.com/NaejEL/flipperzero-i2ctools) - C0 -> SCL / C1 -> SDA / GND -> GND | 3v3 logic levels only!
* Plugins -> Temperature Sensor - HTU21D / SI7021 [(By Mywk)](https://github.com/Mywk/FlipperTemperatureSensor) - [How to Connect](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/applications/plugins/temperature_sensor/Readme.md)
* OFW: Oregon2 additional sensors defines
* OFW: Remove resources from API to prevent frequent API version increase (changes a way how icons included in plugins)
* OFW: Dolphin score update take 2
* OFW: SubGhz: fix variable types and CC1101 GPIO initialization optimization
* OFW: Support for setting all screen orientations 
* OFW: SubGhz: add RAW Read threshold rssi
* OFW: WS -> add protocol Acurite 592TXR 
* OFW: WS -> fix Acurite-606TX protocol
* OFW: Oregon2 extra (new sensor)
* OFW: Infrared CLI, refactor code
* OFW: fbt fixes

#### [🎲 Download latest extra apps pack](https://download-directory.github.io/?url=https://github.com/xMasterX/unleashed-extra-pack/tree/main/apps)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper 1.2.1 (allows .tgz installation) (official link)](https://update.flipperzero.one/builds/qFlipper/1.2.1/)

## Please support development of the project
* Boosty: https://boosty.to/mmxdev
* destream (100 EUR min): https://destream.net/live/MMX/donate
* ETH/BSC/ERC20-Tokens: `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for iOS mobile app / qFlipper

Update using qFlipper (1.2.0+) is now possible with `.tgz` update package! Also you can use Web Updater or self-update package.

