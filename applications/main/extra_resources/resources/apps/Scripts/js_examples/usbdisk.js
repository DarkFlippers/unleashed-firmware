// Script cannot work without usbdisk module so check before
checkSdkFeatures(["usbdisk"]);

let usbdisk = require("usbdisk");
let storage = require("storage");

let imagePath = "/ext/apps_data/mass_storage/128MB.img";
let imageSize = 128 * 1024 * 1024;

let imageExisted = storage.fileExists(imagePath);
if (imageExisted) {
    print("Disk image '128MB' already exists");
} else {
    // CreateImage isn't necessary to overall function, check when its used not at script start
    if (doesSdkSupport(["usbdisk-createimage"])) {
        print("Creating disk image '128MB'...");
        usbdisk.createImage(imagePath, imageSize);
    } else {
        die("Disk image '128MB' not present, can't auto-create");
    }
}

print("Starting UsbDisk...");
usbdisk.start("/ext/apps_data/mass_storage/128MB.img");

print("Started, waiting until ejected...");
while (!usbdisk.wasEjected()) {
    delay(1000);
}

print("Ejected, stopping UsbDisk...");
usbdisk.stop();

if (!imageExisted) {
    print("Removing disk image...");
    storage.remove(imagePath);
}

print("Done");