# FindMy Flipper - FindMy SmartTag Emulator

This app extends the functionality of the FlipperZero's bluetooth capabilities, enabling it to act as an Apple AirTag or Samsung SmartTag, or even both simultaneously. It utilizes the FlipperZero's BLE beacon to broadcast a SmartTag signal to be picked up by the FindMy Network. I made this to serve as a versatile tool for tracking purposes, offering the ability to clone existing tags, generate OpenHaystack key pairs for integration with Apple's FindMy network, and tune the device's beacon broadcast settings.

## Features

1. Tag Emulation: Clone your existing Apple AirTag or Samsung SmartTag to the FlipperZero, or generate a key pair for use with the FindMy network without owning an actual AirTag.
2. Customization: Users can adjust the interval between beacon broadcasts and modify the transmit power to suit their needs, optimizing for both visibility and battery life.
3. Efficient Background Operation: The app is optimized to run in the background, ensuring that your FlipperZero can still be tracked with minimal battery usage and without stopping normal use.

## Usage Guide

### Step 1: Installation
- **Option A:** Use the released/precompiled firmware appropriate (FAP) for your device.
- **Option B:** Build the firmware yourself using `fbt/ufbt`.
- Both Installation options require you to be running a dev build of firmware. When release gets access to the extra BLE beacon this will change, thank you!
- All firmware should now work with main branch, including icons
  
### Step 2: Obtaining SmartTag Data

#### Option A: Cloning Existing Tag (Preferred and allows you to track without additional setup)
1. **Pair a Tag:** First, pair an AirTag or Samsung SmartTag with your device.
2. **Enter 'Lost' Mode:** Keep the tag away from the device it's registered to for approximately 15 minutes.
3. **Download nrfConnect:** Install nrfConnect from the Google Play Store. (Apple version doesn't reveal the needed Raw data, looking for a workaround)
4. **Filter and Scan:**
   - Open the app, click on filters, and exclude all except for the brand of your tag (Apple/Samsung).
   - Adjust the RSSI to the lowest setting (-40 dBm).
   - Initiate a scan. Wait for your SmartTag to appear as a "FindMy" device.
5. **Capture Data:** Click **Raw** or **View Raw** to capture your **payload** and note your tag's **MAC Address**. Immediately remove the tag's battery to prevent key/MAC rotation.
6. **Enter Data in FlipperZero App:** Input the captured **payload** and **MAC Address** into the FlipperZero app.

#### Option B: Open Haystack Method
1. **Generate a Tag:** Download the `generate_keys.py` file and execute it in your terminal. (You will need cryptography ```python3 -m pip install cryptography```)
2. **Follow Prompts:** During execution, you'll be prompted for inputs. By the end, you'll obtain a **Private Key**, **Public Key**, **Payload**, and **MAC Address**.
   - **Private Key** is necessary to receive location reports from Apple.
   - **MAC Address** should be registered in the FlipperZero app:
     1. Open the app and navigate to the config menu.
     2. Choose "register tag" and enter the MAC Address when prompted.
     3. A payload dialog will appear next. Enter your **Payload** here.
     4. Click save.
3. **Configuration Completion:** With this setup, your device is ready for Open Haystack. Proceed with the specific steps for Open Haystack or MaclessHaystack based on your setup.
   - Don't Own a Mac: https://github.com/dchristl/macless-haystack or https://github.com/Chapoly1305/FindMy
   - Own a Mac: https://github.com/seemoo-lab/openhaystack

To use OpenHayStack for tracking, you must use MacOS lower than version 14 (Mail Plug-in Incompetiablity of MacOS 14+ seemoo-lab/openhaystack#224). If you do own a device, I believe a convertor script can be provided without much of effort. If you do not own a Mac device or the system has been upgraded to 14 and beyond. The alternative solution includes,

    https://github.com/dchristl/macless-haystack (recommended in README)
    https://github.com/Chapoly1305/FindMy (a project uses python and docker to provide location lookup as a backend service)

## Setting Up on Mac with OpenHayStack (OHS) App -- If you own a Mac instructions

Follow these steps to get everything working on a Mac using the latest version of the OpenHayStack app.
Thanks to Wr3nch for the help

### Step 1: Create a New Device
- Start by creating a new device in the OpenHayStack app, but **do not deploy** it immediately after creation.

### Step 2: Export Configuration
- Choose to **EXPORT** the configuration by selecting "all accessories as file." To simplify, ensure you only have one entry in the list before exporting.
- It is crucial that the export format is in JSON.

### Step 3: Modify the JSON File
Open the exported JSON file in a text editor and make the following changes:
- **Left OHS, Right keys from my ```generate_keys.py``` script:**
    - `symmetricKey` should be set to the `Hashed adv key`.
    - `privateKey` should be replaced with your `Private Key`.
    - `oldestRelevantSymmetricKey` should also use the `Hashed adv key`.
- Additionally, update the following attributes to `true`:
    - `"isDeployed": true`
    - `"isActive": true`

### Step 4: Re-import the Configuration
- After saving your changes to the JSON file, re-import it back into OpenHayStack.

### Step 5: Adjust Settings in OHS App
- In the OpenHayStack Mac App, navigate to the top bar and change the time setting from `1 Day` to `30min`.
- Give it some time to process and apply the new settings.

By following these steps, you should have your device set up and ready to go with OpenHayStack on a Mac.
****

### Step 3: Configuration on the FlipperZero
- Upon launching the app, open the config menu and either click ```Import Tag From File``` or ```Register Tag Manually```. Put your generated .keys file onto the FlipperZero SD card inside the AppsData/FindMyFlipper folder to import from file. Or you can manually enter the tag information. When using the cloning method, you can export a .txt file from nrfConnect (click save button) amd place that in the same folder in order to import.

### Step 4: Tracking
- Once the app is configured, your FlipperZero can be tracked using the relevant platform's tracking service (FindMy app for Apple devices, SmartThings for Samsung devices, and respective web browsers). If using generated keys and OpenHaystack then you can track on the OHS app or via the Macless Haystack setup. Links to both are above


Customization

- Beacon Interval: Adjust how frequently your FlipperZero broadcasts its presence.
- Transmit Power: Increase or decrease the signal strength to balance between tracking range and battery life.

Background Use

The app is designed to have a negligible impact on battery life, even when running in the background. This allows for continuous tracking without the need for frequent recharging.

Compatibility

- Apple devices for AirTag tracking via the FindMy network.
- Any device that supports Samsung SmartTag tracking, including web browsers (previously FindMyMobile).

Thanks

- Huge thanks to all the people that contributed to the OpenHaystack project, supporting projects, and guides on the subject. This wouldn't be a thing without any of you! Special thanks to WillyJL for helping get the app input working and overall overhaul of the apps functions!

Legal and Privacy

This app is intended for personal and educational use. Users are responsible for complying with local privacy laws and regulations regarding tracking devices. The cloning and emulation of tracking tags should be done responsibly and with respect to the ownership of the original devices.

Disclaimer

This project is not affiliated with Apple Inc. or Samsung. All product names, logos, and brands are property of their respective owners. Use this app responsibly and ethically.
