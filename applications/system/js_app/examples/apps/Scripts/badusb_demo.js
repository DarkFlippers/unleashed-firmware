let badusb = require("badusb");
let notify = require("notification");
let flipper = require("flipper");
let dialog = require("dialog");

badusb.setup({
    vid: 0xAAAA,
    pid: 0xBBBB,
    mfr_name: "Flipper",
    prod_name: "Zero",
    layout_path: "/ext/badusb/assets/layouts/en-US.kl"
});
dialog.message("BadUSB demo", "Press OK to start");

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
    badusb.println("Battery level: " + to_string(flipper.getBatteryCharge()) + "%");

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