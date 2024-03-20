# Application icons {#example_app_images}

## Source code

Source code for this example can be found [here](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/examples/example_images).

## General principle

To use icons, do the following:

* Add a line to the application manifest: `fap_icon_assets="folder"`, where `folder` points to the folder where your icons are located
* Add `#include "application_id_icons.h"` to the application code, where `application_id` is the appid from the manifest
* Every icon in the folder will be available as a `I_icon_name` variable, where `icon_name` is the name of the icon file without the extension

## Example

We have an application with the following manifest:

```
App(
    appid="example_images",
    ...
    fap_icon_assets="images",
)
```

So the icons are in the `images` folder and will be available in the generated `example_images_icons.h` file.

The example code is located in `example_images_main.c` and contains the following line:

```
#include "example_images_icons.h"
```

Image `dolphin_71x25.png` is available as `I_dolphin_71x25`.
