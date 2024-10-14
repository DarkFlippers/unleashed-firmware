# js_gui__text_input {#js_gui__text_input}

# Text input GUI view
Displays a keyboard.

<img src="text_input.png" width="200" alt="Sample screenshot of the view" />

```js
let eventLoop = require("event_loop");
let gui = require("gui");
let textInputView = require("gui/text_input");
```

This module depends on the `gui` module, which in turn depends on the
`event_loop` module, so they _must_ be imported in this order. It is also
recommended to conceptualize these modules first before using this one.

# Example
For an example refer to the `gui.js` example script.

# View props
## `minLength`
Smallest allowed text length

Type: `number`

## `maxLength`
Biggest allowed text length

Type: `number`

Default: `32`

## `header`
Single line of text that appears above the keyboard

Type: `string`

# View events
## `input`
Fires when the user selects the "save" button and the text matches the length
constrained by `minLength` and `maxLength`.

Item type: `string`
