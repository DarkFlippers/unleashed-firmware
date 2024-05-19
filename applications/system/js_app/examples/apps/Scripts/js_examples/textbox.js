let textbox = require("textbox");

// You should set config before adding text
// Focus (start / end), Font (text / hex)
textbox.setConfig("end", "text");

// Can make sure it's cleared before showing, in case of reusing in same script
// (Closing textbox already clears the text, but maybe you added more in a loop for example)
textbox.clearText();

// Add default text
textbox.addText("Example dynamic updating textbox\n");

// Non-blocking, can keep updating text after, can close in JS or in GUI
textbox.show();

let i = 0;
while (textbox.isOpen() && i < 20) {
    print("console", i++);

    // Add text to textbox buffer
    textbox.addText("textbox " + to_string(i) + "\n");

    delay(500);
}

// If not closed by user (instead i < 20 is false above), close forcefully
if (textbox.isOpen()) {
    textbox.close();
}
