### New changes
* Added pack of community apps with some fixes/changes - [(Full list)](https://github.com/Eng1n33r/flipperzero-firmware#apps-included)
* BadUSB extra keyboard layouts [(By v1nc)](https://github.com/v1nc/flipperzero-firmware) - [How to use](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/BadUSBLayouts.md)
* Added ability to extend subghz frequency range
* Updated docs and readme

- [!!!] Replace your subghz/assets/setting_user with file from `sd-card-(version).zip` because file format has been changed

**Note: We changed version names, because our releases not based on official releases, now versions is called
cg1-(commit-sha) - where "cg" = "Code Grabber", 1 = number of build(if ofw commit doesnt change), and (commit sha) - means ofw dev commit on which our version is based**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip`

DFU for update using qFlipper - `flipper-z-f7-full-(version).dfu`

If using DFU update method, download this archive and unpack it to your microSD, replacing all files except files you have edited manually -
`sd-card-(version).zip`