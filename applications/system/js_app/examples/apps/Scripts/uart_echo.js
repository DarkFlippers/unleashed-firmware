let serial = require("serial");
serial.setup("usart", 230400);

while (1) {
    let rx_data = serial.readBytes(1, 0);
    if (rx_data !== undefined) {
        serial.write(rx_data);
        let data_view = Uint8Array(rx_data);
        print("0x" + to_hex_string(data_view[0]));
    }
}