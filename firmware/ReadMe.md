# Flipper firmware

What it does?

- [x] RTOS
- [x] FuriHAL
- [x] FuriCore
- [x] Services
- [x] Applications

# Targets

| Name      | Bootloader    | Firmware      | Reset     | DFU               |
|           | Address       | Address       | Combo     | Combo             |
-----------------------------------------------------------------------------
| f7        | 0x08000000    | 0x00008000    | L+Back    | L+Back, hold L    |

Also there is a ST bootloader combo available on empty device: L+Ok+Back, release Back,Left.
Target independent code and headers in `target/include` folders.

# Building

## With dev docker image:

`docker-compose exec dev make -C firmware`

## With toolchain installed in path:

`make -C firmware`

## Build Options

- `DEBUG` - 0/1 - enable or disable debug build. Default is 1.
- `TARGET` - string - target to build. Default is `f7`.

# Flashing 

Using SWD (STLink):

`make -C firmware flash`

Or use DFU (USB):

`make -C firmware upload`

# Debug

Using SWD (STLink):

`make -C firmware debug`
