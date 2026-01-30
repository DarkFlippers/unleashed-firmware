# 📢 Announcement Post Templates

## Discord / Telegram

---

🦆 **Bad Duck3 Released - DuckyScript 3.0 for Flipper Zero!**

After 4+ years of waiting, proper scripting finally comes to Flipper!

**What's new:**
```
✅ LOOP / WHILE loops
✅ IF / ELSE conditionals  
✅ Variables with expressions
✅ BREAK / CONTINUE
✅ USB + BLE support
✅ Full DS1.0 backward compatibility
```

**Before (DS 1.0):** 1000 lines for 1000 ENTER presses
**After (DS 3.0):** 
```
LOOP 1000
    ENTER
    DELAY 100
END_LOOP
```

🔗 **Get it:** https://github.com/dutchpatriot/unleashed-firmware/tree/feature/bad-duck3-v1.1

First ever DS3.0 implementation on Flipper. USB and Bluetooth. Let me know what you build! 🐬

---

## Reddit (r/flipperzero)

---

**Title:** [RELEASE] Bad Duck3 - DuckyScript 3.0 for Flipper Zero (loops, variables, conditionals!)

**Body:**

Hey everyone!

I've been working on something the community has wanted since day one: **proper DuckyScript 3.0 support for Flipper Zero**.

After 4+ years of being stuck with line-by-line DuckyScript 1.0, Bad Duck3 brings modern scripting to Flipper:

### Features
- **LOOP / END_LOOP** - No more 10,000 line scripts for repetitive tasks
- **WHILE / END_WHILE** - Conditional loops
- **IF / ELSE / END_IF** - Branching logic
- **VAR** - Variables with arithmetic and comparison
- **BREAK / CONTINUE** - Loop control
- **USB + BLE** - Both wired and wireless HID

### Example
Instead of writing 1000 lines:
```
ENTER
DELAY 100
ENTER
DELAY 100
... (998 more times)
```

Now just:
```
LOOP 1000
    ENTER
    DELAY 100
END_LOOP
```

### Links
- **GitHub:** https://github.com/dutchpatriot/unleashed-firmware/tree/feature/bad-duck3-v1.1
- Based on Unleashed firmware

### Installation
Clone the branch and build with `./fbt flash_usb_full`

All existing DS1.0 scripts still work - it's fully backward compatible.

Let me know if you run into any issues or have feature requests!

---

## GitHub Release Notes

---

# Bad Duck3 v1.1.0

**The first DuckyScript 3.0 implementation for Flipper Zero**

## 🎉 Highlights

This release adds **Bluetooth Low Energy support** to Bad Duck3, making it a complete wireless HID injection tool with modern scripting capabilities.

## ✨ New in v1.1.0

- **BLE HID Mode** - Run payloads wirelessly via Bluetooth
- **Device Spoofing** - Customize Bluetooth device name
- **Pairing Support** - PIN display for secure connections

## 📦 What's Included

### DuckyScript 3.0 Features
| Feature | Status |
|---------|--------|
| LOOP / END_LOOP | ✅ |
| WHILE / END_WHILE | ✅ |
| IF / ELSE / END_IF | ✅ |
| Variables (VAR) | ✅ |
| Expressions | ✅ |
| BREAK / CONTINUE | ✅ |

### Transport
| Mode | Status |
|------|--------|
| USB HID | ✅ |
| BLE HID | ✅ |

### Compatibility
- ✅ DuckyScript 1.0 backward compatible
- ✅ Flipper extensions (ALTSTRING, SYSRQ, etc.)
- ✅ All standard BadUSB scripts work

## 📥 Installation

```bash
git clone https://github.com/dutchpatriot/unleashed-firmware.git
cd unleashed-firmware
git checkout feature/bad-duck3-v1.1
./fbt flash_usb_full
```

## 📝 Documentation

- [README](README.md) - Getting started
- [Command Reference](COMMAND_REFERENCE.md) - Full command documentation
- [Changelog](CHANGELOG.md) - Version history

## 🙏 Credits

- RPi Ducky project for interpreter reference
- Unleashed team for BadKB BLE implementation
- Flipper Zero community

---

**Full Changelog:** v1.0.0...v1.1.0
