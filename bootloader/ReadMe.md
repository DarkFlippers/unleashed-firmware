# Flipper bootloader

What it does?

- [+] Hardware initialization
- [+] Boot process LED indicators
- [+] Firmware update
- [ ] Firmware CRC check
- [ ] Interactive UI
- [ ] FS check
- [ ] Recovery mode
- [ ] Errata crutches

# Targets

| Name      | Bootloader    | Firmware      | Reset     | DFU               |
|           | Address       | Address       | Combo     | Combo             |
-----------------------------------------------------------------------------
| f4        | 0x08000000    | 0x00008000    | L+Back    | L+Back, hold L    |

Also there is a ST bootloader combo available on empty device: L+Ok+Back, release Back,Left.
Target independend code and headers in `src`and `target/include` folders.

# Building

## With dev docker image:

`docker-compose exec dev make -C bootloader`

## With toolchain installed in path:

`make -C bootloader `

## Build Options

- `DEBUG` - 0/1 - enable or disable debug build. Default is 1.
- `TARGET` - string - target to build. Default is `f4`.

# Flashing 

Using stlink(st-flash):

`make -C bootloader flash`

Or use ST bootloader:

`make -C bootloader upload`

# Debug

Using stlink (st-util + gdb):

`make -C bootloader  debug`
