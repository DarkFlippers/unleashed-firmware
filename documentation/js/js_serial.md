# js_serial {#js_serial}

# Serial module
```js
let serial = require("serial");
```
# Methods

## setup
Configure serial port. Should be called before all other methods.

### Parameters
- Serial port name (usart, lpuart)
- Baudrate

### Examples:
```js
// Configure LPUART port with baudrate = 115200
serial.setup("lpuart", 115200);
```

## write
Write data to serial port

### Parameters
One or more arguments of the following types:
- A string
- Single number, each number is interpreted as a byte
- Array of numbers, each number is interpreted as a byte
- ArrayBuffer or DataView

### Examples:
```js
serial.write(0x0a); // Write a single byte 0x0A
serial.write("Hello, world!"); // Write a string
serial.write("Hello, world!", [0x0d, 0x0a]); // Write a string followed by two bytes
```

## read
Read a fixed number of characters from serial port.

### Parameters
- Number of bytes to read
- (optional) Timeout value in ms

### Returns
A sting of received characters or undefined if nothing was received before timeout.

### Examples:
```js
serial.read(1); // Read a single byte, without timeout
serial.read(10, 5000); // Read 10 bytes, with 5s timeout
```

## readln
Read from serial port until line break character

### Parameters
(optional) Timeout value in ms

### Returns
A sting of received characters or undefined if nothing was received before timeout.

### Examples:
```js
serial.readln(); // Read without timeout
serial.readln(5000); // Read with 5s timeout
```

## readBytes
Read from serial port until line break character

### Parameters
- Number of bytes to read
- (optional) Timeout value in ms

### Returns
ArrayBuffer with received data or undefined if nothing was received before timeout.

### Examples:
```js
serial.readBytes(4); // Read 4 bytes, without timeout

// Read one byte from receive buffer with zero timeout, returns UNDEFINED if Rx buffer is empty
serial.readBytes(1, 0);
```

## expect
Search for a string pattern in received data stream

### Parameters
- Single argument or array of the following types:
    - A string
    - Array of numbers, each number is interpreted as a byte
- (optional) Timeout value in ms

### Returns
Index of matched pattern in input patterns list, undefined if nothing was found.

### Examples:
```js
// Wait for root shell prompt with 1s timeout, returns 0 if it was received before timeout, undefined if not
serial.expect("# ", 1000); 

// Infinitely wait for one of two strings, should return 0 if the first string got matched, 1 if the second one
serial.expect([": not found", "Usage: "]);
```