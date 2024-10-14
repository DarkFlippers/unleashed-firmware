let badusb = require("badusb");
let notify = require("notification");
let flipper = require("flipper");
let eventLoop = require("event_loop");
let gui = require("gui");
let dialog = require("gui/dialog");

let views = {
    dialog: dialog.makeWith({
        header: "BadUSB demo",
        text: "Press OK to start",
        center: "Start",
    }),
};

badusb.setup({ vid: 0xAAAA, pid: 0xBBBB, mfrName: "Flipper", prodName: "Zero" });

eventLoop.subscribe(views.dialog.input, function (_sub, button, eventLoop, gui) {
    if (button !== "center")
        return;

    gui.viewDispatcher.sendTo("back");

    if (badusb.isConnected()) {
        notify.blink("green", "short");
        print("USB is connected");

        badusb.println("Hello, world!");

        badusb.press("CTRL", "a");
        badusb.press("CTRL", "c");
        badusb.press("DOWN");
        delay(1000);
        badusb.press("CTRL", "v");
        delay(1000);
        badusb.press("CTRL", "v");

        badusb.println("1234", 200);

        badusb.println("Flipper Model: " + flipper.getModel());
        badusb.println("Flipper Name: " + flipper.getName());
        badusb.println("Battery level: " + toString(flipper.getBatteryCharge()) + "%");

        notify.success();
    } else {
        print("USB not connected");
        notify.error();
    }

    eventLoop.stop();
}, eventLoop, gui);

eventLoop.subscribe(gui.viewDispatcher.navigation, function (_sub, _item, eventLoop) {
    eventLoop.stop();
}, eventLoop);

gui.viewDispatcher.switchTo(views.dialog);
eventLoop.run();
