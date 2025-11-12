# Firmware update on Developer Board {#dev_board_fw_update}

It's important to regularly update your Developer Board to ensure that you have access to the latest features and bug fixes. This page will guide you through the necessary steps to update the firmware of your Developer Board.

> [!note]
> This guide assumes that you're familiar with the basics of the command line. If you're new to it, we recommend checking out these [Windows](https://learn.microsoft.com/en-us/powershell/scripting/learn/ps101/01-getting-started?view=powershell-7.4) or [macOS/Linux](https://ubuntu.com/tutorials/command-line-for-beginners#1-overview) command line tutorials.

If you’re not, please refer to the [Windows](https://www.digitalcitizen.life/command-prompt-how-use-basic-commands/) or [MacOS / Linux](https://ubuntu.com/tutorials/command-line-for-beginners#1-overview) command line tutorials.

## Step 1. Install the micro Flipper Build Tool

[micro Flipper Build Tool (uFBT)](https://pypi.org/project/ufbt/) is a cross-platform tool developed and supported by our team that enables basic development tasks for Flipper Zero, such as building and debugging applications, flashing firmware, creating VS Code development configurations, and flashing firmware to the Wi-Fi Developer Board.

**On Linux & macOS:**

1. Open a terminal.
2. Install `pipx` by following the instructions on the [official website](https://pipx.pypa.io/stable/installation/).
3. Restart the terminal.
4. Install `ufbt`:
    ```
    pipx install ufbt
    ```

**On Windows:**

1. Download the latest version of Python on [the official website](https://www.python.org/downloads/windows/) and install it.
2. Open PowerShell.
3. Install `pipx`:
    ```
    py -m pip install --user pipx
    ```
4. Add `pipx` to PATH:
    ```
    py -m pipx ensurepath
    ```
5. Restart PowerShell.
6. Install `ufbt`:
    ```
    pipx install ufbt
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

    3.2. Press and release the **RESET** button while holding the **BOOT** button.
    
    3.3. Release the **BOOT** button.
    \image html https://cdn.flipper.net/Flipper_Zero_devboard_bootloader.jpg width=700

4. Repeat the first command above (listing serial devices) and view the name of your Developer Board that appeared in the list.

***

## Step 3. Flash the firmware

**On Linux & macOS:**

```
python3 -m ufbt devboard_flash
```

**On Windows:** Run the following command in PowerShell:

```
py -m ufbt devboard_flash
```

You should see the following message: `WiFi board flashed successfully`.

### If flashing fails

Occasionally, you might get an error message during the flashing process, such as:

```
A fatal error occurred: Serial data stream stopped: Possible serial noise or corruption.
```

*or*

```
FileNotFoundError: [Errno 2] No such file or directory: '/dev/cu.usbmodem01'
```

To fix it, try doing the following:

- Disconnect the Developer Board from your computer, then reconnect it. After that, switch your Developer Board to Bootloader mode once again, as described in Step 2.

- Use a different USB port on your computer.

- Use a different USB-C cable.

***

## Step 4. Finish the installation

1. Reboot the Developer Board by pressing the **RESET** button.
    \image html https://cdn.flipperzero.one/Flipper_Zero_Wi-Fi_devboard_reboot_after_flashing.jpg width=700

2. Disconnect and reconnect the USB-C cable.

    You've successfully updated the firmware of your Developer Board!

If you followed the **Get started with the Devboard** guide, you're ready for the next step: [Step 3. Plug the Devboard into Flipper Zero](#dev_board_get_started_step-3).

