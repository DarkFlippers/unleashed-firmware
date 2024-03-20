let keyboard = require("keyboard");

keyboard.setHeader("Example Text Input");

// Default text is optional
let text = keyboard.text(100, "Default text", true);
// Returns undefined when pressing back
print("Got text:", text);

keyboard.setHeader("Example Byte Input");

// Default data is optional
let result = keyboard.byte(6, Uint8Array([1, 2, 3, 4, 5, 6]));
// Returns undefined when pressing back
if (result !== undefined) {
    let data = Uint8Array(result);
    result = "0x";
    for (let i = 0; i < data.byteLength; i++) {
        if (data[i] < 0x10) result += "0";
        result += to_hex_string(data[i]);
    }
}
print("Got data:", result);