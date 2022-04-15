# RoadMap

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
