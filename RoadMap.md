# RoadMap for RM BUILD
### This is more of a project wish list...
- It is open to whomever wants to build and PR. You get all the credit if you can create it! :D
- I likely won't have time to cover all these items.
- Some of these would make great sample projects to learn how to make apps for flipper.
- I tried to organize them in terms of difficulty.

## WISH LIST ITEMS:
- Battery Type moved to Power Settings
- `Notepad` APP to allow taking quick notes and saving to SD.
- - Also can possibly open/edit .md,.txt,.fmf and other text friendly formats
- `Write URL to NFC` APP to allow creating URL NFC tags from only the flipper
- - Also can possibly support larger URLs than the ones in samples (due to length limits on NFC types)
- - Also can possibly create other types of tags, like WIFI configurations
- `Doplhin SubGHz Chat` APP to allow listening for and viewing SubGHz messages and sending SubGHz messages
- - Could also be set to transmit a message every X seconds/minutes until another Flipper responds
- `Bluetooth Audio` APP to allow flipper to pair to external speaker or headphones for mp3 playback
- - Also can possible pair to speaker via NFC tap, here are some possible resources:
- - - https://github.com/urish/circuitpython-mp3-ble
- - - https://github.com/averyling82/wifi-bt-audio
- - - https://github.com/YetAnotherElectronicsChannel/ESP32_Bluetooth_Audio_Receiver
- - - https://github.com/redchenjs/bluetooth_visual_speaker_esp32
- - - https://github.com/cefali9154/BluetoothPlayer
- - - https://github.com/jujax/lyrat_passthru
- - Also can support playlists / playlist files
- `Bluetooth OpenHayStack` for Flipper, changes current bluetooth configuration to transmit Bluetooth Open Haystack packet as needed for the device to be findable when settings for Bluetooth is set to ON in settings and OpenHayStack is set to ON in settings.
- - Not sure if could possiblt also serve to find devices
- - Example resource of OpenHayStack as an alternative to Bluetooth On/Off: https://github.com/AlexStrNik/flipperzero-firmware/tree/dev
- `Tamagochi` Save State
- Skip to next or previous SubGHz scan file from emulation screen using UP/DOWN

# 
# 
# RoadMap for ORIGINAL FIRMWARE
# Where we are (0.x.x branch)
Our goal for 0.x.x branch is to build stable usable apps and API.
First public release that we support in this branch is 0.43.1. Your device most likely came with this version.
You can develop applications but keep in mind that API is not final yet.

## What's already implemented

**Applications**

- SubGhz: all most common protocols, reading RAW for everything else
- 125kHz RFID: all most common protocols
- NFC: reading/emulating Mifare Ultralight, reading MiFare Classic and DESFire, basic EMV, basic NFC-B,F,V
- Infrared: all most common RC protocols, RAW format for everything else
- GPIO: UART bridge, basic GPIO controls
- iButton: DS1990, Cyfral, Metacom
- Bad USB: Full USB Rubber Ducky support, some extras for windows alt codes
- U2F: Full U2F specification support

**Extras**

- BLE Keyboard
- Snake game

**System and HAL**

- Furi Core
- Furi HAL 

# Where we're going (Version 1)

Main goal for 1.0.0 is to provide first stable version for both Users and Developers.

## What we're planning to implement in 1.0.0

- Loading applications from SD (tested as PoC, work scheduled for Q2)
- More protocols (gathering feedback)
- User documentation (work in progress)
- FuriCore: get rid of CMSIS API, replace hard real time timers, improve stability and performance (work in progress)
- FuriHal: deep sleep mode, stable API, examples, documentation (work in progress)
- Application improvements (a ton of things that we want to add and improve that are too numerous to list here)

## When will it happen and where I can see the progress?

Release 1.0.0 will most likely happen around the end of Q3

Development progress can be tracked in our public Miro board:

https://miro.com/app/board/uXjVO_3D6xU=/?moveToWidget=3458764522498020058&cot=14
