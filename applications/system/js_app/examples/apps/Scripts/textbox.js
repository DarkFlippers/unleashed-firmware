let textbox = require("textbox");

// Set config before setting text
// Focus (start / end), Font (text / hex)
textbox.setConfig("end", "text");

let text = "Example dynamic updating textbox\n";
textbox.setText(text);

// Non-blocking, can keep updating text after, can close in JS or in GUI
textbox.show();

let i = 0;
while (textbox.isOpen() && i < 20) {
    print("console", i++);
    text += "textbox " + to_string(i) + "\n";
    textbox.setText(text);
    delay(500);
}

// If not closed by user (instead i < 20 is false above), close forcefully
if (textbox.isOpen()) {
    textbox.close();
}
