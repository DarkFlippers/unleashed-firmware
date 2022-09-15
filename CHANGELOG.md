### New changes
* Adapted all plugins to new app system (@xMasterX)
* Fix detect raw bug, fix CAME Atomo crash (@xMasterX)
* Fix Zombiez (@xMasterX)
* Added and fixed DOOM, added sound effects, made it somewhat playable (@xMasterX)
* New clock (timer by GMMan and settings by kowalski7cc) (combined together by @xMasterX)
* Added some games - Flappy Bird, Zombiez (check readme for links)
* Added custom icons for apps (icons by @Svaarich & @Amec0e)
* New update slideshow (by @Svaarich) [(idea and first version by ESurge, see PR 61)](https://github.com/Eng1n33r/flipperzero-firmware/pull/61)
* Added Brazilian Portuguese badusb keyboard layout (by @web-mancha) [(PR 65)](https://github.com/Eng1n33r/flipperzero-firmware/pull/65)
* Save last used SubGHZ config settings (PR 67 by @derskythe) (with some changes) [(link)](https://github.com/Eng1n33r/flipperzero-firmware/pull/67)
* Arkanoid: rand_range, remove temp number (PR 66 by @TQMatvey)[(link)](https://github.com/Eng1n33r/flipperzero-firmware/pull/66)
* OFW PR: Power: refresh battery indicator on charger plug/unplug (OFW PR 1733 by nminaylov)
* OFW PR: Protocol data redecoding before write (OFW PR 1723 by nminaylov)
* OFW PR: updater: fixed failing backups on /int with empty files in it (OFW PR 1735 by hedger)
* OFW PR: NFC Notifications fix (OFW PR 1731 by Astrrra)
* OFW: App loader (.fap Loader)(aka elf loader) !!!

**Note: To avoid issues prefer installing using web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or `.tgz` for iOS mobile app

DFU for update using qFlipper - `flipper-z-f7-full-(version).dfu`

If using DFU update method, download this archive and unpack it to your microSD, replacing all files except files you have edited manually -
`sd-card-(version).zip`

