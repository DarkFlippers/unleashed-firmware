# js_gui__submenu {#js_gui__submenu}

# Submenu GUI view
Displays a scrollable list of clickable textual entries.

<img src="submenu.png" width="200" alt="Sample screenshot of the view" />

```js
let eventLoop = require("event_loop");
let gui = require("gui");
let submenuView = require("gui/submenu");
```

This module depends on the `gui` module, which in turn depends on the
`event_loop` module, so they _must_ be imported in this order. It is also
recommended to conceptualize these modules first before using this one.

# Example
For an example refer to the GUI example.

# View props
## `header`
Single line of text that appears above the list

Type: `string`

## `items`
The list of options

Type: `string[]`

# View events
## `chosen`
Fires when an entry has been chosen by the user. The item contains the index of
the entry.

Item type: `number`
