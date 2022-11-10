# Flipper Application Manifests (.fam)

All components of Flipper Zero firmware — services, user applications, system settings — are developed independently. Each component has a build system manifest file, named `application.fam`, which defines basic properties of that component and its relations to other parts of the system.

When building firmware, **`fbt`** collects all application manifests and processes their dependencies. Then it builds only those components that are referenced in the current build configuration. See [fbt docs](./fbt.md#firmware-application-set) for details on build configurations.

## Application definition

Properties of a firmware component are declared in a form of a Python code snippet, forming a call to App() function with various parameters. 

Only 2 parameters are mandatory: ***appid*** and ***apptype***, others are optional and may only be meaningful for certain application types.

### Parameters

* **appid**: string, application id within the build system. Used for specifying which applications to include in build configuration and to resolve dependencies and conflicts.

* **apptype**: member of FlipperAppType.* enumeration. Valid values are:

| Enum member  | Firmware component type  |
|--------------|--------------------------|
| SERVICE      | System service, created at early startup  |
| SYSTEM       | Application not being shown in any menus. Can be started by other apps or from CLI  |
| APP          | Regular application for main menu |
| PLUGIN       | Application to be built as a part of firmware an to be placed in Plugins menu |
| DEBUG        | Application only visible in Debug menu with debug mode enabled |
| ARCHIVE      | One and only Archive app |
| SETTINGS     | Application to be placed in System settings menu |
| STARTUP      | Callback function to run at system startup. Does not define a separate app |
| EXTERNAL     | Application to be built as .fap plugin |
| METAPACKAGE  | Does not define any code to be run, used for declaring dependencies and application bundles |

* **name**: Name that is displayed in menus.
* **entry_point**: C function to be used as application's entry point. Note that C++ function names are mangled, so you need to wrap them in `extern "C"` in order to use them as entry points.
* **flags**: Internal flags for system apps. Do not use.
* **cdefines**: C preprocessor definitions to declare globally for other apps when current application is included in active build configuration.
* **requires**: List of application IDs to also include in build configuration, when current application is referenced in list of applications to build.
* **conflicts**: List of application IDs that current application conflicts with. If any of them is found in constructed application list, **`fbt`** will abort firmware build process.
* **provides**: Functionally identical to ***requires*** field.
* **stack_size**: Stack size, in bytes, to allocate for application on its startup. Note that allocating a stack that is too small for an app to run will cause system crash due to stack overflow, and allocating too much stack space will reduce usable heap memory size for apps to process data. *Note: you can use `ps` and `free` CLI commands to profile your app's memory usage.*
* **icon**: Animated icon name from built-in assets to be used when building app as a part of firmware.
* **order**: Order of an application within its group when sorting entries in it. The lower the order is, the closer to the start of the list the item is placed. *Used for ordering startup hooks and menu entries.* 
* **sdk_headers**: List of C header files from this app's code to include in API definitions for external applications.
* **targets**: list of strings, target names, which this application is compatible with. If not specified, application is built for all targets. Default value is `["all"]`.


#### Parameters for external applications

The following parameters are used only for [FAPs](./AppsOnSDCard.md):

* **sources**: list of strings, file name masks, used for gathering sources within app folder. Default value of `["*.c*"]` includes C and C++ source files. Application cannot use `"lib"` folder for their own source code, as it is reserved for **fap_private_libs**.
* **fap_version**: tuple, 2 numbers in form of (x,y): application version to be embedded within .fap file. Default value is (0,1), meaning version "0.1".
* **fap_icon**: name of a .png file, 1-bit color depth, 10x10px, to be embedded within .fap file.
* **fap_libs**: list of extra libraries to link application against. Provides access to extra functions that are not exported as a part of main firmware at expense of increased .fap file size and RAM consumption.
* **fap_category**: string, may be empty. App subcategory, also works as path of FAP within apps folder in the file system.
* **fap_description**: string, may be empty. Short application description.
* **fap_author**: string, may be empty. Application's author.
* **fap_weburl**: string, may be empty. Application's homepage.
* **fap_icon_assets**: string. If present, defines a folder name to be used for gathering image assets for this application. These images will be preprocessed and built alongside the application. See [FAP assets](./AppsOnSDCard.md#fap-assets) for details.
* **fap_extbuild**: provides support for parts of application sources to be built by external tools. Contains a list of `ExtFile(path="file name", command="shell command")` definitions. **`fbt`** will run the specified command for each file in the list.
Note that commands are executed at the firmware root folder's root, and all intermediate files must be placed in a application's temporary build folder. For that, you can use pattern expansion by **`fbt`**: `${FAP_WORK_DIR}` will be replaced with the path to the application's temporary build folder, and `${FAP_SRC_DIR}` will be replaced with the path to the application's source folder. You can also use other variables defined internally by **`fbt`**. 

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

* **fap_private_libs**: list of additional libraries that are distributed as sources alongside the application. These libraries will be built as a part of the application build process. 
Library sources must be placed in a subfolder of "`lib`" folder within the application's source folder.
Each library is defined as a call to `Lib()` function, accepting the following parameters:

    - **name**: name of library's folder. Required.
    - **fap_include_paths**: list of library's relative paths to add to parent fap's include path list. Default value is `["."]` meaning  library's source root.
    - **sources**: list of filename masks to be used for gathering include files for this library. Paths are relative to library's source root. Default value is `["*.c*"]`.
    - **cflags**: list of additional compiler flags to be used for building this library. Default value is `[]`.
    - **cdefines**: list of additional preprocessor definitions to be used for building this library. Default value is `[]`.
    - **cincludes**: list of additional include paths to be used for building this library. Paths are relative to application's root. Can be used for providing external search paths for this library's code - for configuration headers. Default value is `[]`.

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

For that snippet, **`fbt`** will build 2 libraries: one from sources in `lib/mbedtls` folder, and another from sources in `lib/loclass` folder. For `mbedtls` library, **`fbt`** will add `lib/mbedtls/include` to the list of include paths for the application and compile only the files specified in `sources` list. Additionally, **`fbt`** will enable `MBEDTLS_ERROR_C` preprocessor definition for `mbedtls` sources. 
For `loclass` library, **`fbt`** will add `lib/loclass` to the list of include paths for the application and build all sources in that folder. Also **`fbt`** will disable treating compiler warnings as errors for `loclass` library specifically - that can be useful when compiling large 3rd-party codebases.

Both libraries will be linked into the application.


## .fam file contents

.fam file contains one or more Application definitions. For example, here's a part of `applications/service/bt/application.fam`:

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

For more examples, see .fam files from various firmware parts.
