# RoadMap

# Where we are (0.x.x branch)

Our goal for 0.x.x branch is to build stable, usable apps and API.
The first public release in this branch is 0.43.1.

## What's already implemented

**System and HAL**

- Furi Core
- Furi HAL
- Loading applications from SD

**Applications**

- SubGhz: all most common protocols, reading RAW for everything else
- 125kHz RFID: all most common protocols
- NFC: reading/emulating Mifare Ultralight, reading MiFare Classic and DESFire, basic EMV, basic NFC-B/F/V
- Infrared: all most common RC protocols, RAW format for everything else
- GPIO: UART bridge, basic GPIO controls
- iButton: DS1990, Cyfral, Metacom
- Bad USB: Full USB Rubber Ducky support, some extras for windows alt codes
- U2F: Full U2F specification support

**External applications**

- Bluetooth
- Snake game

# Where we're going (Version 1)

The main goal for 1.0.0 is to provide the first stable version for both Users and Developers.

## What we're planning to implement in 1.0.0

- More protocols (gathering feedback)
- User documentation (work in progress)
- FuriHal: deep sleep mode, stable API, examples, documentation (work in progress)
- Application improvements (a ton of things that we want to add and improve that are too numerous to list here)

## When will it happen, and where can I see the progress?

Release 1.0.0 will most likely happen around the end of 2023Q1

Development progress can be tracked in our public Miro board:

https://miro.com/app/board/uXjVO_3D6xU=/?moveToWidget=3458764522498020058&cot=14
