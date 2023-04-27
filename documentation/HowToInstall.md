
# Update firmware

## [Get Latest Firmware from GitHub Releases](https://github.com/DarkFlippers/unleashed-firmware/releases)

<br>
<br>

### **If installing for first time - Update to the latest official firmware before proceeding**
### **If you are already using unleashed - no need to install any other FW or version before installing update, just install latest version on top of your current one, all will be fine**

<br>
<br>

## With Web Updater

- Be sure you updated to latest official release before(if installing for the first time), and verify that microSD card is installed
- Open latest release page - [Releases](https://github.com/DarkFlippers/unleashed-firmware/releases/latest)
- Connect your device and select `Install via Web Updater`
after that on web updater page - press `Connect` button
- Press `Install` button

- And wait, if all flashed
 successfully - you will have all needed assets pre installed
- Done
![web](https://user-images.githubusercontent.com/40743392/235005830-98ceda39-a143-47ef-ad4d-5489bc3df98b.png)


<br>
<br>

## With iOS mobile app

- Be sure you updated to latest official release before(if installing for the first time), and verify that microSD card is installed
- Open latest release page - [Releases](https://github.com/DarkFlippers/unleashed-firmware/releases/latest)
- Download `flipper-z-f7-update-(version).tgz`
- Open downloads in ios Files app, select downloaded `.tgz` file, click Share, select Flipper App
- In flipper app click green `Update` button, be sure it shows `Custom flipper-z-f7-update...` in Update Channel
- Wait until update is finished 
- Error in ios app will show up, but flipper will be updated successfully
- And if all flashed successfully - you will have all needed assets pre installed
- Done
![ios](https://user-images.githubusercontent.com/40743392/235005844-bea8f2fd-f50d-41b1-9191-e3842d8658d2.png)

<br>
<br>

## With Android mobile app (with .tgz download)

- Be sure you updated to latest official release before(if installing for the first time), and verify that microSD card is installed
- Open latest release page - [Releases](https://github.com/DarkFlippers/unleashed-firmware/releases/latest)
- Download `flipper-z-f7-update-(version).tgz`
- In flipper app click `Update channel` button, select `Custom`
- Select downloaded `.tgz` file
- Click Update
- Wait until update is finished 
- And if all flashed successfully - you will have all needed assets pre installed
- Done
![andro_tgz](https://user-images.githubusercontent.com/40743392/235005877-d4f5f73c-241c-4a7b-a51d-b8407983856c.png)


<br>
<br>

## With Android mobile app (via web updater link)

- Be sure you updated to latest official release before(if installing for the first time), and verify that microSD card is installed
- Open latest release page - [Releases](https://github.com/DarkFlippers/unleashed-firmware/releases/latest)
- Click `Install via Web Updater`
- It will ask to open with browser or Flipper app, select Flipper App
- Continue to install
- Wait until update is finished 
- And if all flashed successfully - you will have all needed assets pre installed
- Done
![androweb](https://user-images.githubusercontent.com/40743392/235005891-19ef6bb6-094f-437d-afcd-75d60921e3c4.png)


<br>
<br>

## With qFlipper (1.2.0+)

- Download qFlipper that allows `.tgz` installation [Download qFlipper (official link)](https://flipperzero.one/update)
- Be sure you updated to latest official release before(only if installing for the first time), and verify that microSD card is installed
- Open latest release page - [Releases](https://github.com/DarkFlippers/unleashed-firmware/releases/latest)
- Download `flipper-z-f7-update-(version).tgz`
- Launch qFlipper
- Connect your device and select `Install from file`
- Select `flipper-z-f7-update-(version).tgz` that you downloaded
- Update will start
- And wait, if all flashed successfully - you will have all needed assets pre installed
- Done
![qflip](https://user-images.githubusercontent.com/40743392/235005910-819abd34-65d4-4aaa-a11c-9c28bea737e9.png)



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
- Update will start, wait for all stages
- Done
![manual](https://user-images.githubusercontent.com/40743392/235006410-19eaf58e-2425-4e8e-8ec9-973bda362c47.png)





<br>
<br>

# After install:

- ## [Read instructions how to use plugins and more](https://github.com/DarkFlippers/unleashed-firmware#instructions)

- ## [How To: Configure Sub-GHz Remote App](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/SubGHzRemotePlugin.md)
