# Flipper bootloader

What it does?

- [+] Hardware initialization
- [ ] Firmware CRC check
- [+] Firmware update
- [ ] Interactive UI
- [+] Boot process LED indicators
- [ ] FS check
- [ ] Recovery mode

# Targets

| Name      | Bootloader    | Firmware      | Reset | DFU           |
|           | Address       | Address       | Combo | Combo         |
---------------------------------------------------------------------
| f2        | 0x08000000    | 0x00008000    | L+R   | L+R, hold R   |

Target independend code and headers in `src`and `target/include` folders.

# Building

## With dev docker image:

`docker-compose exec dev make -C bootloader`

## With toolchain installed in path:

`make`

## Build Options

- `DEBUG` - 0/1 - enable or disable debug build. Default is 1.
- `TARGET` - string - target to build. Default is `f2`.

# Flashing 

Using stlink(st-flash):

`make flash`

# Debug

Using stlink (st-util + gdb):

`make debug`
