# Flipper Zero Unleashed Firmware

<a href="https://ibb.co/wQ12PVc"><img src="https://i.ibb.co/wQ12PVc/fzCUSTOM.png" alt="fzCUSTOM" border="0"></a>

Welcome to Flipper Zero's Custom Firmware repo!
Our goal is to make any features possible in this device without any limitations! 

Please help us realize emulation for all dynamic (rolling codes) protocols and brute-force app!
<br>
<br>


### This software is for experimental purposes only and is not meant for any illegal activity/purposes. <br> We do not condone illegal activity and strongly encourage keeping transmissions to legal/valid uses allowed by law. <br> Also this software is made without any support from Flipper Devices and in no way related to official devs. 
### Please use for experimental purposes only!


<br>
<br>
Our Discord Community:
<br>
<a href="https://discord.gg/58D6E8BtTU"><img src="https://discordapp.com/api/guilds/937479784148115456/widget.png?style=banner4" alt="Unofficial Discord Community"></a>

<br>
<br>
<br>

# Update firmware

## [Get Latest Firmware from GitHub Releases](https://github.com/Eng1n33r/flipperzero-firmware/releases)

<br>
<br>

### **Update to the latest official firmware before proceeding**

<br>
<br>

## With offline update on flipper

### **Replace (CURRENT VERSION) with version that you downloaded from releases**
- Unpack `flipper-z-f7-update-(CURRENT VERSION).tgz` (or `.zip`) into any free folder on your PC or smartphone
- You should find folder named `f7-update-(CURRENT VERSION)` that contains files like `update.fuf`, `resources.tar` and etc..
- Remove microSD card from flipper and insert it into PC or smartphone (you can skip this step and upload all files using qFlipper)
- Create new folder `update` on the root of the microSD card and move folder that you previously extracted from archive - `f7-update-(CURRENT VERSION)` into `update` on microSD card
- So result should look like `update/f7-update-(CURRENT VERSION)/` with all files in this folder on microSD card, remember iOS default Files app doesnt show all files properly (3 instead of 5), so you need to use another app for unpacking or use PC or Android
- Verify that all files are present on your microSD card:
    * `update/f7-update-(CURRENT VERSION)/flipper-z-f7-full-(CURRENT VERSION).dfu`
    * `update/f7-update-(CURRENT VERSION)/flipper-z-f7-updater-(CURRENT VERSION).bin`
    * `update/f7-update-(CURRENT VERSION)/resources.tar`
    * `update/f7-update-(CURRENT VERSION)/stm32wb5x_BLE_Stack_light_fw.bin`
    * `update/f7-update-(CURRENT VERSION)/update.fuf`


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

- If you doing install for first time or migrating from official fw, unpack 3 folders from archive `sd-card-(CURRENT VERSION).zip` onto your microSD card


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

4. If you doing install for first time or migrating from official fw, unpack 3 folders from archive `sd-card-(CURRENT VERSION).zip` to your microSD card

<br>
<br>

# After install:
- ### If you installed using .dfu - unpack 3 folders from archive `sd-card-(CURRENT VERSION).zip` to your microSD card
<br>

- ## [How To: Configure UniversalRF Remix App](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/UniRFRemix.md)

<br>
<br>

# Where I can find IR, SubGhz, ... DBs, and other stuff?
## [Awesome Flipper Zero - Github](https://github.com/djsime1/awesome-flipperzero)

<br>
<br>

# How to Build by yourself:

## Clone the Repository

You should clone with 
```shell
$ git clone --recursive https://github.com/Eng1n33r/flipperzero-firmware.git
```

## Build with Docker

### Prerequisites

1. Install [Docker Engine and Docker Compose](https://www.docker.com/get-started)
2. Prepare the container:

 ```sh
 docker-compose up -d
 ```

### Compile everything for development

```sh
docker-compose exec dev make
```

### Compile everything for release + get updater package to update from microSD card

```sh
docker-compose exec dev make updater_package TARGET=f7 DEBUG=0 COMPACT=1
```

Check `dist/` for build outputs.

Use **`flipper-z-{target}-full-{suffix}.dfu`** to flash your device.

If compilation fails, make sure all submodules are all initialized. Either clone with `--recursive` or use `git submodule update --init --recursive`.

# Apps included

- [Clock/Stopwatch (By CompaqDisc, Stopwatch & Sound Alert By RogueMaster)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/blob/unleashed/applications/clock_app/clock_app.c)
- [UniversalRF Remix (By ESurge)(Original UniversalRF By jimilinuxguy)](https://github.com/ESurge/flipperzero-firmware-unirfremix)
- [WAV Player (By DrZlo13)](https://github.com/flipperdevices/flipperzero-firmware/tree/zlo/wav-player) With Fix From [Atmanos](https://github.com/at-manos)
- [Tetris (By jeffplang)](https://github.com/jeffplang/flipperzero-firmware/tree/tetris_game/applications/tetris_game)

# Links

* Unofficial Discord: [discord.gg/58D6E8BtTU](https://discord.gg/58D6E8BtTU)
* Docs by atmanos: [https://flipper.atmanos.com/docs](https://flipper.atmanos.com/docs/your-first-program/intro)

* Official Docs: [http://docs.flipperzero.one](http://docs.flipperzero.one)
* Official Forum: [forum.flipperzero.one](https://forum.flipperzero.one/)

# Project structure

- `applications`    - Applications and services used in firmware
- `assets`          - Assets used by applications and services
- `core`            - Furi Core: os level primitives and helpers
- `debug`           - Debug tool: GDB-plugins, SVD-file and etc
- `docker`          - Docker image sources (used for firmware build automation)
- `documentation`   - Documentation generation system configs and input files
- `firmware`        - Firmware source code
- `lib`             - Our and 3rd party libraries, drivers and etc...
- `make`            - Make helpers
- `scripts`         - Supplementary scripts and python libraries home

Also pay attention to `ReadMe.md` files inside of those directories.
