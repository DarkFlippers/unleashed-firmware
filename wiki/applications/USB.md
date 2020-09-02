As we all know, computers completely trust connected input devices like mouse and keyboard. Flipper Zero can emulate a USB slave device, allowing it to be recognized by the computer as a regular input device, such as HID keyboard or Ethernet adapter, just as USB Rubber Ducky. You can write your own keyboard payloads to type any key sequence, as well as fuzzing USB stack on a target device.

## Flashing firwmare (HID mode)
Firmware mode activating only when chosen from menu.

## Bad USB mode
Allow user to run scripts from menu. User should choose script before connecting to victim PC.
1. User selects payload on filesystem
2. Press on it with central button, then payload executed, if here a USB connection. Otherwise, it got message "Awaiting connection"
3. After connection established, it shows progress of execution
4. At end of execution, it says "ok", user can start script again by pressing central button, or return to list payloads by pressing "back"

## USB Serial mode
GPIO can act as UART/I2C/SPI so user can use Flipper as PC serial adapter.

# UI


### Firmware update

### Bad USB
* Payloads
* Settings
