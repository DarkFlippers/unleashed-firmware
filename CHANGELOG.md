### New changes
* Spectrum Analyzer moved into Applications -> Tools
* Fixed bug with subghz remote (unirf) that was causing issues with power state
* Added 868.4 MHz into subghz user config 
* Updated universal IR assets (by @Amec0e)
* Fixed debug builds - add this parameter to fbt command `FIRMWARE_APP_SET=debug_pack` if you building full fw in debug mode

#### **DFU files no longer included in releases to avoid issues with wrong manual installation of assets - use web updater or microSD update package**

[- How to install](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/HowToInstall.md)

**Note: To avoid issues prefer installing using web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or `.tgz` for iOS mobile app

DFU for update using qFlipper is no longer included in releases to avoid issues with assets - Use Web Updater or self-update package!

