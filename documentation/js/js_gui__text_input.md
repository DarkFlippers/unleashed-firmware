# Text input GUI view {#js_gui__text_input}

Displays a keyboard.

<img src="text_input.png" width="200" alt="Sample screenshot of the view" />

```js
let eventLoop = require("event_loop");
let gui = require("gui");
let textInputView = require("gui/text_input");
```

This module depends on the `gui` module, which in turn depends on the
`event_loop` module, so they **must** be imported in this order. It is also
recommended to conceptualize these modules first before using this one.

## Example
For an example, refer to the `gui.js` example script.

## View props

| Prop        | Type   | Description                                      |
|-------------|--------|--------------------------------------------------|
| `minLength`        | `number`  | The shortest allowed text length.                          |
| `maxLength`        | `number`  | The longest allowed text length. <br> Default: `32`        |
| `header`           | `string`  | A single line of text that appears above the keyboard.     |
| `defaultText`      | `string`  | Text to show by default.                                   |
| `defaultTextClear` | `boolean` | Whether to clear the default text on next character typed. |

## View events

| Item        | Type   | Description                                      |
|-------------|--------|--------------------------------------------------|
| `input`     | `string` | Fires when the user selects the "Save" button and the text matches the length constrained by `minLength` and `maxLength`. |
