# About the JavaScript engine {#js_about_js_engine}

> Developing applications for Flipper Zero is now much more accessible with the introduction of JavaScript support.

Previously, building an app for Flipper Zero required C/C++ skills, setting up a development environment, and studying the code of existing applications and documentation. While embedded developers are very familiar with all of this, we wanted to make it easier for people from all backgrounds to create apps for Flipper Zero.

Flipper firmware now includes a built-in scripting engine that runs JavaScript, one of the most widely used programming languages. You can create script files, share them with others, and launch them directly from the **Apps/Scripts** menu on your Flipper Zero — no need for compiling on a PC.

JavaScript support is based on the [mJS scripting engine](https://github.com/cesanta/mjs). Originally designed for microcontrollers, mJS makes efficient use of system resources, requiring less than 50k of flash space and 2k of RAM. We've kept the core features of mJS and also added some useful improvements, such as support for compact binary arrays.

> [!note]
> mJS has some limitations compared to JavaScript engines built into modern browsers. For details on capabilities and limitations, refer to the [mJS documentation on GitHub](https://github.com/cesanta/mjs).

JavaScript apps can interact with Flipper Zero's resources, including its GUI, buttons, USB-HID device, GPIO, UART interfaces, and more. Let's go through the steps to create your first JavaScript app for Flipper Zero.

**Next step:** [Your first JavaScript app](#js_your_first_js_app)
