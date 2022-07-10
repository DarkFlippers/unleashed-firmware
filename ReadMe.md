# Flipper Zero Unleashed Firmware

<a href="https://ibb.co/wQ12PVc"><img src="https://i.ibb.co/wQ12PVc/fzCUSTOM.png" alt="fzCUSTOM" border="0"></a>

Welcome to Flipper Zero's Custom Firmware repo!
Our goal is to make any features possible in this device without any limitations! 

Please help us implement emulation for all dynamic (rolling codes) protocols and brute-force app!
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

# What changed
* SubGhz regional TX restriction removed
* Rolling code protocols now has ability to save & send captured signals
* FAAC SLH (Spa) & BFT Mitto (secure with seed) manual creation
* Custom plugins and games included
* Extra SubGhz frequencies + extra mifare classic keys
* Picopass/iClass plugin is included in releases
* Other small fixes and changes

See changelog in releases for latest updates!

### Current modified and new SubGhz protocols list:
- CAME Atomo
- FAAC SLH (Spa)
- Keeloq(+ proper manufacturer codes selection)
- Nice Flor S
- SecPlus v1 & v2
- Star Line

### Apps included:

- Clock/Stopwatch [(By CompaqDisc, Stopwatch & Sound Alert By RogueMaster)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/blob/unleashed/applications/clock_app/clock_app.c)
- UniversalRF Remix [(By ESurge)(Original UniversalRF By jimilinuxguy)](https://github.com/ESurge/flipperzero-firmware-unirfremix)
- Tetris [(By jeffplang)](https://github.com/jeffplang/flipperzero-firmware/tree/tetris_game/applications/tetris_game)
- Spectrum Analyzer [(By jolcese)](https://github.com/jolcese/flipperzero-firmware/tree/spectrum/applications/spectrum_analyzer) - [Ultra Narrow mode & scan channels non-consecutively](https://github.com/theY4Kman/flipperzero-firmware/commits?author=theY4Kman)
- Arkanoid [(By gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins)
- Tic Tac Toe [(By gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins)


## Support us so we can buy equipment and develop new features
* ETH/BSC/ERC20-Tokens: `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`

# Instructions
## [- How to install firmware](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/HowToInstall.md)

## [- How to build firmware](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/HowToBuild.md)

## [- Configure UniversalRF Remix App](https://github.com/Eng1n33r/flipperzero-firmware/blob/dev/documentation/UniRFRemix.md)

<br>
<br>

# Where I can find IR, SubGhz, ... DBs, and other stuff?
## [Awesome Flipper Zero - Github](https://github.com/djsime1/awesome-flipperzero)
## [UberGuidoZ Playground - Large collection of files - Github](https://github.com/UberGuidoZ/Flipper)

<br>
<br>

# Links

* Unofficial Discord: [discord.gg/58D6E8BtTU](https://discord.gg/58D6E8BtTU)
* Docs by atmanos / How to write your own app (outdated): [https://flipper.atmanos.com/docs](https://flipper.atmanos.com/docs/your-first-program/intro)

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
- `site_scons`      - Build helpers
- `scripts`         - Supplementary scripts and python libraries home

Also pay attention to `ReadMe.md` files inside of those directories.
