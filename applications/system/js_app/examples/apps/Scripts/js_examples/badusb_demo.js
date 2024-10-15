let badusb = require("badusb");
let notify = require("notification");
let flipper = require("flipper");
let gui = require("gui");
let dialog = require("gui/dialog");


// TODO: Add event loop from ofw
badusb.setup({
    vid: 0xAAAA,
    pid: 0xBBBB,
    mfrName: "Flipper",
    prodName: "Zero",
    layout_path: "/ext/badusb/assets/layouts/en-US.kl"
});
let views = {
    dialog: dialog.makeWith({
        header: "BadUSB demo",
        text: "Press OK to start",
        center: "Start",
    }),
};

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

    // Alt+Numpad method works only on Windows!!!
    badusb.altPrintln("This was printed with Alt+Numpad method!");

    // There's also badusb.print() and badusb.altPrint()
    // which don't add the return at the end

    notify.success();
} else {
    print("USB not connected");
    notify.error();
}

// Optional, but allows to interchange with usbdisk
badusb.quit();