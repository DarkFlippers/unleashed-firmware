# Firmware update on Developer Board {#dev_board_fw_update}

> [!IMPORTANT] 
> 
> It's important to regularly update your Developer Board to ensure that you have access to the latest features and bug fixes. 
> This tutorial will guide you through the necessary steps to update the firmware of your Developer Board.

This tutorial assumes that you're familiar with the basics of the command line. 

If you’re not, please refer to the [Windows](https://www.digitalcitizen.life/command-prompt-how-use-basic-commands/) or [MacOS / Linux](https://ubuntu.com/tutorials/command-line-for-beginners#1-overview) command line tutorials.

## Installing the micro Flipper Build Tool

Micro Flipper Build Tool (uFBT) is a cross-platform tool that enables basic development tasks for Flipper Zero, such as building and debugging applications, flashing firmware, and creating VS Code development configurations.

Install uFBT on your computer by running the following command in the Terminal:

**For Linux & macOS:**

```bash
python3 -m pip install --upgrade ufbt
```

**For Windows:**

```powershell
python -m pip install --upgrade ufbt
```

If you want to learn more about uFBT, visit [the project's page](https://pypi.org/project/ufbt/).

## Connecting the Developer Board to your computer

1. List all of the serial devices on your computer.

    **Windows**

    On Windows, go to Device Manager and expand the Ports (COM & LPT) section.

    **macOS**

    On macOS, you can run the following command in the Terminal:
    ```bash
    ls /dev/cu.*
    ```

    **Linux**

    On Linux, you can run the following command in the Terminal:
    ```text
    ls /dev/tty*
    ```
    View the devices in the list.
2. Connect the Developer Board to your computer using a USB-C cable.
![The Developer Board in Wired mode](https://github.com/user-attachments/assets/d13e4e90-d83d-45bf-8787-6eadba590795)
4. Switch your Developer Board to Bootloader mode:
    3.1.  Press and hold the **BOOT** button.
    3.2.  Press the **RESET** button while holding the **BOOT** button.
    3.3. Release the **BOOT** button.
   ![You can easily switch the Dev Board to Bootloader mode](https://github.com/user-attachments/assets/aecc957f-f37b-4bec-af9f-9efd4837152e)
6. Repeat Step 1 and view the name of your Developer Board that appeared in the list.

    For example, on macOS:

    ```shell
    /dev/cu.usbmodem01
    ```

## Flashing the firmware

To flash the firmware onto your Developer Board, run the following command in the terminal:

```shell
python3 -m ufbt devboard_flash
```

You should see the following message: `WiFi board flashed successfully`.

## If flashing failed

If you get an error message during the flashing process, such as this:

```shell
A fatal error occurred: Serial data stream stopped: Possible serial noise or corruption.
```

Or this:

```shell
FileNotFoundError: [Errno 2] No such file or directory: '/dev/cu.usbmodem01'
```

Try doing the following:
* Disconnect the Developer Board from your computer, then reconnect it.
* Use a different USB port on your computer.
* Use a different USB-C cable.

## Finishing the installation

After flashing the firmware:
1. Reboot the Developer Board by pressing the **RESET** button. ![Reset the Developer Board](https://github.com/user-attachments/assets/7527dd7b-eaa5-4fac-8d67-7ba52e552756)
3. Disconnect and reconnect the USB-C cable.

The Developer Board should appear as a serial device on your computer. Now, you can use it with the Black Magic Debug client of your choice.
