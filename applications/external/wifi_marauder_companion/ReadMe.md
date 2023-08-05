[![FAP Build](https://github.com/0xchocolate/flipperzero-wifi-marauder/actions/workflows/build.yml/badge.svg)](https://github.com/0xchocolate/flipperzero-wifi-marauder/actions/workflows/build.yml)

# WiFi Marauder companion app for Flipper Zero

Requires a connected dev board running Marauder FW. [See install instructions from UberGuidoZ here.](https://github.com/UberGuidoZ/Flipper/tree/main/Wifi_DevBoard#marauder-install-information)

<img src="https://github.com/0xchocolate/flipperzero-wifi-marauder/blob/feature_wifi_marauder_app/screenshots/marauder-topmenu.png?raw=true" width=20% height=20% /> <img src="https://github.com/0xchocolate/flipperzero-wifi-marauder/blob/feature_wifi_marauder_app/screenshots/marauder-script-demo.png?raw=true" width=20% height=20% /> <img src="https://github.com/0xchocolate/flipperzero-wifi-marauder/blob/feature_wifi_marauder_app/screenshots/marauder-save-pcaps.png?raw=true" width=20% height=20% />

## Get the app
1. Make sure you're logged in with a github account (otherwise the downloads in step 2 won't work)
2. Navigate to the [FAP Build](https://github.com/0xchocolate/flipperzero-wifi-marauder/actions/workflows/build.yml)
   GitHub action workflow, and select the most recent run, scroll down to artifacts.
3. The FAP is built for the `dev` and `release` channels of both official and unleashed
   firmware. Download the artifact corresponding to your firmware version.
4. (Optional step to avoid confusion) Go to "Apps/GPIO" on the Flipper SD Card, delete any existing Marauder app, on some firmwares there will be a `ESP32CAM_Marauder.fap` or similar.
5. Extract `esp32_wifi_marauder.fap` from the ZIP file downloaded in step 3 to your Flipper Zero SD card, preferably under Apps/GPIO along with the rest of the GPIO apps. (If you're using qFlipper to transfer files you need to extract the content of the ZIP file to your computer before you drag it to qFlipper, as qFlipper does not support direct dragging from a ZIP file (at least on Windows)).

From a local clone of this repo, you can also build the app yourself using ufbt.

### FYI - the ESP flasher is now its own app: https://github.com/0xchocolate/flipperzero-esp-flasher


## Support

For app feedback, bugs, and feature requests, please [create an issue here](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion/issues).

You can find me (0xchocolate) on discord as @cococode#6011.

If you'd like to donate to the app development effort:  
**ETH**: `0xf32A1F0CD6122C97d8953183E53cB889cc087C9b`  
**BTC**: `bc1qtw7s25cwdkuaups22yna8sttfxn0usm2f35wc3`

Find more info about Marauder and support its developer (justcallmekoko aka WillStunForFood) here: https://github.com/justcallmekoko/ESP32Marauder

If you found the app preinstalled in a firmware release, consider supporting the maintainers!
