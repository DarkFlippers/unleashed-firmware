
# How to Build by yourself

## Install required software

- Git - [Download](https://git-scm.com/downloads) for Windows, on Linux/Mac install via package manager (`brew`, `apt`, ...)

For development:

- Git
- Python3
- VSCode

## Clone the Repository

You should clone with

```shell
git clone --recursive https://github.com/RogueMaster/flipperzero-firmware-wPlugins.git
```

## Building

Check out [documentation/fbt.md](fbt.md) for details on building and flashing firmware.

### Compile

Linux.MacOS

```shell
./fbt
```

Windows (powershell)

```powershell
.\fbt.cmd
```

Production standard Options

```shell
./fbt
	--with-updater
	COMPACT=1
	DEBUG=0
	updater_package
```

Usefull options, check `./fbt -h` and [fbt.md](fbt.md) for more

```shell
DEBUG=1 # Enable debug build
COMPACT=1 # Optimize for size
DIST_SUFFIX=custom # Suffix for binaries in build output for dist targets
CUSTOM_FLIPPER_NAME=Flipper # Replaces OTP flipper name with custom string of 8 chars
UPDATE_SPLASH=update_default # Directory name with slideshow frames to render after installing update package
LOADER_AUTOSTART= # Application name to automatically run on Flipper boot
FIRMWARE_APPS="{
	'default': ['crypto_start', 'basic_services', 'basic_apps', 'updater_app', 'archive', 'custom_apps', 'passport', 'system_settings', 'about', 'basic_plugins', 'custom_games', 'wifi_plugins', 'external_apps', 'debug_apps'],
	'unit_tests': ['basic_services', 'updater_app', 'unit_tests'],
	'ext_apps': ['basic_services', 'external_apps']}"
	# Map of (configuration_name->application_list)
-c # Clean
```

Check `dist/` for build outputs.

Use **`flipper-z-{target}-full-{suffix}.dfu`** to flash your device.

If compilation fails, make sure all submodules are all initialized. Either clone with `--recursive` or use `git submodule update --init --recursive`.
