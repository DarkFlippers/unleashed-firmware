# Dexv  Firmware

[![Discord](https://img.shields.io/discord/975068703559409685?label=&logo=discord&logoColor=ffffff&color=7389D8&labelColor=6A7EC2)](https://discord.gg/dexvirus)

![Show me the code](https://media.discordapp.net/attachments/746304505879986267/1043574668940021760/image.png)

Welcome to [Dexv Zero](https://discord.gg/dexvirus)'s Firmware repo!
My goal is to make a really nice flipper cfw, to make it a pleasure for everyone to work with.

## Change Log

+Added EVERY apps i could find for the flipper.

+Added dumps for Nfc, Rfid, Badusb, Subghz and a bunch of music player dumps.

+Added Dexv-themed graphics to everything

# Clone the Repository

You should clone with 
```shell
$ git clone --recursive https://github.com/DXVVAY/dexv0
```

# Read the Docs

Check out details on [how to build firmware](documentation/fbt.md), [write applications](documentation/AppsOnSDCard.md), [un-brick your device](documentation/KeyCombo.md) and more in `documentation` folder made by the flipper team. 


## Install without compiling

# 1 Install dexv0-flipper-fw.tgz from releases then in Qflipper go to the sd card by pressing the document folder at the navigation bar in the qflipper app.

![image](https://user-images.githubusercontent.com/89728480/204052575-b679dbe3-4b81-4634-b2be-d5a1265e105c.png)

Then find the update folder in the root of your sd card.

![image](https://user-images.githubusercontent.com/89728480/204052595-b3541b03-9130-443e-88d8-9d08811cc632.png)

Then paste the folder in Dexvmaster1.0.tgz onto the update folder

![image](https://user-images.githubusercontent.com/89728480/204052665-b0b9aa87-10e7-4a74-873d-807b2d8b4204.png)

1. Navigate down using the flipper's directional pad or in the qflipper app 

![image](https://user-images.githubusercontent.com/89728480/204052691-841888fd-78a7-44bc-af51-8e559482c98a.png)

2. Then find the file browser in your flipper zero using qflipper or the actual flipper zero screen

![image](https://user-images.githubusercontent.com/89728480/204052726-7f95fce9-b1ac-4263-96c5-8d5848ed3c5c.png)

3. Locate the update folder in browser at the root directory of your sd card using the flipper zero screen (not Qflipper file browser)


![image](https://user-images.githubusercontent.com/89728480/204052778-d202506c-01b0-4d9a-a72f-04fbf709c5a2.png)

In the image provided i have multiple fw's but select the one called f7-update-RM420FAP in your case.

4. Find the file named update in the f7-update-RM420FAP directory and click on run in app

![image](https://user-images.githubusercontent.com/89728480/204052959-b0ce5240-bb69-40b2-af8c-88b2d0159c96.png)

5.When your done updating install Apps.rar from releases then in Qflipper go to the sd card by pressing the document folder at the navigation bar in the qflipper app

![image](https://user-images.githubusercontent.com/89728480/204055617-69bf9c93-ed4b-48b7-bdfd-ff946fddd315.png)

Then extract the folders that are in Apps.rar into the apps folder and your done.

Now you have to wait for your flipper to finish the updating/installation process then your done with a good little sick dolphin that has Dexvirus
# Update firmware

the Latest Firmware update will be uploaded on this repo.

Flipper Zero's firmware consists of two components:

- Core2 firmware set - proprietary components by ST: FUS + radio stack. FUS is flashed at factory, and you should never update it.
- Core1 Firmware - HAL + OS + Drivers + Applications.

They both must be flashed in the order described.

## With offline update package

With Flipper attached over USB:

`./fbt flash_usb`

Just building the package:

`./fbt updater_package`




# Links

* Discord: [dexvirus/discord](https://discord.gg/dexvirus)

## Games
  
- [15 (By x27)](https://github.com/x27/flipperzero-game15)
- [2048 (By OlegSchwann)](https://github.com/OlegSchwann/flipperzero-firmware/tree/hackaton/game_2048/applications/game-2048) [(Score By DevMilanIan)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/186)
- [Arkanoid (By gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins) [(Score By DevMilanIan)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/188)
- [BlackJack (By teeebor)](https://github.com/teeebor/flipper_games)
- [Chess (By Okalachev)](https://github.com/okalachev/flipperzero-firmware/tree/chess) Crashes 1st load if FW <~750KB or every load on larger FW  `Broken?`
- [Dice Roller Including SEX/WAR/8BALL/WEED DICE (By RogueMaster)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/blob/420/applications/dice/dice.c)
- [Dice (By Ka3u6y6a)](https://github.com/Ka3u6y6a/flipper-zero-dice)
- [Doom (By p4nic4ttack)](https://github.com/p4nic4ttack/doom-flipper-zero/)
- [Flappy Bird (By DroomOne)](https://github.com/DroomOne/flipperzero-firmware/tree/dev/applications/flappy_bird) [Flappy: Border hitboxes, bigger Pilars (By TQMatvey)](https://github.com/DarkFlippers/unleashed-firmware/pull/114) [Increase pilars line width to improve visibility (By ahumeniy)](https://github.com/DarkFlippers/unleashed-firmware/pull/140)
- [Game of Life (Updated to work by tgxn) (By itsyourbedtime)](https://github.com/tgxn/flipperzero-firmware/blob/dev/applications/game_of_life/game_of_life.c)
- [Heap Defence (By xMasterX)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/commit/fc776446de9fdd553b221c02668b925b689378d8) [(original by wquinoa & Vedmein)](https://github.com/Vedmein/flipperzero-firmware/tree/hd/svisto-perdelki)
- [Mandelbrot Set (By Possibly-Matt)](https://github.com/Possibly-Matt/flipperzero-firmware-wPlugins)
- [Minesweeper (By panki27)](https://github.com/panki27/minesweeper)
- [Monty Hall (By DevMilanIan)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/203)
- [Scorched Tanks (By jasniec)](https://github.com/jasniec/flipper-scorched-tanks-game)
- [Snake (By OlegSchwann)-OFW](https://github.com/flipperdevices/flipperzero-firmware/pull/829)(With updates from DrZlo13, xMasterX, QtRoS and RogueMaster) [Snake Score Saving (By JuanJakobo)](https://github.com/flipperdevices/flipperzero-firmware/pull/1922) [Turns anywhere (By TQMatvey)](https://github.com/DarkFlippers/unleashed-firmware/pull/125) [Food Spawns Anywwhere (By TQMatvey)](https://github.com/DarkFlippers/unleashed-firmware/pull/130)
- [Solitaire (By teeebor)](https://github.com/teeebor/flipper_games)
- [T-Rex (By gelin)](https://github.com/gelin/t-rex-runner) WIP
- [TAMA P1 (By GMMan)](https://github.com/GMMan/flipperzero-firmware/tree/tama-p1) requires [this rom](https://tinyurl.com/tamap1) IN `tama_p1` on SD as `rom.bin` to make it work.
- [Tanks (By Alexgr13)](https://github.com/alexgr13/flipperzero-firmware/tree/fork/dev/applications/tanks-game)
- [Tetris (By jeffplang)](https://github.com/jeffplang/flipperzero-firmware/tree/tetris_game/applications/tetris_game)
- [Tic Tac Toe (By gotnull)](https://github.com/gotnull/flipperzero-firmware-wPlugins)
- [Video Poker (By PixlEmly)](https://github.com/PixlEmly/flipperzero-firmware-testing/blob/420/applications/VideoPoker/poker.c)
- [Zombiez (Reworked By DevMilanIan)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/240) [(Original By Dooskington)](https://github.com/Dooskington/flipperzero-zombiez)

## Plugins

- [Air Mouse (By ginkage)](https://github.com/ginkage/FlippAirMouse/)
- [Authenticator/TOTP (By akopachov)](https://github.com/akopachov/flipper-zero_authenticator)
- [Bad Apple (By GMMan)](https://github.com/GMMan/flipperzero-badapple) `No working video.bin found`
- [Barcode Generator (By McAzzaMan)](https://github.com/McAzzaMan/flipperzero-firmware/tree/UPC-A_Barcode_Generator/applications/barcode_generator)
- [Bluetooth Remote (By Cutch)-OFW](https://github.com/flipperdevices/flipperzero-firmware/pull/1330)
- [BPM Tapper (By panki27)](https://github.com/panki27/bpm-tapper)
- [Calculator (By n-o-T-I-n-s-a-n-e)](https://github.com/n-o-T-I-n-s-a-n-e)
- [Ceasar Cipher (By panki27)](https://github.com/panki27/caesar-cipher)
- [Clock/Stopwatch (By CompaqDisc, Stopwatch & Sound Alert By RogueMaster)](https://gist.github.com/CompaqDisc/4e329c501bd03c1e801849b81f48ea61) [12/24HR (By non-bin)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/254) [Refactoring (By GMMan)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/256)
- [Counter (By Krulknul)](https://github.com/Krulknul/dolphin-counter)
- [DAP Link (By DrZlo13)-OFW](https://github.com/flipperdevices/flipperzero-firmware/pull/1897)
- [Deauther PWNDTOOLS V2.6.0 (By HEX0DAYS)](https://github.com/HEX0DAYS/FlipperZero-PWNDTOOLS) `Req: ESP8266` [Original](https://github.com/SpacehuhnTech/esp8266_deauther)
- [DHT Temp Monitor (By quen0n)](https://github.com/quen0n/FipperZero-DHT-Monitor) `Req: DHT11/DHT22(AM2302)/AM2301`
- [Distance Sensor (By Sanqui)](https://github.com/Sanqui/flipperzero-firmware/tree/hc_sr04)) `Req: HC-SR04` Ported/Modified by xMasterX
- [Dolphin Backup (By nminaylov)-OFW](https://github.com/flipperdevices/flipperzero-firmware/pull/1384) Modified by RogueMaster
- [Dolphin Restorer (By nminaylov)](https://github.com/flipperdevices/flipperzero-firmware/pull/1384) Cloned by RogueMaster
- [DSTIKE Deauther (By SequoiaSan)](https://github.com/SequoiaSan/FlipperZero-Wifi-ESP8266-Deauther-Module/tree/FlipperZero-Module-v2/FlipperZeroModule/FlipperZero-ESP8266-Deauth-App) `Req: ESP8266`
- [DTMF Dolphin (By litui)](https://github.com/litui/dtmf_dolphin)
- [Flashlight (By xMasterX)](https://github.com/xMasterX/flipper-flashlight)
- [GPS (By ezod)](https://github.com/ezod/flipperzero-gps) `Req: NMEA 0183`
- [HEX Viewer (By QtRoS)](https://github.com/QtRoS/flipperzero-firmware)
- [iButton Fuzzer (By xMasterX)](https://github.com/DarkFlippers/unleashed-firmware)
- [i2c Tools (By NaejEL)](https://github.com/NaejEL/flipperzero-i2ctools)
- [Lightmeter (By oleksiikutuzov)](https://github.com/oleksiikutuzov/flipperzero-lightmeter) `Req: BH1750`
- [IFTTT Virtual Button (By Ferrazzi)](https://github.com/Ferrazzi/FlipperZero_IFTTT_Virtual_Button) `Req: ESP8266 w/ IFTTT FW Flashed`
- [Metronome (By panki27)](https://github.com/panki27/Metronome)
- [Morse Code (By wh00hw)](https://github.com/DarkFlippers/unleashed-firmware/pull/144)
- [Mouse Jacker (By mothball187)](https://github.com/mothball187/flipperzero-nrf24/tree/main/mousejacker) ([Pin Out](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/420/applications/mousejacker) from nocomp/Frog/UberGuidoZ) `Req: NRF24`
- [Mouse Jiggler (By Jacob-Tate)](https://github.com/Jacob-Tate/flipperzero-firmware/blob/dev/applications/mouse_jiggler/mouse_jiggler.c) (Original By MuddleBox)
- [Multi Converter (By theisolinearchip)](https://github.com/theisolinearchip)
- [Music Beeper (By DrZlo13)](https://github.com/flipperdevices/flipperzero-firmware/pull/1189) (With Changes By qqMajiKpp/Haseo)
- [Music Player (By DrZlo13)-OFW](https://github.com/flipperdevices/flipperzero-firmware/pull/1189)
- [NFC Magic (By gornekich)](https://github.com/flipperdevices/flipperzero-firmware/pull/1966)
- [NRF Sniff (By mothball187)](https://github.com/mothball187/flipperzero-nrf24/tree/main/nrfsniff) ([Pin Out](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/420/applications/nrfsniff) from nocomp/Frog/UberGuidoZ) `Req: NRF24`
- [NRF24 Scanner v1.5 (By vad7)](https://github.com/vad7/nrf24scan)
- [Ocarina (By invalidna-me)](https://github.com/invalidna-me/flipperzero-ocarina) [Here are the LOTZ Songs](https://www.zeldadungeon.net/wiki/Ocarina_of_Time_Songs)
- [Paint (By n-o-T-I-n-s-a-n-e)](https://github.com/n-o-T-I-n-s-a-n-e)
- [Password Generator (By anakod)](https://github.com/anakod/flipper_passgen)
- [PicoPass Reader (By Bettse)](https://github.com/flipperdevices/flipperzero-firmware/pull/1366)
- [Pomodoro Timer (By sbrin)](https://github.com/sbrin/flipperzero_pomodoro)
- [RFID Fuzzer (By Ganapati)](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/pull/245) [Changes by Unleashed/xMasterX](https://github.com/DarkFlippers/unleashed-firmware)
- [RF Remix (By ESurge)](https://github.com/ESurge/flipperzero-firmware-unirfremix) [(Original By jimilinuxguy)](https://github.com/jimilinuxguy/flipperzero-universal-rf-remote/tree/028d615c83f059bb2c905530ddb3d4efbd3cbcae/applications/jukebox) [(More protocols thanks to darmiel & xMasterX)](https://github.com/darmiel/flipper-playlist/blob/feat/unirf-protocols/applications/unirfremix/unirfremix_app.c)
- [RC2014 ColecoVision (By ezod)](https://github.com/ezod/flipperzero-rc2014-coleco)
- [SAM (By Unknown)][Original?](https://github.com/ctoth/SAM)
- [Sentry Safe (By H4ckd4ddy)](https://github.com/H4ckd4ddy/flipperzero-sentry-safe-plugin) ([Pin Out](https://github.com/RogueMaster/flipperzero-firmware-wPlugins/tree/420/applications/sentry_safe) from [UberGuidoZ](https://github.com/UberGuidoZ/))
- [Signal Generator (By nminaylov)-OFW](https://github.com/flipperdevices/flipperzero-firmware/pull/1793)
- [Spectrum Analyzer (By jolcese)](https://github.com/jolcese/flipperzero-firmware/tree/spectrum/applications/spectrum_analyzer) [Updates (for testing) Thanks to theY4Kman](https://github.com/theY4Kman/flipperzero-firmware)
- [Sub-GHz Bruteforcer (By Ganapati & xMasterX)](https://github.com/derskythe/flipperzero-subbrute/tree/master)
- [Sub-GHz Playlist (By darmiel)](https://github.com/darmiel/flipper-playlist)
- [Temperature Sensor (By Mywk)](https://github.com/Mywk/FlipperTemperatureSensor) `Req: HTU2XD, SHT2X, SI702X, SI700X, SI701X or AM2320`
- [Temperature Sensor (By xMasterX)](https://github.com/DarkFlippers/unleashed-firmware/commit/9c4612e571db42f5e6123a3f159e01337453a6af) `Req: AM2320`
- [Timelapse (By theageoflove)](https://github.com/theageoflove/flipperzero-zeitraffer)
- [Tuning Fork (By besya)](https://github.com/besya/flipperzero-tuning-fork)
- [UART Echo (By DrZlo13)-OFW](https://github.com/flipperdevices/flipperzero-firmware/pull/831)
- [USB HID Autofire (By pbek)](https://github.com/pbek/usb_hid_autofire)
- [USB Keyboard (By huuck)](https://github.com/huuck/FlipperZeroUSBKeyboard)
- [WAV Player (By DrZlo13)](https://github.com/flipperdevices/flipperzero-firmware/tree/zlo/wav-player) Updated by Atmanos & RogueMaster To Work
- [WiFi (Deauther) V2 (By Timmotools)](https://github.com/Timmotools/flipperzero_esp8266_deautherv2) `Req: ESP8266` 
- [WiFi (Marauder) (By 0xchocolate)](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion) `Req: ESP32 WITH MARAUDER FLASHED`
- [WiFi Scanner v.0.4 (By SequoiaSan)](https://github.com/SequoiaSan/FlipperZero-WiFi-Scanner_Module-ESP8266) `Req: ESP8266 or ESP32`
- [Wii EC Analyser (By csBlueChip)](https://github.com/csBlueChip/FlipperZero_WiiEC)
- [Zero Tracker (By DrZZlo13)](https://github.com/DrZlo13/flipper-zero-music-tracker)

