Flipper Zero can connects to host system via USB 2.0 in several possible modes. When USB connected to host, the menu for choosing USB mode appears:

- Serial port (Default mode)
- Firmware Update (reboot to bootloader)
- Mass storage, mount SD card filesystem **TODO: only SD Card filesystem or not?**
- Bad USB mode (HID device emulation)
- USB NFC Reader

# Serial port (Default mode)

Activating when Flipper Zero operates in normal mode. Allow to communicate with PC from any application or forward commands from UART/I2C/SPI interfaces from external GPIO. When `Serial Port` mode activated, the USB port icon brings shown in status bar. 

![](./../../wiki_static/ui/status-bar-usb.png)

# Firmware Update (Bootloader DFU Mode)

![](./../../wiki_static/ui/USB-firmware-update-mode.jpg)

To activate `Firmware Update` mode Flipper Zero must reboot to bootloader. Firmware mode activating only when chosen from menu or triggered from desktop application via `Serial Port` mode.

# Bad USB mode

Flipper Zero can emulate a USB slave device, allowing it to be recognized by the computer as a regular input device, such as HID keyboard just as USB Rubber Ducky. You can write your own keyboard payloads to type any key sequence, as well as fuzzing USB stack on a target device.

Allow user to run scripts from menu. User should choose script before connecting to victim PC.  

1. User selects payload on filesystem
2. Press on it with central button, then payload executed, if here a USB connection. Otherwise, it got message "Awaiting connection"
3. After connection established, it shows progress of execution
4. At end of execution, it says "ok", user can start script again by pressing central button, or return to list payloads by pressing "back"

- Mass storage, mount SD card filesystem