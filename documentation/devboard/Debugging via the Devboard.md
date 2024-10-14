# Debugging via the Devboard {#dev_board_debugging_guide}

On this page, you'll learn about how debugging via the Wi-Fi Developer Board works. To illustrate this process, we'll start a debug session for Flipper Zero's firmware in VS Code using the native Flipper Build Tool.

***

## Overview

The Developer Board acts as the debug probe, which provides a bridge between the IDE (integrated development environment) with a debugger running on a host computer and the target microcontroller (in your Flipper Zero). The user controls the debugging process on the computer connected to the Developer Board via [Wi-Fi](#dev_board_wifi_connection) or [USB cable](#dev_board_usb_connection).

\image html https://cdn.flipperzero.one/Flipper_Zero_WiFi_hardware_CDN.jpg width=700

Data exchange between the Wi-Fi Developer Board and your Flipper Zero is conducted via the Serial Wire Debug interface. The following GPIO pins serve this purpose:

- **Pin 10:** Serial Wire Clock (SWCLK)

- **Pin 12:** Serial Wire Debug Data I/O (SWDIO)

To learn more about Flipper Zero pinout, visit [GPIO & modules in Flipper Docs](https://docs.flipper.net/gpio-and-modules).

***

## Prerequisites

### Step 1. Installing Git

You'll need Git installed on your computer to clone the firmware repository. If you don't have Git, install it following the [official installation guide](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git).

### Step 2. Building the firmware

Before starting debugging, you need to clone and build Flipper Zero firmware:

1. Open the **Terminal** (on Linux & macOS) or **PowerShell** (on Windows) in the directory where you want to store the firmware source code.

2. Clone the firmware repository:

    ```
    git clone --recursive https://github.com/flipperdevices/flipperzero-firmware.git
    cd flipperzero-firmware
    ```

3. Run the **Flipper Build Tool (FBT)** to build the firmware:

    ```
    ./fbt
    ```

***

## Debugging the firmware

From the **flipperzero-firmware** directory that you cloned earlier, run the following command:

```
./fbt flash
```

This will upload the firmware you've just built to your Flipper Zero via the Developer Board. After that, you can start debugging the firmware. We recommend using **VS Code** with the recommended extensions (as described below), and we have pre-made configurations for it.

To debug in **VS Code**, do the following:

1. In VS Code, open the **flipperzero-firmware** directory.

2. You should see a notification about recommended extensions. Install them.

    If there were no notifications, open the **Extensions** tab, enter `@recommended` in the search bar, and install the workspace recommendations.

3. Run the `./fbt vscode_dist` command. This will generate the VS Code configuration files needed for debugging.

4. In VS Code, open the **Run and Debug** tab and select a debugger from the dropdown menu:

    - **Attach FW (blackmagic):** Can be used via **Wi-Fi** or **USB**
    - **Attach FW (DAP):** Can be used via **USB** only

    Note that when debugging via USB, you need to make sure the selected debugger matches the debug mode on your Devboard. To check the debug mode on your Devboard, access the Devboard's web interface as described [here](#dev_board_wifi_connection) and check the **USB mode** field. If you want to use a different debug mode, enable this mode by following the steps in [Devboard debug modes](#dev_board_debug_modes).

5. If needed, flash your Flipper Zero with the `./fbt flash` command, then click the ▷ **Start Debugging** button in the debug sidebar to start the debugging session.

6. Note that starting a debug session halts the execution of the firmware, so you'll need to click the I▷ **Continue** button on the toolbar at the top of your VS Code window to continue execution.

\image html https://cdn.flipperzero.one/Flipper_Zero_Wi-Fi_devboard_VS_Code.jpg width=900

> [!note]
> If you want to use a different debug mode on your Developer Board, visit [Devboard debug modes](#dev_board_debug_modes).
>
> If you want to read logs via the Developer Board, see [Reading logs via the Devboard](#dev_board_reading_logs).
> 
> To learn about debugging in VS Code, see [VS Code official guide](https://code.visualstudio.com/docs/editor/debugging).
