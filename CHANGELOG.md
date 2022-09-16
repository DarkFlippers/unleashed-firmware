### New changes
* Critical issue fix: Fixed subghz read mode doesn’t work at all
* Updated archive app (Rename, File info) (+ fixes) (by @derskythe) [(PR 68)](https://github.com/Eng1n33r/flipperzero-firmware/pull/68)
* Plugins: Added uart echo and usb mouse apps
* Plugins: Fix nrfsniff log spam, add new icons for plugins (icons by @Svaarich)
* Plugins: Changed app types and added new category for music players
* Plugins: Added new start screen for doom (by @Svaarich)
* Reduced max SubGHz history items to resolve memory issues, was 99, now 65
* Updated universal remote assets (by @Amec0e)
* OFW: Fbt: fixed gdb-py path for MacOS

**Known issues: Picopass reader plugin crash**

**Note: To avoid issues prefer installing using web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or `.tgz` for iOS mobile app

DFU for update using qFlipper - `flipper-z-f7-full-(version).dfu`

If using DFU update method, download this archive and unpack it to your microSD, replacing all files except files you have edited manually -
`sd-card-(version).zip`

