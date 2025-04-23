# Built-in methods {#js_builtin}

## require()
Load a module plugin.

**Parameters**
- Module name

**Examples**
```js
let serial = require("serial"); // Load "serial" module
```

<br>

## delay()
**Parameters**
- Delay value in ms

**Examples**
```js
delay(500); // Delay for 500ms
```
<br>

## print()
Print a message on a screen console.

**Parameters**
The following argument types are supported:
- String
- Number
- Bool
- undefined

**Examples**
```js
print("string1", "string2", 123);
```
<br>

## Console object
Same as `print`, but output to serial console only, with corresponding log level.

### console.log()

<br>

### console.warn()

<br>

### console.error()

<br>

### console.debug()

<br>

## load()
Runs a JS file and returns value from it.

**Parameters**
- The path to the file
- An optional object to use as the global scope while running this file

**Examples**
```js
load("/ext/apps/Scripts/script.js");
```
<br>

## chr()
Convert an ASCII character number to string.

**Examples**
```js
chr(65); // "A"
```
<br>

## die()
Exit JavaScript with given message.

**Examples**
```js
die("Some error occurred");
```
<br>

## parseInt()
Convert a string to number with an optional base.

**Examples**
```js
parseInt("123"); // 123
parseInt("7b", 16); // 123
```
<br>

## Number object

### Number.toString()
Convert a number to string with an optional base.

**Examples**
```js
let num = 123;
num.toString(); // "123"
num.toString(16); // "0x7b"
```
<br>

## ArrayBuffer object

**Fields**

- byteLength: The length of the buffer in bytes
<br>

### ArrayBuffer.slice()
Creates an `ArrayBuffer` that contains a sub-part of the buffer.

**Parameters**
- The index to start the new buffer at
- An optional non-inclusive index of where to stop the new buffer

**Examples**
```js
Uint8Array([1, 2, 3]).buffer.slice(0, 1) // ArrayBuffer([1])
```
<br>

## DataView objects
Wrappers around `ArrayBuffer` objects, with dedicated types such as:
- `Uint8Array`
- `Int8Array`
- `Uint16Array`
- `Int16Array`
- `Uint32Array`
- `Int32Array`

**Fields**

- byteLength: The length of the buffer in bytes
- length: The length of the buffer in typed elements
- buffer: The underlying `ArrayBuffer`
<br>

## Array object

**Fields**

- length: How many elements there are in the array
<br>

### Array.splice()
Removes elements from the array and returns them in a new array.

**Parameters**
- The index to start taking elements from
- An optional count of how many elements to take

**Examples**
```js
let arr = [1, 2, 3];
arr.splice(1); // [2, 3]
arr; // [1]
```
<br>

### Array.push()
Adds a value to the end of the array.

**Examples**
```js
let arr = [1, 2];
arr.push(3);
arr; // [1, 2, 3]
```
<br>

## String object

**Fields**

- length: How many characters there are in the string
<br>

### String.charCodeAt()
Returns the character code at an index in the string.

**Examples**
```js
"A".charCodeAt(0) // 65
```
<br>

### String.at()
Same as `String.charCodeAt()`.
<br>

### String.indexOf()
Return index of first occurrence of substr within the string or `-1` if not found.

**Parameters**
- Substring to search for
- Optional index to start searching from

**Examples**
```js
"Example".indexOf("amp") // 2
```
<br>

### String.slice()
Return a substring between two indices.

**Parameters**
- The index to start the new string at
- An optional non-inclusive index of where to stop the new string

**Examples**
```js
"Example".slice(2) // "ample"
```
<br>

### String.toUpperCase()
Transforms the string to upper case.

**Examples**
```js
"Example".toUpperCase() // "EXAMPLE"
```
<br>

### String.toLowerCase()
Transforms the string to lower case.

**Examples**
```js
"Example".toLowerCase() // "example"
```
<br>

## __dirname
Path to the directory containing the current script.

**Examples**
```js
print(__dirname); // /ext/apps/Scripts
```
<br>

## __filename
Path to the current script file.

**Examples**
```js
print(__filename); // /ext/apps/Scripts/path.js
```
<br>

# SDK compatibility methods {#js_builtin_sdk_compatibility}

## sdkCompatibilityStatus()
Checks compatibility between the script and the JS SDK that the firmware provides.

**Returns**
- `"compatible"` if the script and the JS SDK are compatible
- `"firmwareTooOld"` if the expected major version is larger than the version of the firmware, or if the expected minor version is larger than the version of the firmware
- `"firmwareTooNew"` if the expected major version is lower than the version of the firmware

**Examples**
```js
sdkCompatibilityStatus(0, 3); // "compatible"
```
<br>

## isSdkCompatible()
Checks compatibility between the script and the JS SDK that the firmware provides in a boolean fashion.

**Examples**
```js
isSdkCompatible(0, 3); // true
```
<br>

## checkSdkCompatibility()
Asks the user whether to continue executing the script if the versions are not compatible. Does nothing if they are.

**Examples**
```js
checkSdkCompatibility(0, 3);
```
<br>

## doesSdkSupport()
Checks whether all of the specified extra features are supported by the interpreter.

**Examples**
```js
doesSdkSupport(["gui-widget"]); // true
```
<br>

## checkSdkFeatures()
Checks whether all of the specified extra features are supported by the interpreter, asking the user if they want to continue running the script if they're not.

**Examples**
```js
checkSdkFeatures(["gui-widget"]);
```
