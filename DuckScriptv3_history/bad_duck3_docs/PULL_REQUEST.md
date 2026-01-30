# Pull Request: Bad Duck3 - DuckyScript 3.0 Support for Flipper Zero

## Summary

This PR introduces **Bad Duck3**, the first DuckyScript 3.0 implementation for Flipper Zero. It adds a new standalone application that supports modern DuckyScript control flow features while maintaining full USB and BLE HID capability.

## Motivation

The Flipper Zero community has been limited to DuckyScript 1.0 for over 4 years, requiring users to:
- Write thousands of repetitive lines for simple loops
- Use external tools to generate expanded scripts
- Lack conditional logic for adaptive payloads

DuckyScript 3.0 was released by Hak5 in 2022 with the USB Rubber Ducky Gen 3, but no Flipper implementation existed until now.

## What's New

### DuckyScript 3.0 Features
- `LOOP` / `END_LOOP` - Execute blocks N times
- `WHILE` / `END_WHILE` - Conditional loops
- `VAR` - Variable declaration and assignment
- `IF` / `ELSE` / `END_IF` - Conditional execution
- `BREAK` / `CONTINUE` - Loop control
- Expression evaluation with arithmetic and comparison operators
- Built-in variables (`$_RANDOM`, `$_LOOP_COUNT`, etc.)

### Transport Support
- **USB HID** - Standard wired keyboard emulation
- **BLE HID** - Wireless Bluetooth keyboard emulation (ported from BadKB)

### Backward Compatibility
- All DuckyScript 1.0 commands remain fully supported
- Existing scripts work without modification
- Flipper-specific extensions (ALTSTRING, SYSRQ, etc.) preserved

## Example: Before vs After

**Before (DS 1.0) - 100 lines:**
```duckyscript
ENTER
DELAY 500
ENTER
DELAY 500
ENTER
DELAY 500
... (97 more times)
```

**After (DS 3.0) - 4 lines:**
```duckyscript
LOOP 100
    ENTER
    DELAY 500
END_LOOP
```

## Technical Implementation

- **New standalone app** - Does not modify existing BadUSB/BadKB
- **Clean interpreter architecture** - Based on proven RPi Ducky implementation
- **Memory efficient** - Loop stack and variable store fit within Flipper's constraints
- **BLE implementation** - Ported from BadKB v1 with full pairing support

## Files Changed

```
applications/main/bad_duck3/       # New application
├── application.fam
├── bad_duck3_app.c
├── bad_duck3_app_i.h
├── helpers/
│   ├── ducky_script_3.c          # DS3.0 interpreter
│   ├── ducky_script_3.h
│   ├── ducky_script_commands.c   # Command handlers
│   ├── bad_duck3_hid.c           # HID abstraction
│   └── bad_duck3_hid.h
├── scenes/
│   └── ...
└── views/
    └── ...

tools/duckyscript_generator/       # Helper tooling (optional)
```

## Testing

- [x] USB HID - Windows 10/11
- [x] USB HID - macOS
- [x] USB HID - Linux
- [x] BLE HID - Windows 10/11
- [x] BLE HID - macOS  
- [x] BLE HID - Android
- [x] BLE HID - iOS
- [x] Nested loops (8 levels deep)
- [x] Variable operations
- [x] Conditional execution
- [x] BREAK/CONTINUE
- [x] Backward compatibility with DS1.0 scripts

## Screenshots

*[Add screenshots of the app UI here]*

## Related Issues

- Addresses long-standing community requests for loop support
- Complements existing BadKB BLE functionality
- First implementation of DS3.0 on any Flipper firmware

## Checklist

- [x] Code follows project coding style
- [x] No modifications to existing apps
- [x] Memory usage within acceptable limits
- [x] Tested on hardware
- [x] Documentation included

---

**This represents the first DuckyScript 3.0 implementation for Flipper Zero after 4+ years of the platform's existence.**
