# Wi-Fi connection to the Devboard {#dev_board_wifi_connection}

You can connect to the Developer Board wirelessly in two ways:

- **Wi-Fi access point mode (default):** The Devboard creates its own Wi-Fi network, which you can connect to in order to access its web interface and debug via Wi-Fi. The downside is that you will need to disconnect from your current Wi-Fi network, resulting in a loss of internet connection.

- **Wi-Fi client mode:** You can connect to the Devboard through an existing Wi-Fi network, allowing you to access the Devboard web interface and debug via Wi-Fi without losing your internet connection.

Let's go over both of these modes below.

***

## Wi-Fi access point (AP) mode {#wifi-access-point}

\image html https://cdn.flipperzero.one/Flipper_Zero_WiFi_devboard_Access_Point_CDN.jpg width=700

Out of the box, the Developer Board is configured to work as a Wi-Fi access point. To connect the Developer Board in this mode, do the following:

1. Plug the Wi-Fi Devboard into your Flipper Zero by turning off your Flipper Zero and connecting the Developer Board, and then turning it back on.

2. Open Wi-Fi settings on your client device (phone, laptop, or other).

3. Connect to the network:

    Name: `blackmagic`
    Password: `iamwitcher`

    If your computer fails to find the **blackmagic** network, read the [troubleshooting section](#wifi-access-point_troubleshooting) below.

4. To access the Devboard's web interface, open a browser and go to <http://192.168.4.1> or <http://blackmagic.local>.

### If your computer fails to find the black magic network {#wifi-access-point_troubleshooting}

- Reset Wi-Fi connection on your computer.

- The Developer Board is probably configured to work in Wi-Fi client mode. → Reset your Developer Board settings to default by pressing and holding the **BOOT** button for **10 seconds**, then wait for the Devboard to reboot. After the reset, the Devboard will work in Wi-Fi access point mode.

\image html https://cdn.flipperzero.one/Flipper_Zero_Wi-Fi_devboard_reboot.jpg width=700

***

## Wi-Fi client (STA) mode {#wifi-client-mode}

\image html https://cdn.flipperzero.one/Flipper_Zero_WiFi_devboard_STA_CDN.jpg width=700

To connect the Developer Board in **Wi-Fi client** mode, you need to configure it to connect to your Wi-Fi network by doing the following:

1. Plug the Wi-Fi Devboard into your Flipper Zero by turning off your Flipper Zero and connecting the Developer Board, and then turning the device back on.

2. Connect to the Developer Board in [Wi-Fi access point](#wifi-access-point) mode.

3. In a browser, go to the Devboard's web interface at <http://192.168.4.1> or <http://blackmagic.local>.

4. Select the **STA** mode and enter your network's **SSID** (name) and **password**. For convenience, you can click the **+** button to see the list of nearby 2.4 GHz networks (5 GHz networks aren't supported).

5. Save the configuration and reboot the Developer Board.
    \image html https://cdn.flipperzero.one/Flipper_Zero_WiFi_devboard_connect_to_WiFi_CDN.jpg width=700

6. Now, you can access the Devboard's web interface at [http://blackmagic.local](https://blackmagic.local) via the existing Wi-Fi network without losing connection to the internet.
