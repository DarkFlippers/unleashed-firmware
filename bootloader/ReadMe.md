# Flipper bootloader

What it does?

- [x] Hardware initialization
- [x] Boot process LED indicators
- [x] Firmware update
- [x] Errata crutches
- [ ] Recovery mode

# Targets

| Name      | Bootloader    | Firmware      | Reset     | DFU               |
|           | Address       | Address       | Combo     | Combo             |
-----------------------------------------------------------------------------
| f7        | 0x08000000    | 0x00008000    | L+Back    | L+Back, hold L    |

Also there is a ST bootloader combo available on empty device: L+Ok+Back, release Back,Left.
Target independent code and headers in `src` and `target/include` folders.

# Building

## With dev docker image:

`docker-compose exec dev make -C bootloader`

## With toolchain installed in path:

`make -C bootloader `

## Build Options

- `DEBUG` - 0/1 - enable or disable debug build. Default is 1.
- `TARGET` - string - target to build. Default is `f4`.

# Flashing 

Using SWD (STLink):

`make -C bootloader flash`

Or use DFU (USB):

`make -C bootloader upload`

# Debug

Using SWD (STLink):

`make -C bootloader debug`
