# Byte input GUI view {#js_gui__byte_input}

Displays a hexadecimal keyboard.

<img src="byte_input.png" width="200" alt="Sample screenshot of the view" />

```js
let eventLoop = require("event_loop");
let gui = require("gui");
let byteInputView = require("gui/byte_input");
```

This module depends on the `gui` module, which in turn depends on the
`event_loop` module, so they **must** be imported in this order. It is also
recommended to conceptualize these modules first before using this one.

## Example
For an example refer to the `gui.js` example script.

## View props

| Prop        | Type   | Description                                      |
|-------------|--------|--------------------------------------------------|
| `length`      | `number` | The length in bytes of the buffer to modify.           |
| `header`      | `string` | A single line of text that appears above the keyboard. |
| `defaultData` | `string` | Data to show by default.                               |

## View events

| Item        | Type   | Description                                      |
|-------------|--------|--------------------------------------------------|
| `input`     | `ArrayBuffer` | Fires when the user selects the "Save" button. |
