# Submenu GUI view {#js_gui__submenu}

Displays a scrollable list of clickable textual entries.

<img src="submenu.png" width="200" alt="Sample screenshot of the view" />

```js
let eventLoop = require("event_loop");
let gui = require("gui");
let submenuView = require("gui/submenu");
```

This module depends on the `gui` module, which in turn depends on the
`event_loop` module, so they **must** be imported in this order. It is also
recommended to conceptualize these modules first before using this one.

## View props

| Property | Type      | Description                               |
|----------|-----------|-------------------------------------------|
| `header` | `string`  | A single line of text that appears above the list. |
| `items`  | `string[]`| The list of options.                     |


## View events

| Item     | Type    | Description                                                   |
|----------|---------|---------------------------------------------------------------|
| `chosen` | `number`| Fires when an entry has been chosen by the user. The item contains the index of the entry. |
