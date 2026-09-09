// Script cannot work without subghz module so check before
checkSdkFeatures(["subghz"]);

let subghz = require("subghz");
subghz.setup();

function printRXline() {
    if (subghz.getState() !== "RX") {
        subghz.setRx(); // to RX
    }

    let rssi = subghz.getRssi();
    let freq = subghz.getFrequency();
    let ext = subghz.isExternal();

    print("rssi: ", rssi, "dBm", "@", freq, "MHz", "ext: ", ext);
}

function changeFrequency(freq) {
    if (subghz.getState() !== "IDLE") {
        subghz.setIdle(); // need to be idle to change frequency
    }
    subghz.setFrequency(freq);
}

subghz.setIdle();
print(subghz.getState()); // "IDLE"
subghz.setRx();
print(subghz.getState()); // "RX"

changeFrequency(433920000);
printRXline();
delay(1000);

print("Sending 0.sub")
subghz.transmitFile("/ext/subghz/0.sub");
// Can also specify repeat count: subghz.transmitFile(path, repeat)
// If not provided, defaults to 1 repeat for RAW and 10 repeats for parsed
// These 10 repeats by default are to simulate holding the button on remote
print("Send success");
delay(1000);

changeFrequency(315000000);
printRXline();

// Optional, done automatically at script end
subghz.end()
// But can be used to setup again, which will retry to detect external modules