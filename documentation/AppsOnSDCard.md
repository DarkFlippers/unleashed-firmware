# FAP (Flipper Application Package)

[fbt](./fbt.md) supports building applications as FAP files. FAPs are essentially `.elf` executables with extra metadata and resources bundled in.

FAPs are built with the `faps` target. They can also be deployed to the `dist` folder with the `fap_dist` target.

FAPs do not depend on being run on a specific firmware version. Compatibility is determined by the FAP's metadata, which includes the required [API version](#api-versioning).

## How to set up an application to be built as a FAP

FAPs are created and developed the same way as internal applications that are part of the firmware.

To build your application as a FAP, create a folder with your app's source code in `applications_user`, then write its code the way you'd do when creating a regular built-in application. Then configure its `application.fam` manifest, and set its _apptype_ to FlipperAppType.EXTERNAL. See [Application Manifests](./AppManifests.md#application-definition) for more details.

- To build your application, run `./fbt fap_{APPID}`, where APPID is your application's ID in its manifest.
- To build your app and upload it over USB to run on Flipper, use `./fbt launch APPSRC=applications_user/path/to/app`. This command is configured in the default [VS Code profile](../.vscode/ReadMe.md) as a "Launch App on Flipper" build action (Ctrl+Shift+B menu).
- To build an app without uploading it to Flipper, use `./fbt build APPSRC=applications_user/path/to/app`. This command is also availabe in VSCode configuration as "Build App".
- To build all FAPs, run `./fbt faps` or `./fbt fap_dist`.

## FAP assets

FAPs can include static and animated images as private assets. They will be automatically compiled alongside application sources and can be referenced the same way as assets from the main firmware.

To use that feature, put your images in a subfolder inside your application's folder, then reference that folder in your application's manifest in the `fap_icon_assets` field. See [Application Manifests](./AppManifests.md#application-definition) for more details.

To use these assets in your application, put `#include "{APPID}_icons.h"` in your application's source code, where `{APPID}` is the `appid` value field from your application's manifest. Then you can use all icons from your application's assets the same way as if they were a part of `assets_icons.h` of the main firmware.

Images and animated icons should follow the same [naming convention](../assets/ReadMe.md#asset-naming-rules) as those from the main firmware.

## Debugging FAPs

**`fbt`** includes a script for gdb-py to provide debugging support for FAPs, `debug/flipperapps.py`. It is loaded in default debugging configurations by **`fbt`** and stock VS Code configurations.

With it, you can debug FAPs as if they were a part of the main firmware — inspect variables, set breakpoints, step through the code, etc.

If debugging session is active, firmware will trigger a breakpoint after loading a FAP it into memory, but before running any code from it. This allows you to set breakpoints in the FAP's code. Note that any breakpoints set before the FAP is loaded may need re-setting after the FAP is actually loaded, since before loading it debugger cannot know the exact address of the FAP's code.

### Setting up debugging environment

The debugging support script looks up debugging information in the latest firmware build directory (`build/latest`). That directory is symlinked by `fbt` to the latest firmware configuration (Debug or Release) build directory when you run `./fbt` for the chosen configuration. See [fbt docs](./fbt.md#nb) for details.

To debug FAPs, do the following:

1. Build firmware with `./fbt`
2. Flash it with `./fbt flash`
3. [Build your FAP](#how-to-set-up-an-application-to-be-built-as-a-fap) and run it on Flipper

After that, you can attach with `./fbt debug` or VS Code and use all debug features.

It is **important** that firmware and application build type (debug/release) match and that the matching firmware folder is linked as `build/latest`. Otherwise, debugging will not work.

## How Flipper runs an application from an SD card

Flipper's MCU cannot run code directly from external storage, so it needs to be copied to RAM first. That is done by the App Loader application responsible for loading the FAP from the SD card, verifying its integrity and compatibility, copying it to RAM, and adjusting it for its new location.

Since FAP has to be loaded to RAM to be executed, the amount of RAM available for allocations from heap is reduced compared to running the same app from flash, as a part of the firmware. Note that the amount of occupied RAM is less than the total FAP file size since only code and data sections are allocated, while the FAP file includes extra information only used at app load time.

Applications are built for a specific API version. It is a part of the hardware target's definition and contains a major and minor version number. The App Loader checks if the application's major API version matches the firmware's major API version.

The App Loader allocates memory for the application and copies it to RAM, processing relocations and providing concrete addresses for imported symbols using the [symbol table](#symbol-table). Then it starts the application.

## API versioning

Not all parts of firmware are available for external applications. A subset of available functions and variables is defined in the "api_symbols.csv" file, which is a part of the firmware target definition in the `firmware/targets/` directory.

**`fbt`** uses semantic versioning for the API. The major version is incremented when there are breaking changes in the API. The minor version is incremented when new features are added.

Breaking changes include:

- Removing a function or a global variable
- Changing the signature of a function

API versioning is mostly automated by **`fbt`**. When rebuilding the firmware, **`fbt`** checks if there are any changes in the API exposed by headers gathered from `SDK_HEADERS`. If so, it stops the build, adjusts the API version, and asks the user to go through the changes in the `.csv` file. New entries are marked with a "`?`" mark, and the user is supposed to change the mark to "`+`" for the entry to be exposed for FAPs, or to "`-`" for it to be unavailable.

**`fbt`** will not allow building a firmware until all "`?`" entries are changed to "`+`" or "`-`".

**NB:** **`fbt`** automatically manages the API version. The only case where manually incrementing the major API version is allowed (and required) is when existing "`+`" entries are to be changed to "`-`".

### Symbol table

The symbol table is a list of symbols exported by firmware and available for external applications. It is generated by **`fbt`** from the API symbols file and is used by the App Loader to resolve addresses of imported symbols. It is build as a part of the `fap_loader` application.

**`fbt`** also checks if all imported symbols are present in the symbol table. If there are any missing symbols, it will issue a warning listing them. The application won't be able to run on the device until all required symbols are provided in the symbol table.
