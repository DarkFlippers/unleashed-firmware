# js_textbox {#js_textbox}

# Textbox module
```js
let textbox = require("textbox");
```
# Methods

## setConfig
Set focus and font for the textbox.

### Parameters
- focus: "start" to focus on the beginning of the text, or "end" to focus on the end of the text
- font: "text" to use the default proportional font, or "hex" to use a monospaced font, which is convenient for aligned array output in HEX

### Example
```js
textbox.setConfig("start", "text");
textbox.addText("Hello world");
textbox.show();
```

## addText
Add text to the end of the textbox.

### Parameters
- text (string): The text to add to the end of the textbox

### Example
```js
textbox.addText("New text 1\nNew text 2");
```

## clearText
Clear the textbox.

### Example
```js
textbox.clearText();
```

## isOpen
Return true if the textbox is open.

### Returns
True if the textbox is open, false otherwise.

### Example
```js
let isOpen = textbox.isOpen();
```

## show
Show the textbox. You can add text to it using the `addText()` method before or after calling the `show()` method.

### Example
```js
textbox.show();
```

## close
Close the textbox.

### Example
```js
if (textbox.isOpen()) {
    textbox.close();
}
```
