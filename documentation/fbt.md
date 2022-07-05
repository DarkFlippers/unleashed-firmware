# Flipper Build Tool

FBT is the entry point for firmware-related commands and utilities.
It is invoked by `./fbt` in firmware project root directory. Internally, it is a wrapper around [scons](https://scons.org/) build system.

## Requirements

Please install Python packages required by assets build scripts: `pip3 install -r scripts/requirements.txt`
Make sure that `gcc-arm-none-eabi` toolchain & OpenOCD executables are in system's PATH.

## NB

* `fbt` constructs all referenced environments & their targets' dependency trees on startup. So, to keep startup time as low as possible, we're hiding construction of certain targets behind command-line options.
* `fbt` always performs `git submodule update --init` on start, unless you set `FBT_NO_SYNC=1` in environment:
    * On Windows, that's `set "FBT_NO_SYNC=1"` in the shell you're running `fbt` from
    * On \*nix, it's `$ FBT_NO_SYNC=1 ./fbt ...`
* `fbt` builds updater & firmware in separate subdirectories in `build`, with their names depending on optimization settings (`COMPACT` & `DEBUG` options). However, for ease of integration with IDEs, latest built variant's directory is always linked as `built/latest`. Additionally, `compile_commands.json` is generated in that folder, which is used for code completion support in IDE.

## Invoking FBT

To build with FBT, call it specifying configuration options & targets to build. For example,

`./fbt --with-updater COMPACT=1 DEBUG=0 VERBOSE=1 updater_package copro_dist`

To run cleanup (think of `make clean`) for specified targets, add `-c` option.

## FBT targets

FBT keeps track of internal dependencies, so you only need to build the highest-level target you need, and FBT will make sure everything they depend on is up-to-date.

### High-level (what you most likely need)

- `fw_dist` - build & publish firmware to `dist` folder. This is a default target, when no other are specified
- `updater_package` - build self-update package. _Requires `--with-updater` option_
- `copro_dist` - bundle Core2 FUS+stack binaries for qFlipper
- `flash` - flash attached device with OpenOCD over ST-Link
- `flash_usb` - build, upload and install update package to device over USB.  _Requires `--with-updater` option_
- `debug` - build and flash firmware, then attach with gdb with firmware's .elf loaded
- `debug_updater` - attach gdb with updater's .elf loaded. _Requires `--with-updater` option_
- `debug_other` - attach gdb without loading any .elf. Allows to manually add external elf files with `add-symbol-file` in gdb.
- `blackmagic` - debug firmware with Blackmagic probe (WiFi dev board)
- `openocd` - just start OpenOCD
- `get_blackmagic` - output blackmagic address in gdb remote format. Useful for IDE integration

### Firmware targets

- `firmware_extapps` - build all plug-ins as separate .elf files
    - `firmware_snake_game`, etc - build single plug-in as .elf by its name
    - Check out `--extra-ext-apps` for force adding extra apps to external build 
    - `firmware_snake_game_list`, etc - generate source + assembler listing for app's .elf
- `flash`, `firmware_flash` - flash current version to attached device with OpenOCD over ST-Link
- `flash_blackmagic` - flash current version to attached device with Blackmagic probe
- `firmware_all`, `updater_all` - build basic set of binaries
- `firmware_list`, `updater_list` - generate source + assembler listing

### Assets

- `resources` - build resources and their Manifest
    - `dolphin_ext` - process dolphin animations for SD card 
- `icons` - generate .c+.h for icons from png assets
- `proto` - generate .pb.c+.pb.h for .proto sources
- `proto_ver` - generate .h with protobuf version 
- `dolphin_internal`, `dolphin_blocking` - generate .c+.h for corresponding dolphin assets
 

## Command-line parameters

- `--options optionfile.py` (default value `fbt_options.py`) - load file with multiple configuration values
- `--with-updater` - enables updater-related targets and dependency tracking. Enabling this options introduces extra startup time costs, so use it when bundling update packages. Or if you have a fast computer and don't care about a few extra seconds of startup time
- `--extra-int-apps=app1,app2,appN` - forces listed apps to be built as internal with `firmware` target
- `--extra-ext-apps=app1,app2,appN` - forces listed apps to be built as external with `firmware_extapps` target


## Configuration 

Default configuration variables are set in configuration file `fbt_options.py`. 
Values set on command-line have higher precedence over configuration file.

You can find out available options with `./fbt -h`.

### Firmware application set

You can create customized firmware builds by modifying application list to be included in the build. Application presets are configured with `FIRMWARE_APPS` option, which is a map(configuration_name:str -> application_list:tuple(str)). To specify application set to use in a build, set `FIRMWARE_APP_SET` to its name.
For example, to build firmware image with unit tests, run `./fbt FIRMWARE_APP_SET=unit_tests`.

Check out `fbt_options.py` for details.
