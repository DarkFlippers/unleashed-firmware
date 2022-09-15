# FAP (Flipper Application Package)

[fbt](./fbt.md) has support for building applications as FAP files. FAP are essentially .elf executables with extra metadata and resources bundled in.

FAPs are built with `firmware_extapps` (or `plugin_dist`) **`fbt`** targets.

FAPs do not depend on being run on a specific firmware version. Compatibility is determined by the FAP's metadata, which includes the required [API version](#api-versioning).


## How to set up an application to be built as a FAP

FAPs are created and developed the same way as internal applications that are part of the firmware. 

To build your application as a FAP, just create a folder with your app's source code in `applications_user`, then write its code the way you'd do when creating a regular built-in application. Then configure its `application.fam` manifest — and set its *apptype* to FlipperAppType.EXTERNAL. See [Application Manifests](./AppManifests.md#application-definition) for more details. 

 * To build your application, run `./fbt firmware_{APPID}`, where APPID is your application's ID in its manifest.
 * To build your app, then upload it over USB & run it on Flipper, use `./fbt launch_app APPSRC=applications/path/to/app`. This command is configured in default [VSCode profile](../.vscode/ReadMe.md) as "Launch App on Flipper" build action (Ctrl+Shift+B menu).
 * To build all FAPs, run `./fbt plugin_dist`.


## Debugging FAPs

**`fbt`** includes a script for gdb-py to provide debugging support for FAPs, `debug/flipperapps.py`. It is loaded in default debugging configurations by **`fbt`** and stock VSCode configurations.

With it, you can debug FAPs as if they were a part of main firmware — inspect variables, set breakpoints, step through the code, etc.

### Setting up debugging environment

Debugging support script looks up debugging information in latest firmware build dir (`build/latest`). That directory is symlinked by fbt to the latest firmware configuration (Debug or Release) build dir, when you run `./fbt` for chosen configuration. See [fbt docs](./fbt.md#nb) for details.

So, to debug FAPs, do the following:
1. Build firmware with `./fbt`
2. Flash it with `./fbt flash`
3. [Build your FAP](#how-to-set-up-an-application-to-be-built-as-a-fap) and run it on Flipper.

After that, you can attach with `./fbt debug` or VSCode and use all debug features.

It is **important** that firmware and application build type (debug/release) match, and that matching firmware folder is linked as `build/latest`. Otherwise, debugging will not work. 


## How Flipper runs an application from SD card

Flipper's MCU cannot run code directly from external storage, so it needs to be copied to RAM first. That is done by the App Loader application, which is responsible for loading the FAP from SD card, verifying its integrity and compatibility, copying it to RAM and adjusting it for its new location. 

Since FAP has to be loaded to RAM to be executed, the amount of RAM available for allocations from heap is reduced compared to running the same app from flash, as a part of firmware. Note that the amount of occupied RAM is less than total FAP file size, since only code and data sections are allocated, while FAP file includes extra information only used at app load time.

Applications are built for a specific API version. It is a part of the hardware target's definition and contains a major and minor version number. Application loader checks if the application's major API version matches firmware's major API version.

App loader allocates memory for the application and copies it to RAM, processing relocations and providing concrete addresses for imported symbols using the [symbol table](#symbol-table). Then it starts the application.


## API Versioning

Not all parts of firmware are available for external applications. A subset of available functions and variables is defined in "api_symbols.csv" file, which is a part of firmware target definition in `firmware/targets/` directory. 

**`fbt`** uses semantic versioning for API versioning. Major version is incremented when there are breaking changes in the API, minor version is incremented when there are new features added. 

Breaking changes include:
- removal of a function or a global variable;
- changing the signature of a function.

API versioning is mostly automated by **`fbt`**. When rebuilding the firmware, **`fbt`** checks if there are any changes in the API exposed by headers gathered from `SDK_HEADERS`. If there are, it stops the build, adjusts the API version and asks the user to go through the changes in .csv file. New entries are marked with "`?`" mark, and the user is supposed to change the mark to "`+`" for the entry to be exposed for FAPs, "`-`" for it to be unavailable.

**`fbt`** will not allow building a firmware until all "`?`" entries are changed to "`+`" or "`-`".

**NB:** **`fbt`** automatically manages the API version. The only case where manually incrementing the major API version is allowed (and required) is when existing "`+`" entries are to be changed to "`-`". 

### Symbol Table

The symbol table is a list of symbols exported by firmware and available for external applications. It is generated by **`fbt`** from the API symbols file and is used by the App Loader to resolve addresses of imported symbols. It is build as a part of the `fap_loader` application.

**`fbt`** also checks if all imported symbols are present in the symbol table. If there are any missing symbols, it will issue a warning listing them. Such application won't be able to run on the device until all requires symbols are provided in the symbol table.
