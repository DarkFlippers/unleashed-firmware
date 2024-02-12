let uart = require("uart");
uart.setup(115200);

// uart.write("\n");
uart.write([0x0a]);
let console_resp = uart.expect("# ", 1000);
if (console_resp === undefined) {
    print("No CLI response");
} else {
    uart.write("uci\n");
    let uci_state = uart.expect([": not found", "Usage: "]);
    if (uci_state === 1) {
        uart.expect("# ");
        uart.write("uci show wireless\n");
        uart.expect(".key=");
        print("key:", uart.readln());
    } else {
        print("uci cmd not found");
    }
}
