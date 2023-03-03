# Flipper Build Tool

FBT is the entry point for firmware-related commands and utilities.
It is invoked by `./fbt` in the firmware project root directory. Internally, it is a wrapper around [scons](https://scons.org/) build system.

## Environment

To use `fbt`, you only need `git` installed in your system.

`fbt` by default downloads and unpacks a pre-built toolchain, and then modifies environment variables for itself to use it. It does not contaminate your global system's path with the toolchain.
 > However, if you wish to use tools supplied with the toolchain outside `fbt`, you can open an *fbt shell*, with properly configured environment.
 >    - On Windows, simply run `scripts/toolchain/fbtenv.cmd`.
 >    - On Linux & MacOS, run `source scripts/toolchain/fbtenv.sh` in a new shell.
 
 If your system is not supported by pre-built toolchain variants or you want to use custom versions of dependencies, you can `set FBT_NOENV=1`. `fbt` will skip toolchain & environment configuration and will expect all tools to be available on your system's `PATH`. *(this option is not available on Windows)*
 
 If `FBT_TOOLCHAIN_PATH` variable is set, `fbt` will use that directory to unpack toolchain into. By default, it downloads toolchain into `toolchain` subdirectory repo's root.

If you want to enable extra debug output for `fbt` and toolchain management scripts, you can `set FBT_VERBOSE=1`.

`fbt` always performs `git submodule update --init` on start, unless you set `FBT_NO_SYNC=1` in the environment:
  - On Windows, it's `set "FBT_NO_SYNC=1"` in the shell you're running `fbt` from
  - On \*nix, it's `$ FBT_NO_SYNC=1 ./fbt ...`

 > There are more variables controlling basic `fbt` behavior. See `fbt` & `fbtenv` scripts' sources for details.


## Invoking FBT

To build with FBT, call it and specify configuration options & targets to build. For example:

`./fbt COMPACT=1 DEBUG=0 VERBOSE=1 updater_package copro_dist`

To run cleanup (think of `make clean`) for specified targets, add the `-c` option.

## Build directories

`fbt` builds updater & firmware in separate subdirectories in `build`, and their names depend on optimization settings (`COMPACT` & `DEBUG` options). However, for ease of integration with IDEs, the latest built variant's directory is always linked as `built/latest`. Additionally, `compile_commands.json` is generated in that folder (it is used for code completion support in IDEs).
 
`build/latest` symlink & compilation database are only updated upon *firmware build targets* - that is, when you're re-building the firmware itself. Running other tasks, like firmware flashing or building update bundles *for a different debug/release configuration or hardware target*, does not update `built/latest` dir to point to that configuration.

## VSCode integration

`fbt` includes basic development environment configuration for VS Code. Run `./fbt vscode_dist` to deploy it. That will copy the initial environment configuration to the `.vscode` folder. After that, you can use that configuration by starting VS Code and choosing the firmware root folder in the "File > Open Folder" menu.

- On the first start, you'll be prompted to install recommended plugins. We highly recommend installing them for the best development experience. _You can find a list of them in `.vscode/extensions.json`._
- Basic build tasks are invoked in the Ctrl+Shift+B menu.
- Debugging requires a supported probe. That includes:
  - Wi-Fi devboard with stock firmware (blackmagic).
  - ST-Link and compatible devices.
  - J-Link for flashing and debugging (in VSCode only). _Note that J-Link tools are not included with our toolchain and you have to [download](https://www.segger.com/downloads/jlink/) them yourself and put them on your system's PATH._
- Without a supported probe, you can install firmware on Flipper using the USB installation method.

## FBT targets

**`fbt`** keeps track of internal dependencies, so you only need to build the highest-level target you need, and **`fbt`** will make sure everything they depend on is up-to-date.

### High-level (what you most likely need)

- `fw_dist` - build & publish firmware to the `dist` folder. This is a default target when no others are specified.
- `fap_dist` - build external plugins & publish to the `dist` folder.
- `updater_package`, `updater_minpackage` - build a self-update package. The minimal version only includes the firmware's DFU file; the full version also includes a radio stack & resources for the SD card.
- `copro_dist` - bundle Core2 FUS+stack binaries for qFlipper.
- `flash` - flash the attached device with OpenOCD over ST-Link.
- `flash_usb`, `flash_usb_full` - build, upload and install the update package to the device over USB. See details on `updater_package` and `updater_minpackage`.
- `debug` - build and flash firmware, then attach with gdb with firmware's .elf loaded.
- `debug_other`, `debug_other_blackmagic` - attach GDB without loading any `.elf`. It will allow you to manually add external `.elf` files with `add-symbol-file` in GDB.
- `updater_debug` - attach GDB with the updater's `.elf` loaded.
- `blackmagic` - debug firmware with Blackmagic probe (WiFi dev board).
- `openocd` - just start OpenOCD.
- `get_blackmagic` - output the blackmagic address in the GDB remote format. Useful for IDE integration.
- `get_stlink` - output serial numbers for attached STLink probes. Used for specifying an adapter with `OPENOCD_ADAPTER_SERIAL=...`.
- `lint`, `format` - run clang-format on the C source code to check and reformat it according to the `.clang-format` specs.
- `lint_py`, `format_py` - run [black](https://black.readthedocs.io/en/stable/index.html) on the Python source code, build system files & application manifests.
- `firmware_pvs` - generate a PVS Studio report for the firmware. Requires PVS Studio to be availabe on your system's `PATH`.
- `cli` - start a Flipper CLI session over USB.

### Firmware targets

- `faps` - build all external & plugin apps as [`.faps`](./AppsOnSDCard.md#fap-flipper-application-package).
- **`fbt`** also defines per-app targets. For example, for an app with `appid=snake_game` target names are:
  - `fap_snake_game`, etc. - build single app as `.fap` by its application ID.
  - Check out [`--extra-ext-apps`](#command-line-parameters) for force adding extra apps to external build.
  - `fap_snake_game_list`, etc - generate source + assembler listing for app's `.fap`.
- `flash`, `firmware_flash` - flash the current version to the attached device with OpenOCD over ST-Link.
- `jflash` - flash the current version to the attached device with JFlash using a J-Link probe. The JFlash executable must be on your `$PATH`.
- `flash_blackmagic` - flash the current version to the attached device with a Blackmagic probe.
- `firmware_all`, `updater_all` - build a basic set of binaries.
- `firmware_list`, `updater_list` - generate source + assembler listing.
- `firmware_cdb`, `updater_cdb` - generate a `compilation_database.json` file for external tools and IDEs. It can be created without actually building the firmware.

### Assets

- `resources` - build resources and their manifest files
  - `dolphin_ext` - process dolphin animations for the SD card
- `icons` - generate `.c+.h` for icons from PNG assets
- `proto` - generate `.pb.c+.pb.h` for `.proto` sources
- `proto_ver` - generate `.h` with a protobuf version
- `dolphin_internal`, `dolphin_blocking` - generate `.c+.h` for corresponding dolphin assets

## Command-line parameters

- `--options optionfile.py` (default value `fbt_options.py`) - load a file with multiple configuration values
- `--extra-int-apps=app1,app2,appN` - force listed apps to be built as internal with the `firmware` target
- `--extra-ext-apps=app1,app2,appN` - force listed apps to be built as external with the `firmware_extapps` target
- `--proxy-env=VAR1,VAR2` - additional environment variables to expose to subprocesses spawned by `fbt`. By default, `fbt` sanitizes the execution environment and doesn't forward all inherited environment variables. You can find the list of variables that are always forwarded in the `environ.scons` file.

## Configuration

Default configuration variables are set in the configuration file: `fbt_options.py`.
Values set in the command line have higher precedence over the configuration file.

You can find out available options with `./fbt -h`.

### Firmware application set

You can create customized firmware builds by modifying the list of applications to be included in the build. Application presets are configured with the `FIRMWARE_APPS` option, which is a `map(configuration_name:str -> application_list:tuple(str))`. To specify an application set to use in the build, set `FIRMWARE_APP_SET` to its name.
For example, to build a firmware image with unit tests, run `./fbt FIRMWARE_APP_SET=unit_tests`.

Check out `fbt_options.py` for details.
