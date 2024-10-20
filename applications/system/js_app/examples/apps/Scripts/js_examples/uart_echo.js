let serial = require("serial");
serial.setup("usart", 230400);

while (1) {
    let rx_data = serial.readBytes(1, 1000);
    if (rx_data !== undefined) {
        serial.write(rx_data);
        let data_view = Uint8Array(rx_data);
        print("0x" + data_view[0].toString(16));
    }
}

// There's also serial.end(), so you can serial.setup() again in same script
// You can also use serial.readAny(timeout), will avoid starving your loop with single byte reads
