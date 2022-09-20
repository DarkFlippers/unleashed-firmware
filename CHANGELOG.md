### New changes
* PR: RFID Fuzzer - support for HIDProx, update for UI (PR #74 by mvanzanten) (xMasterX -> time between cards set to 6)
* Fix NFC User dict (list) crash, now it displays only first 200 elements for large lists
* Fix SubGHz transmitter GUI button
* Fix SubGHz Magellen protocol GUI
* Fix null pointer dereference crash in Archive -> Info in root folder (+ fix long path names display)
* OFW: SubGHz: Adding checks for get_upload functions

#### **DFU files no longer included in releases to avoid issues with wrong manual installation of assets - use .tgz file with qFlipper, or install automatically via web updater or use microSD update package**

[- How to install](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/HowToInstall.md)

[- Download qFlipper 1.2.0-rc1 (allows .tgz installation) (official link)](https://update.flipperzero.one/builds/qFlipper/1.2.0-rc1/)

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for iOS mobile app / qFlipper

Update using qFlipper (1.2.0-rc1) is now possible with `.tgz` update package! Also you can use Web Updater or self-update package.

