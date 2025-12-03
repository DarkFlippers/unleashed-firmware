# SubGHz Counter Experimental Mode

## Overview

Experimental Counter Mode is an advanced feature that allows you to customize how rolling codes increment when transmitting SubGHz signals. Different protocols support different counter modes, which can be useful for specific cases, main purpose is to make clone of the original remote that will not conflict with original remote, so both Flipper and original remote can be used at same time in rolling code systems without desync or other issues.

**Be aware, do not experiment with equipment you don't have access to, if you are not sure what mode works for you and can't reprogram your original remote to the receiver - do not use these modes!!!**

In case you have access to the receiver and can do some tests, try some of these modes on your system and let us know how it works for you, in github issues or our communities, thanks!

## How to Use Experimental Counter Mode

To enable a specific counter mode, you need to manually edit your `.sub` file and add a `CounterMode` line.

### Steps:

1. Locate your `.sub` file (in `/ext/subghz/` on your Flipper Zero SD card)
2. Open the file in a text editor
3. Add the following line at the end of the file:
   ```
   CounterMode: X
   ```
   Where `X` is the mode number (0, 1, 2, etc.)

### Example .sub File:

```
Filetype: Flipper SubGhz Key File
Version: 1
Frequency: 433920000
Preset: FuriHalSubGhzPresetOok650Async
Protocol: Nice FloR-S
Bit: 52
Key: AA AA AA AA AA AA AA
CounterMode: 1
```

## Supported Protocols and Counter Modes

### 1. Nice Flor S

**Mode 0 (Default):**
- Standard - acts like regular remote
- Uses rolling counter multiplier from global settings (default +1)
- Counter increments based on the multiplier value (default +1)
- Resets to 0 when overflow occurs (> 0xFFFF)

**Mode 1 (floxi2r):**
- Counter sequence: `0x0001 / 0xFFFE`
- For receiver model floxi2r

**Mode 2 (ox2):**
- Counter sequence: `0x0000 / 0x0001`
- For receiver model ox2

---

### 2. Came Atomo

**Mode 0 (Default):**
- Standard - acts like regular remote
- Uses rolling counter multiplier from global settings (default +1)
- Counter increments based on the multiplier value (default +1)
- Resets to 0 when overflow occurs (> 0xFFFF)

**Mode 1:**
- Counter sequence: `0x0000 / 0x0001 / 0xFFFE / 0xFFFF`
- Works with external CAME RE432 receiver, may work with other type of receivers

**Mode 2:**
- Counter sequence: `0x807B / 0x807C / 0x007B / 0x007C`
- Works with external CAME RE432 receiver, may work with other type of receivers

**Mode 3:**
- Counter freeze - do not increment

---

### 3. Alutech AT-4N

**Mode 0 (Default):**
- Standard - acts like regular remote
- Uses rolling counter multiplier from global settings (default +1)
- Counter increments based on the multiplier value (default +1)
- Resets to 0 when overflow occurs (> 0xFFFF)

**Mode 1:**
- Counter sequence: `0x0000 / 0x0001 / 0xFFFE / 0xFFFF`
- For receiver model MCSW-3.3M

**Mode 2:**
- Counter sequence: `0x0000 / 0x0001 / 0x0002 / 0x0003 / 0x0004 / 0x0005`
- For other receiver boards

---

### 4. KeeLoq

**Mode 0 (Default):**
- Standard - acts like regular remote
- Uses rolling counter multiplier from global settings (default +1)
- Counter increments based on the multiplier value (default +1)
- Resets to 0 when overflow occurs (> 0xFFFF)

**Mode 1:**
- Counter sequence: `0x0000 / 0x0001 / 0xFFFE / 0xFFFF`
- Might work with some systems (let us know!)

**Mode 2:**
- Incremental mode: `+0x3333` each transmission
- Adds 0x3333 (13107 in decimal) to counter each time
- Might work with Doorhan, seen in some "universal remotes"

**Mode 3:**
- Counter sequence: `0x8006 / 0x8007 / 0x0006 / 0x0007`
- Might work with some systems like Hormann EcoStar

**Mode 4:**
- Counter sequence: `0x807B / 0x807C / 0x007B / 0x007C`
- Might work with some systems like Nice Smilo

**Mode 5:**
- Counter sequence: `0x0000 / 0xFFFF`
- Alternates between 0 and maximum value (65535)
- Might work with some systems (let us know!)

**Mode 6:**
- Counter freeze - do not increment

---

## Notes and Warnings

### Important Considerations:

1. **Default Behavior:**
   - If you don't specify a `CounterMode`, Regular remote simulation (cnt +1) is used by default
   - Regular remote simulation is the standard mode and works with most cases

2. **Protocol Compatibility:**
   - Not all protocols support all counter modes
   - Using an unsupported mode number may result in unexpected behavior
   - Always test your configuration before relying on it

### Troubleshooting:

- If your file doesn't work after adding `CounterMode`:
  1. Double-check the syntax: `CounterMode: X` (with space after colon)
  2. Verify the mode number is valid for your protocol


---

## Example Configurations

### Example 1: Nice Flor S with Mode 1
```
Filetype: Flipper SubGhz Key File
Version: 1
Frequency: 433920000
Preset: FuriHalSubGhzPresetOok650Async
Protocol: Nice FloR-S
Bit: 52
Key: 01 23 45 67 89 AB CD
CounterMode: 1
```

### Example 2: KeeLoq with Mode 2 (+0x3333 (Doorhan))
```
Filetype: Flipper SubGhz Key File
Version: 1
Frequency: 433920000
Preset: FuriHalSubGhzPresetOok650Async
Protocol: KeeLoq
Bit: 64
Key: DE AD BE EF CA FE BA BE
Manufacture: Doorhan
CounterMode: 2
```

---

## FAQ

**Q: Will this damage my remote or receiver?**
A: Yes it may do that - depending on the receiver, it may desync your remote, be aware and don't experiment with equipment you don't have access to

**Q: Which mode should I use?**
A: Do not use those mods if you are not sure

**Q: What happens if I use an invalid mode number?**
A: Last mode from the list will be used

**Q: Do I need to add CounterMode to every .sub file?**
A: No, only add it if you need non-default behavior. Mode 0 is automatic if not specified.

---

*Made for Unleashed FW, please mention source when copying*
