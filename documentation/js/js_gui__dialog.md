# Dialog GUI view {#js_gui__dialog}

Displays a dialog with up to three options.

<img src="dialog.png" width="200" alt="Sample screenshot of the view" />

```js
let eventLoop = require("event_loop");
let gui = require("gui");
let dialogView = require("gui/dialog");
```

This module depends on the `gui` module, which in turn depends on the
`event_loop` module, so they **must** be imported in this order. It is also
recommended to conceptualize these modules first before using this one.

# Example
For an example, refer to the `gui.js` example script.

# View props

| **Prop**   | **Type**  | **Description**                                                |
|------------|-----------|----------------------------------------------------------------|
| `header`   | string    | Text that appears in bold at the top of the screen.            |
| `text`     | string    | Text that appears in the middle of the screen.                 |
| `left`     | string    | Text for the left button. If unset, the left button does not show up. |
| `center`   | string    | Text for the center button. If unset, the center button does not show up. |
| `right`    | string    | Text for the right button. If unset, the right button does not show up. |

# View events

| Item     | Type   | Description                                                                 |
|----------|--------|-----------------------------------------------------------------------------|
| `input`  | `string`| Fires when the user presses on either of the three possible buttons. The item contains one of the strings `"left"`, `"center"`, or `"right"` depending on the button. |
