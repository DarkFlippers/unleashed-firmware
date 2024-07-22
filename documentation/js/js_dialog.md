# js_dialog {#js_dialog}

# Dialog module
```js
let dialog = require("dialog");
```
# Methods

## message
Show a simple message dialog with header, text and "OK" button.

### Parameters
- Dialog header text
- Dialog text

### Returns
true if central button was pressed, false if the dialog was closed by back key press

### Examples:
```js
dialog.message("Dialog demo", "Press OK to start");
```

## custom
More complex dialog with configurable buttons

### Parameters
Configuration object with the following fields:
- header: Dialog header text
- text: Dialog text
- button_left: (optional) left button name
- button_right: (optional) right button name
- button_center: (optional) central button name

### Returns
Name of pressed button or empty string if the dialog was closed by back key press

### Examples:
```js
let dialog_params = ({
    header: "Dialog header",
    text: "Dialog text",
    button_left: "Left",
    button_right: "Right",
    button_center: "OK"
});

dialog.custom(dialog_params);
```
