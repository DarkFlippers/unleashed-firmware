# Serial module {#js_serial}

```js
let serial = require("serial");
```
# Methods

## setup()
Configure serial port. Should be called before all other methods.

**Parameters**

- Serial port name (`"usart"`, `"lpuart"`)
- Baudrate
- Optional framing configuration object (e.g. `{ dataBits: "8", parity: "even", stopBits: "1" }`):
  - `dataBits`: `"6"`, `"7"`, `"8"`, `"9"`
    - 6 data bits can only be selected when parity is enabled (even or odd)
    - 9 data bits can only be selected when parity is disabled (none)
  - `parity`: `"none"`, `"even"`, `"odd"`
  - `stopBits`: `"0.5"`, `"1"`, `"1.5"`, `"2"`
    - LPUART only supports whole stop bit lengths (i.e. 1 and 2 but not 0.5 and 1.5)

**Example**

```js
// Configure LPUART port with baudrate = 115200
serial.setup("lpuart", 115200);
```

<br>

## write()
Write data to serial port.

**Parameters**

One or more arguments of the following types:
- A string
- Single number, each number is interpreted as a byte
- Array of numbers, each number is interpreted as a byte
- ArrayBuffer or DataView

**Example**

```js
serial.write(0x0a); // Write a single byte 0x0A
serial.write("Hello, world!"); // Write a string
serial.write("Hello, world!", [0x0d, 0x0a]); // Write a string followed by two bytes
```

<br>

## read()
Read a fixed number of characters from serial port.

**Parameters**

- Number of bytes to read
- *(optional)* Timeout value in ms

**Returns**

A sting of received characters or undefined if nothing was received before timeout.

**Example**

```js
serial.read(1); // Read a single byte, without timeout
serial.read(10, 5000); // Read 10 bytes, with 5s timeout
```

<br>

## readln()
Read from serial port until line break character.

**Parameters**

- *(optional)* Timeout value in ms.

**Returns**

A sting of received characters or undefined if nothing was received before timeout.

**Example**

```js
serial.readln(); // Read without timeout
serial.readln(5000); // Read with 5s timeout
```

<br>

## readAny()
Read any available amount of data from serial port, can help avoid starving your loop with small reads.

**Parameters**

- *(optional)* Timeout value in ms

**Returns**

A sting of received characters or undefined if nothing was received before timeout.

**Example**

```js
serial.readAny(); // Read without timeout
serial.readAny(5000); // Read with 5s timeout
```

<br>

## readBytes()
Read from serial port until line break character.

**Parameters**

- Number of bytes to read
- *(optional)* Timeout value in ms

**Returns**

ArrayBuffer with received data or undefined if nothing was received before timeout.

**Example**

```js
serial.readBytes(4); // Read 4 bytes, without timeout

// Read one byte from receive buffer with zero timeout, returns UNDEFINED if Rx buffer is empty
serial.readBytes(1, 0);
```

<br>

## expect()
Search for a string pattern in received data stream.

**Parameters**

- Single argument or array of the following types:
    - A string
    - Array of numbers, each number is interpreted as a byte
- *(optional)* Timeout value in ms

**Returns**

Index of matched pattern in input patterns list, undefined if nothing was found.

**Example**

```js
// Wait for root shell prompt with 1s timeout, returns 0 if it was received before timeout, undefined if not
serial.expect("# ", 1000);

// Infinitely wait for one of two strings, should return 0 if the first string got matched, 1 if the second one
serial.expect([": not found", "Usage: "]);
```

<br>

## end()
Deinitializes serial port, allowing multiple initializations per script run.

**Example**

```js
serial.end();
// Configure LPUART port with baudrate = 115200
serial.setup("lpuart", 115200);
```