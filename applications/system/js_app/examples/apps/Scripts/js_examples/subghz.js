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

let result = subghz.transmitFile("/ext/subghz/0.sub");
print(result ? "Send success" : "Send failed");
delay(1000);

changeFrequency(315000000);
printRXline();