# File Picker GUI prompt {#js_gui__file_picker}

Allows asking the user to select a file.
It is not GUI view like other JS GUI views, rather just a function that shows a prompt.

# Example
For an example, refer to the `gui.js` example script.

# API reference

## pickFile()
Displays a file picker and returns the selected file, or undefined if cancelled.

**Parameters**
  - `basePath`: the path to start at
  - `extension`: the file extension(s) to show (like `.sub`, `.iso|.img`, `*`)

**Returns**

A `string` path, or `undefined`.
