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

**Modes 7, 8, 9 and 10 - forced resync packs (DoorHan, Phantom / Hiland):**

These four do not step the counter at all, they send a fixed pack of signals. A receiver resyncs only when it hears two transmissions with consecutive counters, so a pack is built out of consecutive pairs, spaced so every hop stays inside the receiver resync window and the pairs together cover the whole counter space. That bypasses the receiver counter from any state. The counter in your file is left where it is, so every press sends the same pack and pulls the receiver back to the same place, and the file stays usable.

Each pack is the sequence of codes a clone remote sends for that board family, reproduced code for code - same counters, same button codes. Modes 7 to 9 are the DoorHan Smart clone, mode 10 is a Phantom / Hiland clone. `X` below is the counter in your file.

The clone sends the two long packs with each code two or three times in a row, for air margin. Here they go out once: the whole pack is built in one piece of memory, and tripling it would ask for 20KB instead of 8KB. Sending the pack over again is not a substitute - a receiver that has already resynced reads the second pass as a fresh walk and operates the gate a second time, where a code repeated back to back is a duplicate it ignores. The short pack keeps its repeats, it has the room.

**Mode 7:**
- For the oldest DoorHan boards
- 12 codes: the pair at your counter `(X / X+1)`, then four pairs walking forward in steps of 15998, then the pair two counters behind yours `(X-2 / X-1)`. The first and the last pair carry your button code, the four walking pairs carry button 0
- About 1.3s per press

**Mode 8:**
- For DHRE, most of the newer boards, and the swing gate clones - all of them send this same pack
- 12 codes: the pair at your counter `(X / X+1)`, then four pairs stepped `+0x3334` apart, then back to your counter, where `X+1` with your button code is the signal that lands
- About 1.3s per press
- This is exactly what `CounterMode: 7` did in earlier releases. If you have a file with `CounterMode: 7` that worked, change it to `CounterMode: 8`

**Mode 9:**
- For CV01 roller shutter boards and other old boards
- 3 codes, all carrying your button code: your counter with its top bit set (`X + 0x8000`), then `X-1`, then `X+1`. No walk over the counter space
- Each of the three goes out twice, as the clone sends it - about 0.65s per press

**Mode 10 (Phantom / Hiland):**
- 2 codes, both carrying your button code: `0xFFFE`, then `0xFFFF`. These are absolute counters, not offsets - this is the one pack that ignores the counter in your file, as the clone does (the same clone sits in the low single digits in its normal mode)
- Each of the two goes out three times, as the clone sends it - about 0.4s per press
- No walk over the counter space, so one press resyncs a receiver holding anything in the 32768 below `0xFFFE`, which is where a receiver that has drifted forward of your remote sits. If it does not take, send once more

Holding the Send button repeats the pack for as long as you hold it, which a receiver that resynced on the first pass reads as another press. Pressing Send again while a pack is going out aborts it.


---

### 5. V2 Phoenix (Phox)

**Mode 0 (Default):**
- Standard - acts like regular remote
- Uses rolling counter multiplier from global settings (default +1)
- Counter increments based on the multiplier value (default +1)
- Resets to 0 when overflow occurs (> 0xFFFF)

**Mode 1 (ofex like):**
- Counter sequence: `0x0000 / 0x0001 / 0xFFFE / 0xFFFF`
- Verified as working

**Mode 2 (0 - 4):**
- Counter sequence: `0x0000 / 0x0001 / 0x0002 / 0x0003 / 0x0004`
- Might work (let us know!)

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
