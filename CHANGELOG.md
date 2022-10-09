### New changes
* Issues with hopping in subghz are fixed in this release
* OFW PR: Desktop: Set external apps as favorites (OFW PR 1816 by @djsime1) (and fixed forgotten furi_record_close)
* PR -> Plugins: Add CAME 12bit 303MHz to SubGHz Bruteforcer (by @derskythe | PR #87)
* PR -> BadUSB: Added Norwegian keyboard layout for BadUSB (by @jd-raymaker | PR #88)
* PR -> Plugins: Feature: allow to set ball speed in Arkanoid (by @an4tur0r | PR #92)
* Add USB Keyboard (& Mouse) plugin, replacing default USB Mouse demo included (plugin by @huuck) [Link to original](https://github.com/huuck/FlipperZeroUSBKeyboard)
* Fix USB Keyboard plugin wrong icon in mouse screen, rewrite view models to new type
* Updated universal remote assets (by @Amec0e)
* Plugins: SubGHz Bruteforcer - Fix wrong max value in BF existing dump
* API 3.0 -> 3.2 (all previous compiled apps still compatible)
* Add 312.2 MHz to subghz user config
* SubGHz: Fix double click after delete scene, fix rename bug
* SubGHz: proper free of rainbow tables
* SubGHz: Fixed stability issues with Came atomo, Nice Flor S, limited max history items to 60 (was 65)
* SubGHz: Fix Read screen GUI (still bugged in OFW)
* Adapted all plugins and other code to new FuriString, fixed all new issues with new string type
* Adapted all plugins to new printf format
* Adapted all plugins to new view model format (aka Removing lambdas)
* Adapted all plugins to new furi_stream
* OFW: Elf loader: do not load .ARM.* sections
* OFW: Removing lambdas
* OFW: BadUSB: add SYSRQ keys
* OFW: Gui: fix memory leak in file browser module 
* OFW: music_player: Return to browser instead of exiting on back button
* OFW: More correct elf loader
* OFW: Furi stream buffer
* OFW: Printf function attributes
* OFW: App name in CLI loader command, RFID data edit fix 
* OFW: Show in-app icons & names in archive browser
* OFW: M*LIB: non-inlined strings, FuriString primitive
* OFW: Remove string_push_uint64
* OFW: Mifare Classic read improvements
* OFW PR: updated icon names (OFW PR 1829 by nminaylov)

#### [🎲 Download extra apps pack](https://download-directory.github.io/?url=https://github.com/UberGuidoZ/Flipper/tree/main/Applications/Unleashed)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper 1.2.1 (allows .tgz installation) (official link)](https://update.flipperzero.one/builds/qFlipper/1.2.1/)

## Please support development of the project
* ETH/BSC/ERC20-Tokens: `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for iOS mobile app / qFlipper

Update using qFlipper (1.2.0+) is now possible with `.tgz` update package! Also you can use Web Updater or self-update package.

