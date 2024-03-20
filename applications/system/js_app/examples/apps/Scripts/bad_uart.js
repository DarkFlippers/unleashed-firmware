let serial = require("serial");
serial.setup("lpuart", 115200);

// serial.write("\n");
serial.write([0x0a]);
let console_resp = serial.expect("# ", 1000);
if (console_resp === undefined) {
    print("No CLI response");
} else {
    serial.write("uci\n");
    let uci_state = serial.expect([": not found", "Usage: "]);
    if (uci_state === 1) {
        serial.expect("# ");
        serial.write("uci show wireless\n");
        serial.expect(".key=");
        print("key:", serial.readln());
    } else {
        print("uci cmd not found");
    }
}
