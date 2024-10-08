# js_badusb {#js_badusb}

# BadUSB module
```js
let badusb = require("badusb");
```
# Methods
## setup
Start USB HID with optional parameters. Should be called before all other methods.

### Parameters
Configuration object (optional):
- vid, pid (number): VID and PID values, both are mandatory
- mfr_name (string): Manufacturer name (32  ASCII characters max), optional
- prod_name (string): Product name (32  ASCII characters max), optional

### Examples:
```js
// Start USB HID with default parameters
badusb.setup();
// Start USB HID with custom vid:pid = AAAA:BBBB, manufacturer and product strings not defined
badusb.setup({ vid: 0xAAAA, pid: 0xBBBB }); 
// Start USB HID with custom vid:pid = AAAA:BBBB, manufacturer string = "Flipper Devices", product string = "Flipper Zero"
badusb.setup({ vid: 0xAAAA, pid: 0xBBBB, mfr_name: "Flipper Devices", prod_name: "Flipper Zero" });
```

## isConnected
Returns USB connection state.

### Example:
```js
if (badusb.isConnected()) {
    // Do something
} else {
    // Show an error
}
```

## press
Press and release a key.

### Parameters
Key or modifier name, key code.

See a list of key names below.

### Examples:
```js
badusb.press("a"); // Press "a" key
badusb.press("A"); // SHIFT + "a"
badusb.press("CTRL", "a"); // CTRL + "a"
badusb.press("CTRL", "SHIFT", "ESC"); // CTRL + SHIFT + ESC combo
badusb.press(98); // Press key with HID code (dec) 98 (Numpad 0 / Insert)
badusb.press(0x47); // Press key with HID code (hex) 0x47 (Scroll lock)
```

## hold
Hold a key. Up to 5 keys (excluding modifiers) can be held simultaneously.

### Parameters
Same as `press`

### Examples:
```js
badusb.hold("a"); // Press and hold "a" key
badusb.hold("CTRL", "v"); // Press and hold CTRL + "v" combo
```

## release
Release a previously held key.

### Parameters
Same as `press`

Release all keys if called without parameters

### Examples:
```js
badusb.release(); // Release all keys
badusb.release("a"); // Release "a" key
```

## print
Print a string.

### Parameters
- A string to print
- (optional) delay between key presses

### Examples:
```js
badusb.print("Hello, world!"); // print "Hello, world!"
badusb.print("Hello, world!", 100); // Add 100ms delay between key presses
```

## println
Same as `print` but ended with "ENTER" press.

### Parameters
- A string to print
- (optional) delay between key presses

### Examples:
```js
badusb.println("Hello, world!");  // print "Hello, world!" and press "ENTER"
```

# Key names list

## Modifier keys

| Name          |
| ------------- |
| CTRL          |
| SHIFT         |
| ALT           |
| GUI           |

## Special keys

| Name               | Notes            |
| ------------------ | ---------------- |
| DOWN               | Down arrow       |
| LEFT               | Left arrow       |
| RIGHT              | Right arrow      |
| UP                 | Up arrow         |
| ENTER              |                  |
| DELETE             |                  |
| BACKSPACE          |                  |
| END                |                  |
| HOME               |                  |
| ESC                |                  |
| INSERT             |                  |
| PAGEUP             |                  |
| PAGEDOWN           |                  |
| CAPSLOCK           |                  |
| NUMLOCK            |                  |
| SCROLLLOCK         |                  |
| PRINTSCREEN        |                  |
| PAUSE              | Pause/Break key  |
| SPACE              |                  |
| TAB                |                  |
| MENU               | Context menu key |
| Fx                 | F1-F24 keys      |
