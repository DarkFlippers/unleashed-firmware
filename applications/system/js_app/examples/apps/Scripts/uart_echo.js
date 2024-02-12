let uart = require("uart");
uart.setup(115200);

while (1) {
    let rx_data = uart.readBytes(1, 0);
    if (rx_data !== undefined) {
        uart.write(rx_data);
        let data_view = Uint8Array(rx_data);
        print("0x" + to_hex_string(data_view[0]));
    }
}