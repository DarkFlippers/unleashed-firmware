# Key Combos

There are times when your flipper feels blue and don't respond to your commands.
In that case you may find this guide useful.


## Basic Combos


### Hardware Reset

- Press `LEFT` and `BACK` and hold for couple seconds
- Release `LEFT` and `BACK`

This combo performs hardware reset by pulling MCU reset line down.
Main components involved: Keys -> DD8(NC7SZ32M5X, OR-gate) -> DD1(STM32WB55, MCU)

There is 1 case when it's not working:

- MCU debug block is active and holding reset line from inside.


### Hardware Power Reset

- Disconnect USB and any external power supplies
- Disconnect USB once again
- Make sure that you've disconnected USB and any external power supplies
- Press `BACK` and hold for 30 seconds (Only will work with USB Disconnected)
- If you have not disconnected USB, then disconnect USB and repeat previous step
- Release `BACK` key

This combo performs reset by switching SYS power line off and then on.
Main components involved: Keys -> DD6(bq25896, charger)

There is 1 case when it's not working:

- Power supply is connected to USB or 5V_ext


### Software DFU

- Press `LEFT` on boot to enter DFU with flipper boot-loader

There is 1 case when it's not working:

- Flipper Boot-loader is damaged or absent


### Hardware DFU

- Press `OK` on boot to enter DFU with ST boot-loader

There is 1 case when it's not working:

- Option Bytes are damaged or set to ignore `OK` key


## DFU Combos


### Hardware Reset + Software DFU

- Press `LEFT` and `BACK` and hold for couple seconds
- Release `BACK`
- Device will enter DFU with indication (Blue LED + DFU Screen)
- Release `LEFT`

This combo performs hardware reset by pulling MCU reset line down.
Then `LEFT` key indicates to boot-loader that DFU mode requested.

There are 2 cases when it's not working:

- MCU debug block is active and holding reset line from inside
- Flipper Boot-loader is damaged or absent


### Hardware Reset + Hardware DFU

- Press `LEFT` and `BACK` and `OK` and hold for couple seconds
- Release `BACK` and `LEFT`
- Device will enter DFU without indication

This combo performs hardware reset by pulling MCU reset line down.
Then `OK` key forces MCU to load internal boot-loader.

There are 2 cases when it's not working:

- MCU debug block is active and holding reset line from inside
- Option Bytes are damaged or set to ignore `OK` key


### Hardware Power Reset + Software DFU

- Disconnect USB and any external power supplies
- Press `BACK` and `LEFT` for 30 seconds
- Release `BACK`
- Device will enter DFU with indication (Blue LED + DFU Screen)
- Release `LEFT`
- Plug USB

This combo performs reset by switching SYS power line off and then on.
Then `LEFT` key indicates to boot-loader that DFU mode requested.

There are 2 cases when it's not working:

- Power supply is connected to USB or 5V_ext
- Flipper Boot-loader is damaged or absent


### Hardware Power Reset + Hardware DFU

- Disconnect USB and any external power supplies
- Press `BACK` and `OK` and hold for 30 seconds
- Release `BACK` and `OK`
- Device will enter DFU without indication
- Plug USB

This combo performs reset by switching SYS power line off and then on.
Then `OK` key forces MCU to load internal boot-loader.

There are 2 cases when it's not working:

- Power supply is connected to USB or 5V_ext
- Option Bytes are damaged or set to ignore `OK` key

# Alternative ways to recover your device

If none of the described methods were useful:

- Ensure battery charged
- Disconnect battery and connect again (Requires disassembly)
- Try to Flash device with ST-Link or other programmer that support SWD

If you still here and your device is not working: it's not software issue.
