# Your first app in C {#app_dev_your_first_app_in_c}

In this tutorial, you'll learn how to set up your development environment and generate an app template using uFBT. You'll also learn how to create simple apps and implement basic functionality using the GUI. Lastly, we'll explain how to build and run your app on Flipper Zero.

## 1. Environment setup

To develop applications for Flipper Zero, you'll need the device itself and a USB cable to connect it to your computer. You'll also need to install the following software on your Windows/MacOS/Linux system:

- [uFBT](https://github.com/flipperdevices/flipperzero-ufbt) (”micro Flipper Build Tool”) — a version of [FBT](#fbt) that supports everything you need for app development.
- [Visual Studio Code](https://code.visualstudio.com/) — recommended IDE for work with uFBT. You may use other IDEs, but they will not be able to integrate directly with uFBT without additional manual configuration.

## 2. Generate an app template

1. Create a folder for your new app, name it as you want, for example `test_app`.
2. In this folder, open Terminal and run: `ufbt create APPID=test_app`. uFBT will generate the file structure required for the app.
3. Run `ufbt vscode_dist`. This will configure VS Code to allow building and debugging your Flipper Zero application directly from the IDE.

> [!warning]
> `APPID` can only contain letters `a-z`, numbers `0-9`, and the symbol `_`.

In the app folder, the file structure will looks as follows:

```
test_app/
├── application.fam
├── test_app.c
├── test_app.png
├── .github/
│   └── workflows/
|       └── build.yml
└── images/
    └── .gitkeep
```

> Here is a breakdown of the structure:
> - `images/.gitkeep` — a placeholder file used to ensure that the `images/` directory is tracked by Git, even if it's empty.
> - `images/` — the folder for storing images.
> - `.github/workflows/build.yml` — GitHub Actions configuration. [Learn more](https://github.com/marketplace/actions/build-flipper-application-package-fap).
> - `test_app.png` — the app icon.
> - `test_app.c` — the app main file, containing the app entry point.
> - `application.fam` — the app manifest that uFBT reads to find out how to build your app. [Learn more about FAM](#app_manifests).

## 3. Template app code

```c
#include <furi.h>
#include <test_app_icons.h>

int32_t test_app_app(void* p) {
    UNUSED(p);
    FURI_LOG_I("TEST", "Hello world");
    FURI_LOG_I("TEST", "I'm test_app!");

    return 0;
}
```

> Here is a breakdown:
> - `#include <furi.h>` — Includes the main header file for FURI, a core component of the Flipper Zero firmware.
> - `#include <test_app_icons.h>` —  Needed if the app uses images. uFBT automatically converts PNG files into Flipper’s internal image format and generates this file. All you need to do is add your PNG files to the `images/` directory.
> - `int32_t` — You'd typically use this return type for the app's main function.
> - `UNUSED(p);` — A macro that silences compiler warnings about the unused `p` parameter.
> - `FURI_LOG_I` — A function that logs an `info` level message to the Flipper console. You can see these messages by opening the console (`ufbt cli`), then running the `log info` command.
> - `return 0;` — A return value of `0` indicates successful program termination (convention in C).

## 4. Add a graphical UI (GUI)

Open the application folder in Visual Studio Code (**File → Open Folder...**).

> [!note]
> It is recommended to install all suggested VS Code extensions when opening the folder.

Let's make the app a bit more advanced and display two lines on the screen: `Hello world` and `I'm test_app!`. Open the `test_app.c` file and replace its contents with the code below:

```c
#include <furi.h>
#include <dialogs/dialogs.h>
#include <test_app_icons.h>

int32_t test_app_app(void* p) {
    UNUSED(p);
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Hello world", 64, 20, AlignCenter, AlignTop);
    dialog_message_set_text(message, "I'm test_app!", 64, 32, AlignCenter, AlignTop);
    dialog_message_show(dialogs, message);
    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);

    return 0;
}
```

> This code utilizes the `Dialogs` GUI service that provides a system API. Learn more: [GUI in Apps](#app_dev_gui).

## 5. Build and run the App

1. Connect your Flipper Zero to your computer using a USB cable.
2. Run the `ufbt launch` command. It will build, upload, and start your app on Flipper.

Alternatively, you can manually build and transfer your app to Flipper Zero:

1. Build the app using the keyboard shortcut `Shift+Ctrl+B` on Win/Linux or `Shift+Cmd+B` on Mac. Your app with all the resources will be built into a self-contained FAP file.
2. In qFlipper, go to **File Manager → SD Card → apps → Examples** (create the folder if it doesn't exist), then simply drag and drop your built app into the folder.
3. Run your app. On your Flipper Zero, go to **Apps** → **Examples** and select your app.

> [!note]
> To run your application on Flipper Zero, the firmware SDK version on the device must match the SDK version used by uFBT.
> If you get an SDK version mismatch error when launching your app, flash your Flipper Zero using the command: `ufbt flash_usb`

If you used the log output method (`FURI_LOG_I`), you can see the logs by opening the console (`ufbt cli`), then running the `log` command. If you changed the default log level, use `log info` instead.

## What's next?

Congrats on completing your first app!

If you feel your C programming skills are a bit rusty, we recommend referring to *The C Programming Language* by B. Kernighan and D. Ritchie, along with other resources listed on the [main page](#applications) of this section.

**Next step:** [Debugging FAPs](#app_dev_debugging_faps)
