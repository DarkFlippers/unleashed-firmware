# js_notification {#js_notification}

# Notification module
```js
let notify = require("notification");
```
# Methods

## success
"Success" flipper notification message

### Examples:
```js
notify.success();
```

## error
"Error" flipper notification message

### Examples:
```js
notify.error();
```

## blink
Blink notification LED

### Parameters
- Blink color (blue/red/green/yellow/cyan/magenta)
- Blink type (short/long)

### Examples:
```js
notify.blink("red", "short"); // Short blink of red LED
notify.blink("green", "short"); // Long blink of green LED
```