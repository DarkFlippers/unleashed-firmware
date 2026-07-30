// Script cannot work without i2c module so check before
checkSdkFeatures(["i2c"]);

// Connect an 24C32N EEPROM to the I2C bus of the board. SDA=pin 15, SCL=pin 16, VCC=pin 9, GND=pin 8.
let i2c = require("i2c");

function i2c_find_first_device() {
    let addr = -1;
    for (let try_addr = 0; try_addr !== 0xff; try_addr++) {
        if (i2c.isDeviceReady(try_addr, 5)) {
            addr = try_addr;
            break;
        }
    }
    return addr;
}

let addr = i2c_find_first_device();
if (addr === -1) {
    print("I2C device not found");
    print("Please connect a 24C32N  EEPROM I2C device to the Flipper Zero.");
    print("SDA=pin 15, SCL=pin 16,  VCC=pin 9, GND=pin 8.");
} else {
    print("I2C device found at address: " + addr.toString(16));
    delay(1000);

    // first two bytes are the start address (0x0000)
    // the remaining bytes are the data to store.
    // can also use Uint8Array([0x00, 0x00, ...]) as write parameter
    i2c.write(addr, [0x00, 0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47]);
    while (i2c.isDeviceReady(addr, 9) === false) {
        print("Waiting for device to be ready...");
    }

    // write the address to read from (we start at address 0x0001)
    // read 3 bytes - 0x42, 0x43, 0x44
    let data_buf = i2c.writeRead(addr, [0x00, 0x01], 3, 100);
    let data = Uint8Array(data_buf);
    print("Read bytes: " + data.length.toString());
    for (let i = 0; i < data.length; i++) {
        print("data[" + i.toString() + "] = " + data[i].toString(16));
    }

    // read two more bytes (0x45, 0x46) from current address
    data_buf = i2c.read(addr, 2);
    data = Uint8Array(data_buf);
    print("Read bytes: " + data.length.toString());
    for (let i = 0; i < data.length; i++) {
        print("data[" + i.toString() + "] = " + data[i].toString(16));
    }
}
