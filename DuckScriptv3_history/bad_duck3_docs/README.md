# Bad Duck3 🦆

**The first DuckyScript 3.0 implementation for Flipper Zero**

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL%203.0-blue.svg)](LICENSE)
[![Flipper Zero](https://img.shields.io/badge/Flipper-Zero-orange.svg)](https://flipperzero.one)

---

## What is Bad Duck3?

Bad Duck3 brings modern DuckyScript 3.0 capabilities to Flipper Zero, adding loops, variables, and conditionals that have been missing from the platform since its launch.

After 4+ years of the Flipper Zero community being limited to DuckyScript 1.0's line-by-line execution, Bad Duck3 finally enables proper programming constructs for HID payloads.

## Features

### 🔄 Control Flow
| Command | Description |
|---------|-------------|
| `LOOP n` / `END_LOOP` | Execute block n times |
| `WHILE (condition)` / `END_WHILE` | Loop while condition is true |
| `IF (condition)` / `END_IF` | Conditional execution |
| `ELSE` | Alternative branch |
| `BREAK` | Exit current loop |
| `CONTINUE` | Skip to next iteration |

### 📊 Variables
| Command | Description |
|---------|-------------|
| `VAR $name = value` | Declare/assign integer |
| `VAR $name = "text"` | Declare/assign string |
| `VAR $name = ($x + 1)` | Expression assignment |

### 🧮 Expressions
| Operator | Description |
|----------|-------------|
| `+` `-` `*` `/` `%` | Arithmetic |
| `<` `>` `<=` `>=` | Comparison |
| `==` `!=` | Equality |
| `&&` `\|\|` | Logical |

### 🎲 Built-in Variables
| Variable | Description |
|----------|-------------|
| `$_RANDOM` | Random number (0-65535) |
| `$_LOOP_COUNT` | Current loop iteration |
| `$_LINE` | Current line number |

### 📡 Transport
- **USB** - Standard wired HID keyboard
- **Bluetooth LE** - Wireless HID with device spoofing

## Quick Start

### Installation

1. Build the firmware with Bad Duck3:
```bash
git clone https://github.com/dutchpatriot/unleashed-firmware.git
cd unleashed-firmware
git checkout feature/bad-duck3-v1.1
./fbt flash_usb_full
```

2. Or install as standalone FAP (if available):
```
Copy bad_duck3.fap to /ext/apps/Bad_Duck3/
```

### Your First DS3.0 Script

Create `hello_loop.txt`:
```duckyscript
REM DuckyScript 3.0 Demo
DELAY 2000

VAR $count = 5

WHILE ($count > 0)
    STRING Hello from loop iteration 
    ENTER
    DELAY 300
    VAR $count = ($count - 1)
END_WHILE

STRING Done!
ENTER
```

### Running Scripts

1. Open **Bad Duck3** from Apps menu
2. Navigate to your script
3. Select USB or BLE mode
4. Press **Start**

## Examples

### Simple Loop (Your keyboard's new best friend)
```duckyscript
REM Press ENTER 1000 times with delay
LOOP 1000
    ENTER
    DELAY 100
END_LOOP
```

### Conditional Payload
```duckyscript
VAR $target = 1

IF ($target == 1)
    REM Windows payload
    GUI r
    DELAY 500
    STRING notepad
    ENTER
ELSE
    REM Mac payload  
    GUI SPACE
    DELAY 500
    STRING textedit
    ENTER
END_IF
```

### Nested Loops
```duckyscript
REM Print 5x5 grid
LOOP 5
    LOOP 5
        STRING X
    END_LOOP
    ENTER
END_LOOP
```

### Random Delays (Anti-detection)
```duckyscript
LOOP 10
    STRING Typing with random timing...
    ENTER
    VAR $wait = ($_RANDOM % 500)
    VAR $wait = ($wait + 200)
    DELAY $wait
END_LOOP
```

### Counter with Break
```duckyscript
VAR $i = 0
LOOP 1000
    VAR $i = ($i + 1)
    STRING .
    IF ($i >= 50)
        BREAK
    END_IF
END_LOOP
STRING Stopped at 50!
ENTER
```

## BLE Mode

Bad Duck3 supports Bluetooth Low Energy HID, allowing wireless payload execution.

### Pairing
1. Select script → Press Left → Connection: BLE
2. On target device, pair with "Bad Duck3" (or spoofed name)
3. Enter PIN shown on Flipper if prompted
4. Run payload

### Device Spoofing
Configure in app settings:
- Device name (e.g., "Logitech Keyboard")
- MAC address randomization

## Differences from Hak5 DuckyScript 3.0

| Feature | Hak5 | Bad Duck3 |
|---------|------|-----------|
| LOOP/WHILE | ✅ | ✅ |
| Variables | ✅ | ✅ |
| IF/ELSE | ✅ | ✅ |
| FUNCTION | ✅ | 🚧 Planned |
| $_OS detection | ✅ | ❌ Not possible |
| Exfiltration | ✅ | ❌ No storage HID |
| Flipper extensions | ❌ | ✅ ALTSTRING, SYSRQ, etc. |

## Flipper-Specific Commands

All standard Flipper BadUSB extensions remain available:

| Command | Description |
|---------|-------------|
| `ALTSTRING text` | Type via ALT+numpad codes |
| `ALTCODE text` | Same as ALTSTRING |
| `SYSRQ key` | Linux SysRq commands |
| `ID VID:PID Name` | Custom USB device ID |
| `DELAY_RANDOM min max` | Random delay range |

## Troubleshooting

### Script won't run
- Check syntax (matching END_* for each block)
- Ensure file is `.txt` format
- Verify no unsupported commands

### BLE won't connect
- Unpair device and re-pair
- Try different target device
- Check Flipper battery level

### Loops seem slow
- Reduce DELAY values
- Consider using `DEFAULT_DELAY` once
- Check for nested delays

## Contributing

Contributions welcome! Areas of interest:
- [ ] FUNCTION/END_FUNCTION support
- [ ] String variable interpolation in STRING command
- [ ] Additional built-in variables
- [ ] Payload library

## Credits

- **dutchpatriot** - Bad Duck3 implementation
- **Raspberry Pi Ducky** - DS3.0 interpreter reference
- **Flipper Unleashed Team** - BadKB BLE implementation
- **Hak5** - DuckyScript language specification

## License

GPL-3.0 - See [LICENSE](LICENSE)

---

**Bad Duck3** - Because your Flipper deserves a real scripting language. 🦆
