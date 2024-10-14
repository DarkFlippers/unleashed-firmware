# Key Combos {#key_combos}

There are times when your Flipper feels blue and doesn't respond to any of your commands due to a software issue. This guide will help you solve this problem.

## Basic combos

### Hardware reset

- Press `LEFT` and `BACK` and hold for a couple of seconds
- Release `LEFT` and `BACK`

This combo performs a hardware reset by pulling the MCU reset line down.
Main components involved: Keys → DD8(NC7SZ32M5X, OR-gate) → DD1(STM32WB55, MCU).

It won't work only in one case:

- The MCU debug block is active and holding the reset line from inside.

### Hardware Power Reset

- Disconnect the USB cable and any external power supplies
- Disconnect the USB once again
- Make sure you've disconnected the USB and any external power supplies
- Press `BACK` and hold for 30 seconds (this will only work with the USB disconnected)
- If you haven't disconnected the USB, then disconnect it and repeat the previous step
- Release the `BACK` key

This combo performs a reset by switching SYS power line off and then on.
Main components involved: Keys → DD6(bq25896, charger).

It won't work only in one case:

- Power supply is connected to USB or 5V_ext

### Software DFU

- Press `LEFT` on boot to enter DFU with Flipper boot-loader

It won't work only in one case:

- Flipper boot-loader is damaged or absent

### Hardware DFU

- Press `OK` on boot to enter DFU with ST boot-loader

It won't work only in one case:

- Option Bytes are damaged or set to ignore the `OK` key

## DFU combos

### Hardware Reset + Software DFU

- Press `LEFT` and `BACK` and hold for a couple of seconds
- Release `BACK`
- Device will enter DFU with an indication (Blue LED + DFU Screen)
- Release `LEFT`

This combo performs a hardware reset by pulling the MCU reset line down. Then, the `LEFT` key indicates to the boot-loader that DFU mode is requested.

It won't work in two cases:

- The MCU debug block is active and holding the reset line from inside
- Flipper boot-loader is damaged or absent

### Hardware Reset + Hardware DFU

- Press `LEFT`, `BACK` and `OK` and hold for a couple of seconds
- Release `BACK` and `LEFT`
- The device will enter DFU without an indication

This combo performs a hardware reset by pulling the MCU reset line down. Then, the `OK` key forces MCU to load the internal boot-loader.

It won't work in two cases:

- The MCU debug block is active and holding the reset line from inside
- Option Bytes are damaged or set to ignore the `OK` key

### Hardware Power Reset + Software DFU

- Disconnect the USB and any external power supplies
- Press `BACK` and `LEFT` for 30 seconds
- Release `BACK`
- The device will enter DFU with an indication (Blue LED + DFU Screen)
- Release `LEFT`
- Plug in the USB

This combo performs a reset by switching the SYS power line off and then on. Next, the `LEFT` key indicates to the boot-loader that DFU mode is requested.

It won't work in two cases:

- Power supply is connected to USB or 5V_ext
- Flipper boot-loader is damaged or absent

### Hardware Power Reset + Hardware DFU

- Disconnect the USB and any external power supplies
- Press `BACK` and `OK` and hold for 30 seconds
- Release `BACK` and `OK`
- The device will enter DFU without indication
- Plug in the USB

This combo performs a reset by switching the SYS power line off and then on. Next, the `OK` key forces MCU to load the internal boot-loader.

It won't work in two cases:

- Power supply is connected to USB or 5V_ext
- Option Bytes are damaged or set to ignore the `OK` key

# Alternative ways to recover your device

If none of the described methods helped you:

- Make sure the battery charged
- Disconnect the battery and connect again (requires disassembly)
- Try to flash the device with ST-Link or another programmer that supports SWD

If you're still here and your device is not working: it's not a software issue.
