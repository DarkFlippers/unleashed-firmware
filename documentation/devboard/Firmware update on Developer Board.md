# Firmware update on Developer Board {#dev_board_fw_update}

It's important to regularly update your Developer Board to ensure that you have access to the latest features and bug fixes. This page will guide you through the necessary steps to update the firmware of your Developer Board.

> [!note]
> This guide assumes that you're familiar with the basics of the command line. If you're new to it, we recommend checking out these [Windows](https://learn.microsoft.com/en-us/powershell/scripting/learn/ps101/01-getting-started?view=powershell-7.4) or [macOS/Linux](https://ubuntu.com/tutorials/command-line-for-beginners#1-overview) command line tutorials.

***

## Step 1. Install the micro Flipper Build Tool

[micro Flipper Build Tool (uFBT)](https://pypi.org/project/ufbt/) is a cross-platform tool developed and supported by our team that enables basic development tasks for Flipper Zero, such as building and debugging applications, flashing firmware, creating VS Code development configurations, and flashing firmware to the Wi-Fi Developer Board.

**On Linux & macOS:**

Run the following command in the Terminal:

```
python3 -m pip install --upgrade ufbt
```

**On Windows:**

1. Download the latest version of Python on 
2. Run the following command in the PowerShell

    ```
    py -m pip install --upgrade ufbt
    ```

***

## Step 2. Connect the Devboard to PC

To update the firmware, you need to switch your Developer Board to Bootloader mode, connect to a PC via a USB cable, and make sure that the PC detects the Developer Board:

1. List all of the serial devices on your computer.

    - **macOS:** Run the `ls /dev/cu.*` command in the Terminal.

    - **Linux:** Run the `ls /dev/tty*` command in the Terminal.

    - **Windows:** Go to **Device Manager** and expand the **Ports (COM & LPT)** section.

2. Connect the Developer Board to your computer using a USB-C cable.
    \image html https://cdn.flipperzero.one/Flipper_Zero_Wi-Fi_devboard_update_wired_connection.jpg width=700

3. Switch your Developer Board to Bootloader mode:

    3.1. Press and hold the **BOOT** button.

    3.2. Press the **RESET** button while holding the **BOOT** button.
    
    3.3. Release the **BOOT** button.
    \image html https://cdn.flipperzero.one/Flipper_Zero_Wi-Fi_devboard_reboot_to_bootloader.png width=700

4. Repeat **Step 1** and view the name of your Developer Board that appeared in the list.

***

## Step 3. Flash the firmware

**On Linux & macOS:**

```
python3 -m ufbt devboard_flash
```

**On Windows:** Run the following command in the PowerShell:

```
py -m ufbt devboard_flash
```

You should see the following message: `WiFi board flashed successfully`.

### If flashing failed

Occasionally, you might get an error message during the flashing process, such as:

```
A fatal error occurred: Serial data stream stopped: Possible serial noise or corruption.
```

*or*

```
FileNotFoundError: [Errno 2] No such file or directory: '/dev/cu.usbmodem01'
```

To fix it, try doing the following:

- Disconnect the Developer Board from your computer, then reconnect it. After that, switch your Developer Board to Bootloader mode once again, as described in 

- Use a different USB port on your computer.

- Use a different USB-C cable.

***

## Step 4. Finish the installation

1. Reboot the Developer Board by pressing the **RESET** button.
    \image html https://cdn.flipperzero.one/Flipper_Zero_Wi-Fi_devboard_reboot_after_flashing.jpg width=700

2. Disconnect and reconnect the USB-C cable.

    You've successfully updated the firmware of your Developer Board!

If you followed the **Get started with the Devboard** guide, you're ready for the next step: [Step 3. Plug the Devboard into Flipper Zero](#dev_board_get_started_step-3).

