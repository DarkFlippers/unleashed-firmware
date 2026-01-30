# Changelog

All notable changes to Bad Duck3 will be documented in this file.

## [1.1.0] - 2026-01-30

### Added
- **Bluetooth Low Energy (BLE) HID support**
  - Wireless payload execution
  - Device name spoofing
  - MAC address randomization
  - PIN display during pairing
- Full feature parity with USB mode

### Changed
- Renamed from "Bad Duck3 v1.0" to "Bad Duck3"
- Unified codebase for USB and BLE

### Technical
- Ported BLE HID implementation from BadKB v1
- Integrated Flipper's native BLE stack
- Added connection mode selection in config

## [1.0.0] - 2026-01-29

### Added
- **DuckyScript 3.0 interpreter** - First implementation for Flipper Zero
  
#### Control Flow
- `LOOP n` / `END_LOOP` - Fixed iteration loops
- `WHILE (condition)` / `END_WHILE` - Conditional loops
- `IF (condition)` / `ELSE` / `END_IF` - Conditionals
- `BREAK` - Exit current loop
- `CONTINUE` - Skip to next iteration
- Nesting support up to 8 levels

#### Variables
- `VAR $name = value` - Integer variables
- `VAR $name = "text"` - String variables
- `VAR $name = (expression)` - Computed values
- 32 variables maximum
- Built-in: `$_RANDOM`, `$_LOOP_COUNT`, `$_LINE`

#### Expressions
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `<`, `>`, `<=`, `>=`, `==`, `!=`
- Logical: `&&`, `||`

### Compatibility
- Full DuckyScript 1.0 backward compatibility
- All Flipper BadUSB extensions supported
- Existing scripts work without modification

### Technical
- Clean standalone app (does not modify BadUSB/BadKB)
- Based on RPi Ducky interpreter architecture
- Memory-efficient design for Flipper's constraints

---

## Roadmap

### Planned for v1.2.0
- [ ] `FUNCTION` / `END_FUNCTION` - User-defined functions
- [ ] String interpolation in `STRING $var`
- [ ] `$_OS` target detection (if feasible)
- [ ] Payload library integration

### Planned for v1.3.0
- [ ] Script encryption
- [ ] Conditional compilation (`#IFDEF`)
- [ ] Include files (`#INCLUDE`)

### Under Consideration
- [ ] Twin Duck mode (HID + Mass Storage)
- [ ] Script debugger
- [ ] Visual script builder on-device

---

## Version History

| Version | Date | Highlights |
|---------|------|------------|
| 1.1.0 | 2026-01-30 | BLE support added |
| 1.0.0 | 2026-01-29 | Initial DS3.0 implementation |
