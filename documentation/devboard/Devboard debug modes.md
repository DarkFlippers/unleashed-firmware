# Devboard debug modes {#dev_board_debug_modes}

The Wi-Fi Devboard for Flipper Zero supports **Black Magic** and **DAPLink** debug modes, and you can switch between them depending on your needs. Note that available modes depend on connection:

- **Wi-Fi:** Only **Black Magic** mode is available.
- **USB:** Switch between **Black Magic** (default) and **DAPLink**. Learn more about switching debug modes for USB connection below.

> [!note]
> Black Magic mode doesn't support RTOS threads, but you can still perform other debugging operations.

***

## Switching debug modes for USB connection

Switching debug modes for working via USB has to be done wirelessly (yes, you read that correctly). Additionally, depending on how the Devboard wireless connection is configured, you may need to follow different steps for **Wi-Fi access point mode** or **Wi-Fi client mode**:

1. If the Devboard isn't connected to your Flipper Zero, turn off your Flipper Zero and connect the Developer Board, then turn the device back on.

2. Access the Devboard's web interface:

    - [Wi-Fi access point mode](#wifi-access-point)

    - [Wi-Fi client mode](#wifi-client-mode)

3. In the **WiFi** tab, click the **USB mode** option and select **BlackMagicProbe** or **DapLink**.

4. Click **SAVE**, then click **REBOOT** to apply the changes.

\image html https://cdn.flipperzero.one/Flipper_Zero_WiFi_devboard_switching_modes_CDN.jpg width=700

> [!note]
> After switching debug modes on your Devboard, remember to select the same debugger in **VS Code** in the **Run and Debug** tab, and click the ▷ **Start Debugging** button.

