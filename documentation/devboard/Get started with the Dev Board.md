# Get started with the Dev Board {#dev_board_get_started}

The Wi-Fi Developer Board serves as a tool to debug the Flipper Zero firmware. To debug the firmware, the initial step involves compiling the firmware from its source code. This process enables the debugging functionality within the firmware and generates all the necessary files required for debugging purposes.

> **NOTE:** Building and debugging the Flipper Zero firmware is fully supported on MacOS and Linux. Support for Windows is in beta test.

***

## Updating the firmware of your Developer Board

Update the firmware of your Developer Board before using it. For more information, visit [Firmware update on Developer Board](https://docs.flipperzero.one/development/hardware/wifi-debugger-module/update).

***

## Installing Git

You’ll need Git installed on your computer to clone the firmware repository. If you don’t have Git, install it by doing the following:

* **MacOS**

  On MacOS, install the **Xcode Command Line Tools** package, which includes Git as one of the pre-installed command-line utilities, by running in the Terminal the following command:

    ```text
    xcode-select --install
    ```

* **Linux**

  On Linux, you can install Git using your package manager. For example, on Ubuntu, run in the Terminal the following command:

     ```text
     sudo apt install git
     ```

For other distributions, refer to your package manager documentation.

***

## Building the firmware

First, clone the firmware repository:

```text
git clone --recursive https://github.com/flipperdevices/flipperzero-firmware.git
cd flipperzero-firmware
```

Then, run the **Flipper Build Tool** (FBT) to build the firmware:

```text
./fbt
```

***

## Connecting the Developer Board

The Developer Board can work in the **Wired** mode and two **Wireless** modes: **Wi-Fi access point (AP)** mode and **Wi-Fi client (STA)** mode. The Wired mode is the simplest to set up, but requires a USB Type-C cable. The Wireless modes are more complex to set up, but they allow you to debug your Flipper Zero wirelessly.

> **NOTE:** Use the following credentials when connecting to the Developer Board in **Wi-Fi access point** mode: Name: **blackmagic**, Password: **iamwitcher**

## Wired

![The Developer Board in Wired mode](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/jZdVlRTPVdSQVegzCyXp7_monosnap-miro-2023-06-22-16-28-06.jpg)

To connect the Developer Board in **Wired** mode, do the following:

1. Cold-plug the Developer Board by turning off your Flipper Zero and connecting the Developer Board, and then turning it back on.

2. On your computer, open the **Terminal** and run the following:

   * **MacOS**

        ```text
        ls /dev/cu.*
        ```

   * **Linux**

        ```text
        ls /dev/tty*
        ```

   Note the list of devices.

3. Connect the Developer Board to your computer via a USB-C cable.

4. Rerun the command. Two new devices have to appear: this is the Developer Board.

> **NOTE:** If the Developer Board doesn’t appear in the list of devices, try using a different cable, USB port, or computer.
>
> **NOTE:**  Flipper Zero logs can only be viewed when the Developer Board is connected via USB. The option to view logs over Wi-Fi will be added in future updates. For more information, visit [Reading logs via the Dev Board](https://docs.flipperzero.one/development/hardware/wifi-debugger-module/reading-logs).

## Wireless

### Wi-Fi access point (AP) mode

![The Developer Board in Wi-Fi access point mode](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/tKRTMHAuruiLSEce2a8Ve_monosnap-miro-2023-06-22-16-39-17.jpg)

Out of the box, the Developer Board is configured to work as a **Wi-Fi access point**. This means it will create its own Wi-Fi network to which you can connect. If your Developer Board doesn’t create a Wi-Fi network, it is probably configured to work in **Wi-Fi client** mode. To reset your Developer Board back to **Wi-Fi access point** mode, press and hold the **BOOT** button for 10 seconds, then wait for the module to reboot.

![You can reconfigure the Developer Board mode by pressing and holding the BOOT button](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/57eELJsAwMxeZCEA1NMJw_monosnap-miro-2023-06-22-20-33-27.jpg)

To connect the Developer Board in **Wi-Fi access point** mode, do the following:

1. Cold-plug the Developer Board by turning off your Flipper Zero and connecting the Developer Board, and then turning it back on.

2. Open Wi-Fi settings on your client device (phone, laptop, or other).

3. Connect to the network:

    * Name: **blackmagic**
    * Password: **iamwitcher**

4. To configure the Developer Board, open a browser and go to `http://192.168.4.1`.

#### Wi-Fi client (STA) mode

![The Developer Board in Wi-Fi client mode](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/xLQpFyYPfUS5Cx0uQhrNd_monosnap-miro-2023-06-23-12-34-36.jpg)

To connect the Developer Board in **Wi-Fi client** mode, you need to configure it to connect to your Wi-Fi network by doing the following:

1. Cold-plug the Developer Board by turning off your Flipper Zero and connecting the Developer Board, and then turning it back on.

2. Connect to the Developer Board in **Wi-Fi access point** mode.

3. In a browser, go to the configuration page on `http://192.168.4.1`.

4. Select the **STA** mode and enter your network’s **SSID** (name) and **password**. For convenience, you can click the **+** button to see the list of nearby networks.

5. Save the configuration and reboot the Developer Board.

![In the Wi-Fi tab, you can set the Developer Board mode](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/klbLVj8lz2bEvm7j4wRaj_monosnap-miro-2023-06-23-13-06-32.jpg)

After rebooting, the Developer Board connects to your Wi-Fi network. You can connect to the device using the mDNS name [blackmagic.local](http://blackmagic.local) or the IP address it got from your router (you’ll have to figure this out yourself, every router is different).

After connecting to your debugger via [blackmagic.local](http://blackmagic.local), you can find its IP address in the **SYS** tab. You can also change the debugger’s mode to **AP** or **STA** there.

![In the SYS tab, you can view the IP address of your Developer Board](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/5XbUptlfqzlV0p6hRUqiG_monosnap-miro-2023-06-22-18-11-30.jpg)

***

## Debugging the firmware

Open the **Terminal** in the **flipperzero-firmware** directory that you cloned earlier and run the following command:

```text
./fbt flash_blackmagic
```

This will upload the firmware you’ve just built to your Flipper Zero via the Developer Board. After that, you can start debugging the firmware using the [GDB](https://www.gnu.org/software/gdb/) debugger. We recommend using **VSCode** with the recommended extensions, and we have pre-made configurations for it.

To debug in **VSCode**, do the following:

1. In VSCode, open the **flipperzero-firmware** directory.

2. You should see a notification about recommended extensions. Install them.

    If there were no notifications, open the **Extensions** tab, enter `@recommended` in the search bar, and install the workspace recommendations.

3. In the **Terminal**, run the `./fbt vscode_dist` command. This will generate the VSCode configuration files needed for debugging.

4. In VSCode, open the **Run and Debug** tab and select **Attach FW (blackmagic)** from the dropdown menu.

5. If needed, flash your Flipper Zero with the `./fbt flash_blackmagic` command, then click the **Play** button in the debug sidebar to start the debugging session.

6. Note that starting a debug session halts the execution of the firmware, so you’ll need to click the **Continue** button on the toolbar at the top of your VSCode window to continue execution.

![Click Continue in the toolbar to continue execution of the firmware](https://archbee-image-uploads.s3.amazonaws.com/3StCFqarJkJQZV-7N79yY/lp8ygGaZ3DvWD3OSI9yGO_monosnap-miro-2023-06-23-17-58-09.jpg)

To learn about debugging, visit the following pages:

* [Debugging with GDB](https://sourceware.org/gdb/current/onlinedocs/gdb.pdf)

* [Debugging in VS Code](https://code.visualstudio.com/docs/editor/debugging)
