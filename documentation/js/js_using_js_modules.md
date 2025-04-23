# Using JavaScript modules {#js_using_js_modules}

In the previous guides, we learned how to write a basic JavaScript app using [built-in functions](#js_builtin). However, the set of built-in functions is limited, so when developing your JS apps, you'll likely want to use external JS modules. These modules offer a wide range of functions (methods) for various tasks.

For example:
* The `serial` module enables transmitting and receiving data via a serial interface
* The `badusb` module enables USB keyboard emulation and sending key press events via USB
* The `math` module provides mathematical functions

JS modules are written in C/C++, making them fast and efficient. They come with Flipper Zero firmware and are stored on the microSD card in compiled form as **FAL (Flipper Application File)** files.

> [!note]
> You can find the implementation of all supported JS modules in the [Flipper Zero firmware repository](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/applications/system/js_app/modules). Also, check out the [docs for JS modules](#js_modules) for more details.

## How to use JS modules in your app

Before using any of the JS module methods, you **must** import the module using the `require()` function. This loads the module into RAM, allowing you to access its methods.

To save RAM and improve performance, avoid loading modules you don't plan to use. Also, all loaded modules will be automatically unloaded from RAM after the app execution ends.

To load a module, call the `require()` function with the module name in quotes. For example, to load the `notification` module, write this:

\code{.js}
let notify = require("notification");
\endcode

Now you can call methods of the `notification` module using the `notify` variable to access them:

\code{.js}
let notify = require("notification");

notify.success();
print("success notification");
\endcode

## What's next?

Congratulations, you've completed the **Getting Started** section of our JS docs. You've learned how to run and debug JS apps, and how to use JS modules. Now, we invite you to check out the [main JavaScript page](#js) where you'll find:

* JavaScript app examples
* Documentation on JS modules
* Additional resources related to JavaScript on Flipper Zero
