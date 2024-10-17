let serial = require("serial");
serial.setup("usart", 230400);

while (1) {
    let rx_data = serial.readBytes(1, 1000);
    if (rx_data !== undefined) {
        serial.write(rx_data);
        let data_view = Uint8Array(rx_data);
        print("0x" + toString(data_view[0], 16));
    }
}
