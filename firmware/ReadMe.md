# Flipper firmware

What does it do?

- [x] RTOS
- [x] FuriHAL
- [x] FuriCore
- [x] Services
- [x] Applications

# Targets

| Name      | Firmware Address  | Reset Combo           | DFU Combo             |
|-----------|-------------------|-----------------------|-----------------------|
| f7        | 0x08000000        | L+Back, release both  | L+Back, release Back  |

Also, there is a "hardware" ST bootloader combo available even on a bricked or empty device: L+Ok+Back, release Back, Left.
Target independent code and headers in `target/include` folders. More details in `documentation/KeyCombo.md`

# Building

Check out `documentation/fbt.md` on how to build and flash firmware.
