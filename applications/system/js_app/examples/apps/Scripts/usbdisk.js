let usbdisk = require("usbdisk");
print("Starting UsbDisk...");
usbdisk.start("/ext/apps_data/mass_storage/128MB.img");
print("Started, waiting until ejected...");
while (!usbdisk.wasEjected()) {
    delay(1000);
}
print("Ejected, stopping UsbDisk...");
usbdisk.stop();
print("Done");