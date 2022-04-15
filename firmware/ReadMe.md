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

Also there is a "hardware" ST bootloader combo available even on a bricked or empty device: L+Ok+Back, release Back, Left.
Target independent code and headers in `target/include` folders. More details in `documentation/KeyCombo.md`

# Building

## With dev docker image:

`docker-compose exec dev make -C firmware`

## With toolchain installed in path:

`make -C firmware`

## Build Options

- `DEBUG` - 0/1 - enable or disable debug build. Default is 1.
- `COMPACT` - 0/1 - enable or disable compiler optimizations. Significantly reduces binary size. Default is 0.
- `TARGET` - string - target to build. Default is `f7`.
- `RAM_EXEC` - 0/1 - whether to build full firmware or RAM-based stage for firmware update. 0 is default, builds firmware.

# Building self-update package

`make DEBUG=0 COMPACT=1 updater_package`

# Flashing 

Using SWD (STLink):

`make -C firmware flash`

Or use DFU (USB):

`make -C firmware upload`

# Debug

Using SWD (STLink):

`make -C firmware debug`
