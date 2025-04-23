# GPIO module {#js_gpio}

The module allows you to control GPIO pins of the expansion connector on Flipper Zero. Call the `require` function to load the module before first using its methods. This module depends on the `event_loop` module, so it **must** be imported after `event_loop` is imported:

```js
let eventLoop = require("event_loop");
let gpio = require("gpio");
```

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

---

# API reference
## get()
Gets a `Pin` object that can be used to manage a pin.

**Parameters**
  - `pin`: pin identifier (examples: `"pc3"`, `7`, `"pa6"`, `3`)

**Returns**

A `Pin` object.

<br>

## Pin object
### Pin.init()
Configures a pin.

**Parameters**
  - `mode`: `Mode` object:
    - `direction` (required): either `"in"` or `"out"`
    - `outMode` (required for `direction: "out"`): either `"open_drain"` or
      `"push_pull"`
    - `inMode` (required for `direction: "in"`): either `"analog"`,
      `"plain_digital"`, `"interrupt"` or `"event"`
    - `edge` (required for `inMode: "interrupt"` or `"event"`): either
      `"rising"`, `"falling"` or `"both"`
    - `pull` (optional): either `"up"`, `"down"` or unset

<br>

### Pin.write()
Writes a digital value to a pin configured with `direction: "out"`.

**Parameters**
  - `value`: boolean logic level to write

<br>

### Pin.read()
Reads a digital value from a pin configured with `direction: "in"` and any
`inMode` except `"analog"`.

**Returns**

Boolean logic level.

<br>

### Pin.readAnalog()
Reads an analog voltage level in millivolts from a pin configured with
`direction: "in"` and `inMode: "analog"`.

**Returns**

Voltage on pin in millivolts.

<br>

### Pin.interrupt()
Attaches an interrupt to a pin configured with `direction: "in"` and
`inMode: "interrupt"` or `"event"`.

**Returns**

An event loop `Contract` object that identifies the interrupt event source. The
event does not produce any extra data.

### Pin.isPwmSupported()
Determines whether this pin supports PWM.
If `false`, all other PWM-related methods on this pin will throw an error when called.

**Returns**

Boolean value.

### Pin.pwmWrite()
Sets PWM parameters and starts the PWM.
Configures the pin with `{ direction: "out", outMode: "push_pull" }`.
Throws an error if PWM is not supported on this pin.

**Parameters**
  - `freq`: Frequency in Hz
  - `duty`: Duty cycle in %

### Pin.isPwmRunning()
Determines whether PWM is running.
Throws an error if PWM is not supported on this pin.

**Returns**

Boolean value.

### Pin.pwmStop()
Stops PWM.
Does not restore previous pin configuration.
Throws an error if PWM is not supported on this pin.
