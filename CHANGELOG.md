### New changes
* PR: SubGHz bruteforcer plugin - fixes and speed-up (-2 min) (by @derskythe | PR #76)
* Fix nfc device typo - key_a_mask was used for key B 
* OFW: Applications loader: do not use view dispatcher queue
* OFW: Power: Also ask charger if charge done
* OFW: Fast flash programming mode (faster firmware flash)

#### **DFU files no longer included in releases to avoid issues with wrong manual installation of assets - use .tgz file with qFlipper, or install automatically via web updater or use microSD update package**

[- How to install](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/HowToInstall.md)

[- Download qFlipper 1.2.0 (allows .tgz installation) (official link)](https://update.flipperzero.one/builds/qFlipper/1.2.0/)

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for iOS mobile app / qFlipper

Update using qFlipper (1.2.0) is now possible with `.tgz` update package! Also you can use Web Updater or self-update package.

