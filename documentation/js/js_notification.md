# Notification module {#js_notification}

```js
let notify = require("notification");
```
## Methods

### success()
"Success" flipper notification message.

**Example**
```js
notify.success();
```

<br>

### error()
"Error" flipper notification message.

**Example**
```js
notify.error();
```

<br>

### blink()
Blink notification LED.

**Parameters**
- Blink color (blue/red/green/yellow/cyan/magenta)
- Blink type (short/long)

**Examples**
```js
notify.blink("red", "short"); // Short blink of red LED
notify.blink("green", "short"); // Long blink of green LED
```