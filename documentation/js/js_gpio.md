# js_gpio {#js_gpio}

# GPIO module
```js
let eventLoop = require("event_loop");
let gpio = require("gpio");
```

This module depends on the `event_loop` module, so it _must_ only be imported
after `event_loop` is imported.

# Example
```js
let eventLoop = require("event_loop");
let gpio = require("gpio");

let led = gpio.get("pc3");
led.init({ direction: "out", outMode: "push_pull" });

led.write(true);
delay(1000);
led.write(false);
delay(1000);
```

# API reference
## `get`
Gets a `Pin` object that can be used to manage a pin.

### Parameters
  - `pin`: pin identifier (examples: `"pc3"`, `7`, `"pa6"`, `3`)

### Returns
A `Pin` object

## `Pin` object
### `Pin.init()`
Configures a pin

#### Parameters
  - `mode`: `Mode` object:
    - `direction` (required): either `"in"` or `"out"`
    - `outMode` (required for `direction: "out"`): either `"open_drain"` or
      `"push_pull"`
    - `inMode` (required for `direction: "in"`): either `"analog"`,
      `"plain_digital"`, `"interrupt"` or `"event"`
    - `edge` (required for `inMode: "interrupt"` or `"event"`): either
      `"rising"`, `"falling"` or `"both"`
    - `pull` (optional): either `"up"`, `"down"` or unset

### `Pin.write()`
Writes a digital value to a pin configured with `direction: "out"`

#### Parameters
  - `value`: boolean logic level to write

### `Pin.read()`
Reads a digital value from a pin configured with `direction: "in"` and any
`inMode` except `"analog"`

#### Returns
Boolean logic level

### `Pin.read_analog()`
Reads an analog voltage level in millivolts from a pin configured with
`direction: "in"` and `inMode: "analog"`

#### Returns
Voltage on pin in millivolts

### `Pin.interrupt()`
Attaches an interrupt to a pin configured with `direction: "in"` and
`inMode: "interrupt"` or `"event"`

#### Returns
An event loop `Contract` object that identifies the interrupt event source. The
event does not produce any extra data.
