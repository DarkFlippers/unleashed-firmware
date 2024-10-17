# Built-in methods {#js_builtin}

## require
Load a module plugin.

### Parameters
- Module name

### Examples:
```js
let serial = require("serial"); // Load "serial" module
```

## delay
### Parameters
- Delay value in ms

### Examples:
```js
delay(500); // Delay for 500ms
```
## print
Print a message on a screen console.

### Parameters
The following argument types are supported:
- String
- Number
- Bool
- undefined

### Examples:
```js
print("string1", "string2", 123);
```

## console.log
## console.warn
## console.error
## console.debug
Same as `print`, but output to serial console only, with corresponding log level.

## to_string
Convert a number to string with an optional base.

### Examples:
```js
to_string(123) // "123"
to_string(123, 16) // "0x7b"
```
