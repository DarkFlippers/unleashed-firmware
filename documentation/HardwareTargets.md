## What a Firmware Target is {#hardware_targets}

Flipper's firmware is modular and supports different hardware configurations in a common code base. It encapsulates hardware-specific differences in `furi_hal`, board initialization code, linker files, SDK data and other information in a _target definition_.

Target-specific files are placed in a single sub-folder in `targets`. It must contain a target definition file, `target.json`, and may contain other files if they are referenced by current target's definition. By default, `fbt` gathers all source files in target folder, unless they are explicitly excluded.

Targets can inherit most code parts from other targets, to reduce common code duplication.


## Target Definition File

A target definition file, `target.json`, is a JSON file that can contain the following fields:

* `include_paths`: list of strings, folder paths relative to current target folder to add to global C/C++ header path lookup list.
* `sdk_header_paths`: list of strings, folder paths relative to current target folder to gather headers from for including in SDK.
* `startup_script`: filename of a startup script, performing initial hardware initialization.
* `linker_script_flash`: filename of a linker script for creating the main firmware image.
* `linker_script_ram`: filename of a linker script to use in "updater" build configuration.
* `linker_script_app`: filename of a linker script to use for linking .fap files.
* `sdk_symbols`: filename of a .csv file containing current SDK configuration for this target.
* `linker_dependencies`: list of libraries to link the firmware with. Note that those not in the list won't be built by `fbt`. Also several link passes might be needed, in such case you may need to specify same library name twice.
* `inherit`: string, specifies hardware target to borrow main configuration from. Current configuration may specify additional values for parameters that are lists of strings, or override values that are not lists.
* `excluded_sources`: list of filenames from the inherited configuration(s) NOT to be built.
* `excluded_headers`: list of headers from the inherited configuration(s) NOT to be included in generated SDK.
* `excluded_modules`: list of strings specifying fbt library (module) names to exclude from being used to configure build environment.


## Apps & Hardware

Not all apps are available on different hardware targets. 

* For apps built into the firmware, you have to specify a compatible app set using `FIRMWARE_APP_SET=...` fbt option. See [fbt docs](./fbt.md) for details on build configurations.

* For apps built as external FAPs, you have to explicitly specify compatible targets in the app's manifest, `application.fam`. For example, to limit the app to a single target, add `targets=["f7"],` to the manifest. It won't be built for other targets.

For details on app manifests, check out [their docs page](./AppManifests.md).


## Building Firmware for a Specific Target

You have to specify TARGET_HW (and, optionally, FIRMWARE_APP_SET) for `fbt` to build firmware for a non-default target. For example, building and flashing debug firmware for f18 can be done with

    ./fbt TARGET_HW=18 flash_usb_full

