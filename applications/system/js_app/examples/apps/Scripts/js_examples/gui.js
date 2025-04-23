// import modules
let eventLoop = require("event_loop");
let gui = require("gui");
let loadingView = require("gui/loading");
let submenuView = require("gui/submenu");
let emptyView = require("gui/empty_screen");
let textInputView = require("gui/text_input");
let byteInputView = require("gui/byte_input");
let textBoxView = require("gui/text_box");
let dialogView = require("gui/dialog");
let filePicker = require("gui/file_picker");
let widget = require("gui/widget");
let icon = require("gui/icon");
let flipper = require("flipper");

// declare clock widget children
let cuteDolphinWithWatch = icon.getBuiltin("DolphinWait_59x54");
let jsLogo = icon.getBuiltin("js_script_10px");
let stopwatchWidgetElements = [
    { element: "string", x: 67, y: 44, align: "bl", font: "big_numbers", text: "00 00" },
    { element: "string", x: 77, y: 22, align: "bl", font: "primary", text: "Stopwatch" },
    { element: "rect", x: 64, y: 27, w: 28, h: 20, radius: 3, fill: false },
    { element: "rect", x: 100, y: 27, w: 28, h: 20, radius: 3, fill: false },
    { element: "icon", x: 0, y: 5, iconData: cuteDolphinWithWatch },
    { element: "icon", x: 64, y: 13, iconData: jsLogo },
    { element: "button", button: "right", text: "Back" },
];

// declare view instances
let views = {
    loading: loadingView.make(),
    empty: emptyView.make(),
    keyboard: textInputView.makeWith({
        header: "Enter your name",
        minLength: 0,
        maxLength: 32,
        defaultText: flipper.getName(),
        defaultTextClear: true,
    }),
    helloDialog: dialogView.make(),
    bytekb: byteInputView.makeWith({
        header: "Look ma, I'm a header text!",
        length: 8,
        defaultData: Uint8Array([0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88]),
    }),
    longText: textBoxView.makeWith({
        text: "This is a very long string that demonstrates the TextBox view. Use the D-Pad to scroll backwards and forwards.\nLorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse rhoncus est malesuada quam egestas ultrices. Maecenas non eros a nulla eleifend vulputate et ut risus. Quisque in mauris mattis, venenatis risus eget, aliquam diam. Fusce pretium feugiat mauris, ut faucibus ex volutpat in. Phasellus volutpat ex sed gravida consectetur. Aliquam sed lectus feugiat, tristique lectus et, bibendum lacus. Ut sit amet augue eu sapien elementum aliquam quis vitae tortor. Vestibulum quis commodo odio. In elementum fermentum massa, eu pellentesque nibh cursus at. Integer eleifend lacus nec purus elementum sodales. Nulla elementum neque urna, non vulputate massa semper sed. Fusce ut nisi vitae dui blandit congue pretium vitae turpis.",
    }),
    stopwatchWidget: widget.makeWith({}, stopwatchWidgetElements),
    demos: submenuView.makeWith({
        header: "Choose a demo",
        items: [
            "Hourglass screen",
            "Empty screen",
            "Text input & Dialog",
            "Byte input",
            "Text box",
            "File picker",
            "Widget",
            "Exit app",
        ],
    }),
};

// demo selector
eventLoop.subscribe(views.demos.chosen, function (_sub, index, gui, eventLoop, views) {
    if (index === 0) {
        gui.viewDispatcher.switchTo(views.loading);
        // the loading view captures all back events, preventing our navigation callback from firing
        // switch to the demo chooser after a second
        eventLoop.subscribe(eventLoop.timer("oneshot", 1000), function (_sub, _, gui, views) {
            gui.viewDispatcher.switchTo(views.demos);
        }, gui, views);
    } else if (index === 1) {
        gui.viewDispatcher.switchTo(views.empty);
    } else if (index === 2) {
        gui.viewDispatcher.switchTo(views.keyboard);
    } else if (index === 3) {
        gui.viewDispatcher.switchTo(views.bytekb);
    } else if (index === 4) {
        gui.viewDispatcher.switchTo(views.longText);
    } else if (index === 5) {
        let path = filePicker.pickFile("/ext", "*");
        if (path) {
            views.helloDialog.set("text", "You selected:\n" + path);
        } else {
            views.helloDialog.set("text", "You didn't select a file");
        }
        views.helloDialog.set("center", "Nice!");
        gui.viewDispatcher.switchTo(views.helloDialog);
    } else if (index === 6) {
        gui.viewDispatcher.switchTo(views.stopwatchWidget);
    } else if (index === 7) {
        eventLoop.stop();
    }
}, gui, eventLoop, views);

// say hi after keyboard input
eventLoop.subscribe(views.keyboard.input, function (_sub, name, gui, views) {
    views.keyboard.set("defaultText", name); // Remember for next usage
    views.helloDialog.set("text", "Hi " + name + "! :)");
    views.helloDialog.set("center", "Hi Flipper! :)");
    gui.viewDispatcher.switchTo(views.helloDialog);
}, gui, views);

// go back after the greeting dialog
eventLoop.subscribe(views.helloDialog.input, function (_sub, button, gui, views) {
    if (button === "center")
        gui.viewDispatcher.switchTo(views.demos);
}, gui, views);

// show data after byte input
eventLoop.subscribe(views.bytekb.input, function (_sub, data, gui, views) {
    let data_view = Uint8Array(data);
    let text = "0x";
    for (let i = 0; i < data_view.length; i++) {
        text += data_view[i].toString(16);
    }
    views.helloDialog.set("text", "You typed:\n" + text);
    views.helloDialog.set("center", "Cool!");
    gui.viewDispatcher.switchTo(views.helloDialog);
}, gui, views);

// go to the demo chooser screen when the back key is pressed
eventLoop.subscribe(gui.viewDispatcher.navigation, function (_sub, _, gui, views, eventLoop) {
    if (gui.viewDispatcher.currentView === views.demos) {
        eventLoop.stop();
        return;
    }
    gui.viewDispatcher.switchTo(views.demos);
}, gui, views, eventLoop);

// go to the demo chooser screen when the right key is pressed on the widget screen
eventLoop.subscribe(views.stopwatchWidget.button, function (_sub, buttonId, gui, views) {
    if (buttonId === "right")
        gui.viewDispatcher.switchTo(views.demos);
}, gui, views);

// count time
eventLoop.subscribe(eventLoop.timer("periodic", 500), function (_sub, _item, views, stopwatchWidgetElements, halfSeconds) {
    let text = (halfSeconds / 2 / 60).toString();
    if (halfSeconds < 10 * 60 * 2)
        text = "0" + text;

    text += (halfSeconds % 2 === 0) ? ":" : " ";

    if (((halfSeconds / 2) % 60) < 10)
        text += "0";
    text += ((halfSeconds / 2) % 60).toString();

    stopwatchWidgetElements[0].text = text;
    views.stopwatchWidget.setChildren(stopwatchWidgetElements);

    halfSeconds++;
    return [views, stopwatchWidgetElements, halfSeconds];
}, views, stopwatchWidgetElements, 0);

// run UI
gui.viewDispatcher.switchTo(views.demos);
eventLoop.run();
