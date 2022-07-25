# How to add a Keyboard layout:

### 1. Add the Keyboard codes:
- Edit the file [firmware/targets/furi_hal_include/furi_hal_usb_hid.h](https://github.com/v1nc/flipperzero-firmware/blob/dev/firmware/targets/furi_hal_include/furi_hal_usb_hid.h)
- Add the keyboard codes, like the enum `HidKeyboardKeysFR`, name it `HidKeyboardKeys[YourLanguageCode]`
- Name the code names `HID_KEYBOARD_[YourLanguageCode]_*`
- Add the ascii map like `hid_asciimap_fr`, if you followed the naming convention you can probably copy the existing ascii map and only need to change all the `HID_KEYBOARD_FR_*` to `HID_KEYBOARD_[YourLanguageCode]_*`

### 2. Apply the ascii map:
- In the same file [firmware/targets/furi_hal_include/furi_hal_usb_hid.h](https://github.com/v1nc/flipperzero-firmware/blob/dev/firmware/targets/furi_hal_include/furi_hal_usb_hid.h), add your new ascii map `hid_asciimap_[YourLanguageCode]` to the `hid_asciimaps` array. Remember the index of the new map

### 3. Activate the ascii map:
- Edit the file [applications/bad_usb/bad_usb_script.c](https://github.com/v1nc/flipperzero-firmware/blob/dev/applications/bad_usb/bad_usb_script.c)
- In the function `ducky_get_layout` add the check for your new language code with the index from the ascii map
- For example, if your new language code is `XY` and the new index in the ascii map is `2`, you would add the following code:
```
else if(strcmp(line, "XY") == 0){
        layout = 2;
}
```

You are done, that was fast, huh?

Build the firmware to test your changes.

If you think your new keyboard layout works as inteded, please create a Pull Request so others can benefit from your work :)