
# How to Build by yourself:

## Install required software

- Git - [Download](https://git-scm.com/downloads) for Windows, on Linux/Mac install via package manager (`brew`, `apt`, ...)

For development:
- Git
- Python3
- VSCode

## Clone the Repository

You should clone with 
```shell
$ git clone --recursive https://github.com/DarkFlippers/unleashed-firmware.git
```

# Build on Linux/macOS

Check out `documentation/fbt.md` for details on building and flashing firmware. 

### Compile everything for development

```sh
./fbt FIRMWARE_APP_SET=debug_pack updater_package
```

### Compile everything for release + get updater package to update from microSD card

```sh
./fbt COMPACT=1 DEBUG=0 updater_package
```

Check `dist/` for build outputs.

Use **`flipper-z-{target}-update-{suffix}.tgz`** to flash your device.


# Build on Windows

Check out `documentation/fbt.md` for details on building and flashing firmware. 

### Compile everything for development

```sh
.\fbt.cmd FIRMWARE_APP_SET=debug_pack updater_package
```

### Compile everything for release + get updater package to update from microSD card

```sh
.\fbt.cmd COMPACT=1 DEBUG=0 updater_package
```

Check `dist/` for build outputs.

Use **`flipper-z-{target}-update-{suffix}.tgz`** to flash your device.



If compilation fails, make sure all submodules are all initialized. Either clone with `--recursive` or use `git submodule update --init --recursive`.
