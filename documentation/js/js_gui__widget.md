# Widget GUI view {#js_gui__widget}

Displays a combination of custom elements on one screen.

<img src="widget.png" width="200" alt="Sample screenshot of the view" />

```js
let eventLoop = require("event_loop");
let gui = require("gui");
let widgetView = require("gui/widget");
```

This module depends on the `gui` module, which in turn depends on the
`event_loop` module, so they **must** be imported in this order. It is also
recommended to conceptualize these modules first before using this one.

## Example
For an example, refer to the `gui.js` example script.

## View props
This view does not have any props.

## Children
This view has the elements as its children.
Elements are objects with properties to define them, in the form `{ element: "type", ...properties }` (e.g. `{ element: "button", button: "right", text: "Back" }`).

| **Element Type** | **Properties** | **Description**                                |
|------------------|----------------|------------------------------------------------|
| `string_multiline` | `x` (number), `y` (number) <br> `align` ((`"t"`, `"c"`, `"b"`) + (`"l"`, `"m"`, `"r"`)) <br> `font` (`"primary"`, `"secondary"`, `"keyboard"`, `"big_numbers"`) <br> `text` (string) | String of text that can span multiple lines.      |
| `string`           | `x` (number), `y` (number) <br> `align` ((`"t"`, `"c"`, `"b"`) + (`"l"`, `"m"`, `"r"`)) <br> `font` (`"primary"`, `"secondary"`, `"keyboard"`, `"big_numbers"`) <br> `text` (string) | String of text.                                   |
| `text_box`         | `x` (number), `y` (number) <br> `w` (number), `h` (number) <br> `align` ((`"t"`, `"c"`, `"b"`) + (`"l"`, `"m"`, `"r"`)) <br> `text` (string) <br> `stripToDots` (boolean)            | Box of with text that can be scrolled vertically. |
| `text_scroll`      | `x` (number), `y` (number) <br> `w` (number), `h` (number) <br> `text` (string)                                                                                                      | Text that can be scrolled vertically.             |
| `button`           | `text` (string) <br> `button` (`"left"`, `"center"`, `"right"`)                                                                                                                      | Button at the bottom of the screen.               |
| `icon`             | `x` (number), `y` (number) <br> `iconData` ([IconData](#js_gui__icon))                                                                                                               | Display an icon.                                  |
| `rect`             | `x` (number), `y` (number) <br> `w` (number), `h` (number) <br> `radius` (number), `fill` (boolean)                                                                                  | Draw a rectangle, optionally rounded and filled.  |
| `circle`           | `x` (number), `y` (number) <br> `radius` (number), `fill` (boolean)                                                                                                                  | Draw a circle, optionally filled.                 |
| `line`             | `x1` (number), `y1` (number) <br> `x2` (number), `y2` (number)                                                                                                                       | Draw a line between 2 points.                     |
