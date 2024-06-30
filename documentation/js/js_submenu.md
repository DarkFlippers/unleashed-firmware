# js_submenu {#js_submenu}

# Submenu module
```js
let submenu = require("submenu");
```
# Methods

## setHeader
Set the submenu header text.

### Parameters
- header (string): The submenu header text

### Example
```js
submenu.setHeader("Select an option:");
```

## addItem
Add a new submenu item.

### Parameters
- label (string): The submenu item label text
- id (number): The submenu item ID, must be a Uint32 number

### Example
```js
submenu.addItem("Option 1", 1);
submenu.addItem("Option 2", 2);
submenu.addItem("Option 3", 3);
```

## show
Show a submenu that was previously configured using `setHeader()` and `addItem()` methods.

### Returns
The ID of the submenu item that was selected, or `undefined` if the BACK button was pressed.

### Example
```js
let selected = submenu.show();
if (selected === undefined) {
    // if BACK button was pressed
} else if (selected === 1) {
    // if item with ID 1 was selected
}
```
