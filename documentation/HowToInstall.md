
# Update firmware

## [Get Latest Firmware from GitHub Releases](https://github.com/Eng1n33r/flipperzero-firmware/releases)

<br>
<br>

### **If installing for first time - Update to the latest official firmware before proceeding**

<br>
<br>

## With Web Updater

- Be sure you updated to latest official release before, and verify that microSD card is installed
- Open latest release page - [Releases](https://github.com/Eng1n33r/flipperzero-firmware/releases/latest)
- Connect your device and select `Install via Web Updater`
after that on web updater page - press `Connect` button
- Press `Install` button

- And wait, if all flashed successfully - you will have all needed assets pre installed
- Done

<br>
<br>

## With offline update on flipper

### **Replace (CURRENT VERSION) with version that you downloaded from releases**
- Unpack `flipper-z-f7-update-(CURRENT VERSION).tgz` (or `.zip`) into any free folder on your PC or smartphone
- You should find folder named `f7-update-(CURRENT VERSION)` that contains files like `update.fuf`, `resources.tar` and etc..
- Remove microSD card from flipper and insert it into PC or smartphone (you can skip this step and upload all files using qFlipper)
- Create new folder `update` on the root of the microSD card and move folder that you previously extracted from archive - `f7-update-(CURRENT VERSION)` into `update` on microSD card
- So result should look like `update/f7-update-(CURRENT VERSION)/` with all files in this folder on microSD card, remember iOS default Files app doesn't show all files properly (3 instead of 6), so you need to use another app for unpacking or use PC or Android
- Verify that all files are present on your microSD card
- After all you need to insert microSD card back into flipper, navigate into filebrowser, open this file 
`update/f7-update-(CURRENT VERSION)/update.fuf`
- Update will start, wait for all stages, and when flipper started after update, you can upload any custom [IR libs](https://github.com/logickworkshop/Flipper-IRDB), and other stuff using qFlipper or directly into microSD card

<br>
<br>

## With qFlipper

### **Replace (CURRENT VERSION) with version that you downloaded from releases**
- Connect your device and select `Update from file`
then select **`flipper-z-f7-full-(CURRENT VERSION).dfu`**

- And wait, if all flashed successfully - you can manually upload IR libs and other stuff to sd card

- If you doing install for first time or migrating from official fw, unpack files from archive `sd-card-(CURRENT VERSION).zip` onto your microSD card


<br>
<br>

## With USB DFU 

1. Download latest [Firmware](https://github.com/Eng1n33r/flipperzero-firmware/releases)

2. Reboot Flipper to Bootloader
 - Press and hold `← Left` + `↩ Back` for reset 
 - Release `↩ Back` and keep holding `← Left` until blue LED lights up
 - Release `← Left`

### **Replace (CURRENT VERSION) with version that you downloaded from releases**
3. Run `dfu-util -D flipper-z-f7-full-(CURRENT VERSION).dfu -a 0`

4. If you doing install for first time or migrating from official fw, unpack files from archive `sd-card-(CURRENT VERSION).zip` to your microSD card

<br>
<br>

# After install:

- ## [Read instructions how to use plugins and more](https://github.com/Eng1n33r/flipperzero-firmware#instructions)

- ## [How To: Configure UniversalRF Remix App](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/UniRFRemix.md)
