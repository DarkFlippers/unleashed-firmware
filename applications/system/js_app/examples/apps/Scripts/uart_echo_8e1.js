// This script is like uart_echo, except it uses 8E1 framing (8 data bits, even
// parity, 1 stop bit) as opposed to the default 8N1 (8 data bits, no parity,
// 1 stop bit)

let serial = require("serial");
serial.setup("usart", 230400, { dataBits: "8", parity: "even", stopBits: "1" });

while (1) {
    let rx_data = serial.readBytes(1, 1000);
    if (rx_data !== undefined) {
        serial.write(rx_data);
        let data_view = Uint8Array(rx_data);
        print("0x" + data_view[0].toString(16));
    }
}
