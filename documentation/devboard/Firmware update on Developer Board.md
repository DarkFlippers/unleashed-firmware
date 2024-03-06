# Firmware update on Developer Board {#dev_board_fw_update}

It's important to regularly update your Developer Board to keep it up to date. This tutorial will guide you through the necessary steps to successfully update the firmware of your Developer Board.

This tutorial assumes that you're familiar with the basics of the command line. If you’re unfamiliar with the command line, please refer to the [Windows](https://www.digitalcitizen.life/command-prompt-how-use-basic-commands/) or [MacOS/Linux](https://ubuntu.com/tutorials/command-line-for-beginners#1-overview) command line tutorials.

***

## Downloading the latest firmware

The first thing you need to do is to download the latest Developer Board firmware.

To get the latest pre-built firmware, do the following:

1. Go to the [Update Server page](https://update.flipperzero.one/builds/blackmagic-firmware).
![The Update Server page hosts different versions of the Developer Board firmware](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/gIXVO9VrE4LK05CmcMSSD_monosnap-miro-2023-07-19-17-36-23.jpg)

   There, you can find the following version of the Developer Board firmware:

   * **Release:** The most stable version of the firmware, which went through rigorous testing. The Release firmware version has the following format: **X.Y.Z/**, where X, Y, and Z are the build numbers. We recommend installing this version of the firmware.

   * **Release-candidate:** The firmware version that hasn't been tested yet and may contain bugs. The Release-candidate firmware version has the following format: **X.Y.Z-rc/**, where X, Y, and Z are the build numbers.

   * **Development:** The firmware version which builds every day and contains the latest features but might be unstable.

2. Open the folder with the latest Release firmware and download the `blackmagic-firmware-s2-full-X.Y.Z.tgz` file.

***

## Extracting the firmware

After downloading the firmware archive, extract it into a folder:

* On Windows, you can use any archive manager for this, for example, [7-Zip](https://www.7-zip.org/).

* On MacOS and Linux, you can use the `tar` command:

    ```text
    tar -xzf blackmagic-firmware-s2-full-X.Y.Z.tgz -C <destination_directory>
    ```

Don't forget to replace `X.Y.Z` with the actual version number and set the destination directory!

***

## Installing the prerequisites for flashing

Install the tools below if you haven't already.

### Python

Download and install [Python3](https://www.python.org/downloads/). Make sure to check the “Add Python to PATH” option during installation.

### pip

To install the pip package manager, run the following command in the Terminal:

```text
python3 -m ensurepip --upgrade
```

If this command fails, please refer to the [official pip documentation](https://pip.pypa.io/en/stable/installation/) for alternative installation methods.

### esptool

esptool is a command-line utility for flashing ESP8266 and ESP32 microcontrollers, including the ESP32-S2 in your Developer Board.

To install esptool, run the following command in the Terminal:

```text
pip3 install esptool
```

If this command fails, try using **pip** instead of **pip3**. If this didn’t help, please refer to the [official esptool installation manual](https://docs.espressif.com/projects/esptool/en/latest/esp32/installation.html).

***

## Connecting the Developer Board to your computer

1. List all of the serial devices on your computer.

   * ***Windows***

     On Windows, go to Device Manager and expand the Ports (COM & LPT) section.

   * ***macOS***

     On macOS, you can run the following command in the Terminal:

        ```text
        ls /dev/cu.*
        ```

   * ***Linux***

     On Linux, you can run the following command in the Terminal:

        ```text
        ls /dev/tty*
        ```

    View the devices in the list.

2. Connect the Developer Board to your computer using a USB-C cable.\
![The Developer Board in Wired mode](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/Aq7gfMI-m_5H6sGGjwb4I_monosnap-miro-2023-07-19-19-47-39.jpg)

3. Switch your Developer Board to Bootloader mode:

    3.1. Press and hold the **BOOT** button.
  
    3.2. Press the **RESET** button while holding the **BOOT** button.
  
    3.3. Release the **BOOT** button.
![You can easily switch the Dev Board to Bootloader mode](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/KynP9iT6sJ3mXLaLyI82__image.png)

4. Repeat Step 1 and view the name of your Developer Board that appeared in the list.

    For example, on macOS:

    ```text
    /dev/cu.usbmodem01
    ```

***

## Flashing the firmware

### Getting the flash command

1. Run the Terminal and navigate to the folder with the extracted firmware.

2. Run the following command to read the file with the flash command:

    ```text
    cat flash.command
    ```

    If you see a similar output, you can proceed to the Flashing step:

    ```text
    esptool.py -p (PORT) -b 460800 --before default_reset --after hard_reset --chip esp32s2 write_flash --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 bootloader.bin 0x10000 blackmagic.bin 0x8000 partition-table.bin
    ```

    Don't use the exact command above for your Developer Board in the next step since it's just an example and may not match your firmware version!

    If you get an error, ensure you’re in the correct directory and extracted the firmware archive correctly.

***

### Flashing

1. Copy the command you got from the previous step and replace the `(PORT)` part with the name of the serial device you learned earlier.

    For Windows, replace `(PORT)` with the COM port number—for example, `COM3`.

2. Run the command in the Terminal.

    Your command should look similar to this:

    ```text
    esptool.py -p /dev/cu.usbmodem01 -b 460800 --before default_reset --after hard_reset --chip esp32s2 write_flash --flash_mode dio --flash_freq 80m --flash_size 4MB 0x1000 bootloader.bin 0x10000 blackmagic.bin 0x8000 partition-table.bin
    ```

    If you get an error, ensure that you’ve entered the correct serial device name and that the Developer Board is in Bootloader mode.

3. Wait till the firmware flashing is over. The flashing process takes about 30 seconds.

    The Terminal output should look similar to this:

    ```text
    esptool.py v4.6.1
    Serial port /dev/cu.usbmodem01
    Connecting...
    Chip is ESP32-S2 (revision v0.0)
    Features: WiFi, No Embedded Flash, No Embedded PSRAM, ADC and temperature sensor 
    calibration in BLK2 of efuse V2
    Crystal is 40MHz
    MAC: 00:11:22:33:44:55
    Uploading stub...
    Running stub...
    Stub running...
    Changing baud rate to 460800
    Changed.
    Configuring flash size...
    Flash will be erased from 0x00001000 to 0x00004fff...
    Flash will be erased from 0x00010000 to 0x000ecfff...
    Flash will be erased from 0x00008000 to 0x00008fff...
    Compressed 13248 bytes to 9298...
    Wrote 13248 bytes (9298 compressed) at 0x00001000 in 0.3 seconds (effective 402.7 kbit/s)...
    Hash of data verified.
    Compressed 904288 bytes to 562550...
    Wrote 904288 bytes (562550 compressed) at 0x00010000 in 6.7 seconds (effective 1076.5 kbit/s)...
    Hash of data verified.
    Compressed 3072 bytes to 124...
    Wrote 3072 bytes (124 compressed) at 0x00008000 in 0.1 seconds (effective 360.8 kbit/s)...
    Hash of data verified.
    Leaving...
    Hard resetting via RTS pin...
    ```

    If the Terminal output has these two lines at the end, your Developer Board has been successfully updated:

    ```text
    Leaving...
    Hard resetting via RTS pin...
    ```

    If you get this warning, you can safely ignore it:

    ```text
    WARNING: ESP32-S2 (revision v0.0) chip was placed into download mode using GPIO0.
    esptool.py can not exit the download mode over USB. To run the app, reset the chip manually.
    To suppress this note, set --after option to 'no_reset
    ```

#### If flashing failed

If you get an error message during the flashing process, such as:

```text
A fatal error occurred: Serial data stream stopped: Possible serial noise or corruption.
```

or

```text
FileNotFoundError: [Errno 2] No such file or directory: '/dev/cu.usbmodem01'
```

Try doing the following:

* Disconnect the Developer Board from your computer, then reconnect it.

* Use a different USB port on your computer.

* Use a different USB-C cable.

***

## Finishing the installation

After flashing the firmware, you can reboot the Developer Board by pressing the **RESET** button.

![Reset the Developer Board](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/rcQeKARgrVwa51tLoo-qY_monosnap-miro-2023-07-20-18-29-33.jpg)

The Developer Board should appear as a serial device on your computer. Now, you can use it with the Black Magic Debug client of your choice.
