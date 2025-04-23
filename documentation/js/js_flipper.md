# Flipper module {#js_flipper}

The module contains methods and values to query device information and properties. Call the `require` function to load the module before first using its methods:

```js
let flipper = require("flipper");
```

# Values

## firmwareVendor
String representing the firmware installed on the device.
Original firmware reports `"flipperdevices"`.
Do **NOT** use this to check the presence or absence of features, refer to [other ways to check SDK compatibility](#js_builtin_sdk_compatibility).

## jsSdkVersion
Version of the JavaScript SDK.
Do **NOT** use this to check the presence or absence of features, refer to [other ways to check SDK compatibility](#js_builtin_sdk_compatibility).

<br>

---

# Methods

## getModel()
Returns the device model.

**Example**
```js
flipper.getModel(); // "Flipper Zero"
```

<br>

## getName()
Returns the name of the virtual dolphin.

**Example**
```js
flipper.getName(); // "Fur1pp44"
```

<br>

## getBatteryCharge()
Returns the battery charge percentage.

**Example**
```js
flipper.getBatteryCharge(); // 100
```
