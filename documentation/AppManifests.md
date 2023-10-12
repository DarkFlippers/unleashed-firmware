# Flipper Application Manifests (.fam)

All components of Flipper Zero firmware — services, user applications, and system settings — are developed independently. Each component has a build system manifest file named `application.fam`, which defines the basic properties of that component and its relations to other parts of the system.

When building firmware, **`fbt`** collects all application manifests and processes their dependencies. Then it builds only those components referenced in the current build configuration. See [FBT docs](./fbt.md#firmware-application-set) for details on build configurations.

## Application definition

A firmware component's properties are declared in a Python code snippet, forming a call to the `App()` function with various parameters.

Only two parameters are mandatory: **_appid_** and **_apptype_**. Others are optional and may only be meaningful for certain application types.

### Parameters

- **appid**: string, application ID within the build system. It is used to specify which applications to include in the build configuration and resolve dependencies and conflicts.

- **apptype**: member of FlipperAppType.\* enumeration. Valid values are:

| Enum member | Firmware component type                                                                     |
| ----------- | ------------------------------------------------------------------------------------------- |
| SERVICE     | System service, created at early startup                                                    |
| SYSTEM      | Application is not being shown in any menus. It can be started by other apps or from CLI    |
| APP         | Regular application for the main menu                                                       |
| PLUGIN      | Application to be built as a part of the firmware and to be placed in the Plugins menu      |
| DEBUG       | Application only visible in Debug menu with debug mode enabled                              |
| ARCHIVE     | One and only Archive app                                                                    |
| SETTINGS    | Application to be placed in the system settings menu                                        |
| STARTUP     | Callback function to run at system startup. Does not define a separate app                  |
| EXTERNAL    | Application to be built as `.fap` plugin                                                    |
| METAPACKAGE | Does not define any code to be run, used for declaring dependencies and application bundles |

- **name**: name displayed in menus.
- **entry_point**: C function to be used as the application's entry point. Note that C++ function names are mangled, so you need to wrap them in `extern "C"` to use them as entry points.
- **flags**: internal flags for system apps. Do not use.
- **cdefines**: C preprocessor definitions to declare globally for other apps when the current application is included in the active build configuration.
- **requires**: list of application IDs to include in the build configuration when the current application is referenced in the list of applications to build.
- **conflicts**: list of application IDs with which the current application conflicts. If any of them is found in the constructed application list, **`fbt`** will abort the firmware build process.
- **provides**: functionally identical to **_requires_** field.
- **stack_size**: stack size in bytes to allocate for an application on its startup. Note that allocating a stack too small for an app to run will cause a system crash due to stack overflow, and allocating too much stack space will reduce usable heap memory size for apps to process data. _Note: you can use `ps` and `free` CLI commands to profile your app's memory usage._
- **icon**: animated icon name from built-in assets to be used when building the app as a part of the firmware.
- **order**: order of an application within its group when sorting entries in it. The lower the order is, the closer to the start of the list the item is placed. _Used for ordering startup hooks and menu entries._
- **sdk_headers**: list of C header files from this app's code to include in API definitions for external applications.
- **targets**: list of strings and target names with which this application is compatible. If not specified, the application is built for all targets. The default value is `["all"]`.

#### Parameters for external applications

The following parameters are used only for [FAPs](./AppsOnSDCard.md):

- **sources**: list of strings, file name masks used for gathering sources within the app folder. The default value of `["*.c*"]` includes C and C++ source files. Applications cannot use the `"lib"` folder for their own source code, as it is reserved for **fap_private_libs**.
- **fap_version**: string, application version. The default value is "0.1". You can also use a tuple of 2 numbers in the form of (x,y) to specify the version. It is also possible to add more dot-separated parts to the version, like patch number, but only major and minor version numbers are stored in the built .fap.
- **fap_icon**: name of a `.png` file, 1-bit color depth, 10x10px, to be embedded within `.fap` file.
- **fap_libs**: list of extra libraries to link the application against. Provides access to extra functions that are not exported as a part of main firmware at the expense of increased `.fap` file size and RAM consumption.
- **fap_category**: string, may be empty. App subcategory, also determines the path of the FAP within the apps folder in the file system.
- **fap_description**: string, may be empty. Short application description.
- **fap_author**: string, may be empty. Application's author.
- **fap_weburl**: string, may be empty. Application's homepage.
- **fap_icon_assets**: string. If present, it defines a folder name to be used for gathering image assets for this application. These images will be preprocessed and built alongside the application. See [FAP assets](./AppsOnSDCard.md#fap-assets) for details.
- **fap_extbuild**: provides support for parts of application sources to be built by external tools. Contains a list of `ExtFile(path="file name", command="shell command")` definitions. **`fbt`** will run the specified command for each file in the list.
- **fal_embedded**: boolean, default `False`. Applies only to PLUGIN type. If `True`, the plugin will be embedded into host application's .fap file as a resource and extracted to `apps_assets/APPID` folder on its start. This allows plugins to be distributed as a part of the host application.

Note that commands are executed at the firmware root folder, and all intermediate files must be placed in an application's temporary build folder. For that, you can use pattern expansion by **`fbt`**: `${FAP_WORK_DIR}` will be replaced with the path to the application's temporary build folder, and `${FAP_SRC_DIR}` will be replaced with the path to the application's source folder. You can also use other variables defined internally by **`fbt`**.

Example for building an app from Rust sources:

```python
    sources=["target/thumbv7em-none-eabihf/release/libhello_rust.a"],
    fap_extbuild=(
        ExtFile(
            path="${FAP_WORK_DIR}/target/thumbv7em-none-eabihf/release/libhello_rust.a",
            command="cargo build --release --verbose --target thumbv7em-none-eabihf --target-dir ${FAP_WORK_DIR}/target --manifest-path ${FAP_SRC_DIR}/Cargo.toml",
        ),
    ),
```

- **fap_private_libs**: list of additional libraries distributed as sources alongside the application. These libraries will be built as a part of the application build process.
  Library sources must be placed in a subfolder of the `lib` folder within the application's source folder.
  Each library is defined as a call to the `Lib()` function, accepting the following parameters:

    - **name**: name of the library's folder. Required.
    - **fap_include_paths**: list of the library's relative paths to add to the parent fap's include path list. The default value is `["."]`, meaning the library's source root.
    - **sources**: list of filename masks to be used for gathering include files for this library. Paths are relative to the library's source root. The default value is `["*.c*"]`.
    - **cflags**: list of additional compiler flags to be used for building this library. The default value is `[]`.
    - **cdefines**: list of additional preprocessor definitions to be used for building this library. The default value is `[]`.
    - **cincludes**: list of additional include paths to be used for building this library. Paths are relative to the application's root. This can be used for providing external search paths for this library's code — for configuration headers. The default value is `[]`.

Example for building an app with a private library:

```python
    fap_private_libs=[
            Lib(
                name="mbedtls",
                fap_include_paths=["include"],
                sources=[
                    "library/des.c",
                    "library/sha1.c",
                    "library/platform_util.c",
                ],
                cdefines=["MBEDTLS_ERROR_C"],
            ),
            Lib(
                name="loclass",
                cflags=["-Wno-error"],
            ),
        ],
```

For that snippet, **`fbt`** will build 2 libraries: one from sources in `lib/mbedtls` folder and another from sources in the `lib/loclass` folder. For the `mbedtls` library, **`fbt`** will add `lib/mbedtls/include` to the list of include paths for the application and compile only the files specified in the `sources` list. Additionally, **`fbt`** will enable `MBEDTLS_ERROR_C` preprocessor definition for `mbedtls` sources.
For the `loclass` library, **`fbt`** will add `lib/loclass` to the list of the include paths for the application and build all sources in that folder. Also, **`fbt`** will disable treating compiler warnings as errors for the `loclass` library, which can be useful when compiling large 3rd-party codebases.

Both libraries will be linked with the application.

## `.fam` file contents

The `.fam` file contains one or more application definitions. For example, here's a part of `applications/service/bt/application.fam`:

```python
App(
    appid="bt_start",
    apptype=FlipperAppType.STARTUP,
    entry_point="bt_on_system_start",
    order=70,
)

App(
    appid="bt_settings",
    name="Bluetooth",
    apptype=FlipperAppType.SETTINGS,
    entry_point="bt_settings_app",
    stack_size=1 * 1024,
    requires=[
        "bt",
        "gui",
    ],
    order=10,
)
```

For more examples, see `.fam` files from various firmware parts.
