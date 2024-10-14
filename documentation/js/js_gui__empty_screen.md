# js_gui__empty_screen {#js_gui__empty_screen}

# Empty Screen GUI View
Displays nothing.

<img src="empty.png" width="200" alt="Sample screenshot of the view" />

```js
let eventLoop = require("event_loop");
let gui = require("gui");
let emptyView = require("gui/empty_screen");
```

This module depends on the `gui` module, which in turn depends on the
`event_loop` module, so they _must_ be imported in this order. It is also
recommended to conceptualize these modules first before using this one.

# Example
For an example refer to the GUI example.

# View props
This view does not have any props.
