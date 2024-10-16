let eventLoop = require("event_loop");
let gui = require("gui");
let dialog = require("gui/dialog");
let textInput = require("gui/text_input");
let loading = require("gui/loading");
let storage = require("storage");

// No eval() or exec() so need to run code from file, and filename must be unique
storage.makeDirectory("/ext/.tmp");
storage.makeDirectory("/ext/.tmp/js");
storage.rmrf("/ext/.tmp/js/repl")
storage.makeDirectory("/ext/.tmp/js/repl")
let ctx = {
    tmp_template: "/ext/.tmp/js/repl/",
    tmp_number: 0,
};

let views = {
    dialog: dialog.makeWith({
        header: "Interactive Console",
        text: "Press OK to Start",
        center: "Run Some JS"
    }),
    textInput: textInput.makeWith({
        header: "Type JavaScript Code:",
        defaultText: "2+2",
        defaultTextClear: true,
        // Props for makeWith() are passed in reverse order, so maxLength must be after defaultText
        minLength: 0,
        maxLength: 256,
    }),
    loading: loading.make(),
};

eventLoop.subscribe(views.dialog.input, function (_sub, button, gui, views) {
    if (button === "center") {
        gui.viewDispatcher.switchTo(views.textInput);
    }
}, gui, views);

eventLoop.subscribe(views.textInput.input, function (_sub, text, gui, views, ctx) {
    gui.viewDispatcher.switchTo(views.loading);

    let path = ctx.tmp_template + toString(ctx.tmp_number++);
    let file = storage.openFile(path, "w", "create_always");
    file.write("({run:function(){return " + text + ";},})");
    file.close();

    // Hide GUI before running, we want to see console and avoid deadlock if code fails
    gui.viewDispatcher.sendTo("back");
    let result = load(path).run();
    storage.remove(path);
    gui.viewDispatcher.sendTo("front");

    // Must convert to string explicitly
    if (typeof result === "number") {
        result = toString(result);
    } else if (typeof result === "undefined") {
        result = "undefined";
    } else if (typeof result === "boolean") {
        result = result ? "true" : "false";
    } else if (typeof result === "object") {
        result = JSON.stringify(result);
    }

    views.dialog.set("header", "JS Returned:");
    views.dialog.set("text", result);
    gui.viewDispatcher.switchTo(views.dialog);
    views.textInput.set("defaultText", text);
}, gui, views, ctx);

eventLoop.subscribe(gui.viewDispatcher.navigation, function (_sub, _, eventLoop) {
    eventLoop.stop();
}, eventLoop);

gui.viewDispatcher.switchTo(views.dialog);
eventLoop.run();
