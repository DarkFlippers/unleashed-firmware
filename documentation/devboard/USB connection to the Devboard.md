# USB connection to the Devboard {#dev_board_usb_connection}

\image html https://cdn.flipperzero.one/Flipper_Zero_WiFi_devboard_USB_connection_CDN.jpg width=700

To connect to the Developer Board via USB, do the following:

1. If the Devboard isn't connected to your Flipper Zero, turn off your Flipper Zero and connect the Developer Board to it. Then, turn your Flipper Zero back on.

2. On your computer, check the list of serial devices.

    - **macOS:** On your computer, run `ls /dev/cu.*` in the Terminal.

    - **Linux:** On your computer, run `ls /dev/tty*` in the Terminal.

    - **Windows:** Go to **Device Manager** and expand the **Ports (COM & LPT)** section.

3. Connect the Devboard to your computer via a USB-C cable.

4. Repeat **Step 2**. Two new devices will appear — this is the Developer Board.

> [!warning]
> If the Developer Board doesn't appear in the list of devices, try using a different cable, USB port, or computer.