
# BadUSB support for different layouts:

## Author: [v1nc](https://github.com/v1nc/flipperzero-firmware)

This firmware supports the `DUCKY_LANG` keyword to change the keyboard layout.

Add `DUCKY_LANG DE` to the first or second line of the script to choose the layout.
Currently implemented:
* `US` = US (default)
* `DE` = German QWERTZ
* `FR` = AZERTY (based on [this](https://github.com/ikazeer/flipperzero-AZERTY))

**If you are writing payloads on windows you need to make sure EOL Conversion is set to LF Unix in your text editor and not windows CR LF. If you do not it will fail to run the payload.**

## How to add your own layout: [instruction](https://github.com/v1nc/flipperzero-firmware/blob/dev/documentation/HowToAddLayout.md)