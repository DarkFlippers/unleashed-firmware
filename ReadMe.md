# v1nc flipper zero firmware
<img src="https://raw.githubusercontent.com/v1nc/flipperzero-firmware/dev/logo.png" width="200" />

This is a fork of the [unleashed firmware](https://github.com/Eng1n33r/flipperzero-firmware).

It is intended to be as vanilla as possible, with functional additions but no deeper changes to the flipper software.

For any problem create an issue or check out the original firmware repo.

## Installation:
* download from [releases](https://github.com/v1nc/flipperzero-firmware/releases)
* [build](https://github.com/v1nc/flipperzero-firmware/blob/dev/documentation/HowToBuild.md) it yourself

## Fork changes:

### BadUSB support for different layouts:
This is the only firmware atm that supports the `DUCKY_LANG` keyword to change the keyboard layout.

Add `DUCKY_LANG DE` to the first or second line of the script to choose the layout.
Currently implemented:
* `US` = US (default)
* `DE` = German QWERTZ
* `FR` = AZERTY (based on [this](https://github.com/ikazeer/flipperzero-AZERTY))

**If you are writing payloads on windows you need to make sure EOL Conversion is set to LF Unix in your text editor and not windows CR LF. If you do not it will fail to run the payload.**


If you need another layout, create an issue or look [here](https://github.com/v1nc/flipperzero-firmware/blob/dev/documentation/HowToAddLayout.md) how to add new layouts yourself.


### additional plugins:
* [MouseJiggler](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/mouse_jiggler)
* [BarcodeGenerator](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/barcode_generator)
* [WAV Player](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/wav_player)
* [TouchTunes](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/jukebox)
* [HID Analyzer](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/hid_analyzer)

### additional games:
* [Dice](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/dice)
* [CHIP8 Emulator](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/chip8)
* [Flappy Bird](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/flappy_bird)
* [Video Poker](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/VideoPoker)
* [2048](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/game2048)
* [Game of Life](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/unleashed/applications/game_of_life)

### This software is for experimental purposes only and is not meant for any illegal activity/purposes. <br> We do not condone illegal activity and strongly encourage keeping transmissions to legal/valid uses allowed by law. <br> Also this software is made without any support from Flipper Devices and in no way related to official devs. 
### Please use for experimental purposes only!

## Support Unleashed Firmware developers so they can buy equipment and develop new features
* ETH/BSC/ERC20-Tokens: `0xFebF1bBc8229418FF2408C07AF6Afa49152fEc6a`
* BTC: `bc1q0np836jk9jwr4dd7p6qv66d04vamtqkxrecck9`
* DOGE: `D6R6gYgBn5LwTNmPyvAQR6bZ9EtGgFCpvv`
* LTC: `ltc1q3ex4ejkl0xpx3znwrmth4lyuadr5qgv8tmq8z9`

_logo generated with DALLE-2_
