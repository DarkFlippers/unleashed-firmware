# Developing apps using JavaScript SDK {#js_developing_apps_using_js_sdk}

In the [previous guide](#js_your_first_js_app), we learned how to create and run a JavaScript app on Flipper Zero. However, when debugging a script, you often need to repeatedly modify the code and test it on the device. While you can use qFlipper for this, it involves a lot of repetitive steps. Fortunately, there's a more efficient alternative — the Flipper Zero JavaScript SDK, a set of tools that simplify app development in JavaScript.

Main features of the Flipper Zero JavaScript SDK:

* [Loading and running an app with a single command](#js_sdk_run_app)
* [Code completion](#js_sdk_code_completion)
* [JS code minifier (compressor)](#js_sdk_js_minifier)

In this guide, we'll install the JavaScript SDK and learn how to run JavaScript apps on Flipper Zero using it.

## How to get JavaScript SDK

The JavaScript SDK for Flipper Zero is distributed as an [NPM package](npmjs.com/package/\@flipperdevices/fz-sdk), so you can install it using a package manager like npm, pnpm, or yarn. You'll also need Node.js, a JavaScript runtime environment required for the NPM package manager to work.

> [!note]
> In this guide, we'll use **npm**, the default package manager for Node.js.

Follow these steps:

1. Install **Node.js + npm** on your PC. Check out this [official Downloads page](https://nodejs.org/en/download/package-manager), select your OS and preferences, and run the provided commands in your terminal.

2. Open a terminal in the folder where you want to store your project.

3. Run the `npx @flipperdevices/create-fz-app@latest` command to create a JavaScript app template and include the JavaScript SDK into it. This command will launch an interactive wizard. You'll need to specify the project name and choose a package manager (in our case, **npm**).

You'll now find a JavaScript app template in your project folder, alongside the JavaScript SDK package, all necessary dependencies and configs. The app code will be in the `index.ts` file.

Now, let's take a look at the main features of the Flipper Zero JavaScript SDK.

## Running your app {#js_sdk_run_app}

To run the application:

1. Connect your Flipper Zero to your PC via USB.

2. Open a terminal in your app's folder.

3. Run the `npm start` command to copy the JS file to Flipper Zero and run it.

\image html js_sdk_npm_start.jpg width=800

You'll see output messages from the `print()` function in the terminal.

## Updating your app {#js_sdk_update_app}

After making changes to your app's code, simply run `npm start` again. As long as your Flipper Zero is still connected, the updated app will launch, and the old `.js` file on Flipper Zero will be replaced with the new version.


## Other JavaScript SDK features

As you can see, it's quite easy to launch and update your app with a single command. Now let's explore two more important features of the Flipper Zero JavaScript SDK: **code completion** and **JS minifier**.


### Code completion {#js_sdk_code_completion}

Code completion helps speed up the development process by automatically suggesting code as you type, reducing the need to refer to documentation.

\image html js_sdk_code_completion.jpg width=800

> [!note]
> Code completion works in code editors and IDEs that support Language Server, for example,  [VS Code](https://code.visualstudio.com/).


### JS minifier {#js_sdk_js_minifier}

The JS minifier reduces the size of JavaScript files by removing unnecessary characters (like spaces, tabs and line breaks) and shortening variable names. This can make your scripts run a bit faster without changing their logic.

However, it has a drawback — it can make debugging harder, as error messages in minified files are harder to read in larger applications. For this reason, it's recommended to disable the JS minifier during debugging and it's disabled by default. To enable it, set the `minify` parameter to `true` in the `fz-sdk.config.json5` file in your app folder. This will minify your JavaScript app before loading it onto Flipper Zero.


## Differences with normal Flipper JavaScript

With the Flipper JavaScript SDK, you will be developing in **TypeScript**. This means that you get a better development experience, with more accurate code completion and warnings when variable types are incompatible, but it also means your code will be different from basic Flipper JS.

Some things to look out for:
- Importing modules:
  - Instead of `let module = require("module");`
  - You will use `import * as module from "@flipperdevices/fz-sdk/module";`
- Multiple source code files:
  - The Flipper JavaScript SDK does not yet support having multiple `.ts` files and importing them
  - You can use `load()`, but this will not benefit from TypeScript type checking
- Casting values:
  - Some Flipper JavaScript functions will return generic types
  - For example `eventLoop.subscribe()` will run your callback with a generic `Item` type
  - In some cases you might need to cast these values before using them, you can do this by:
  - Inline casting: `<string>item`
  - Declare with new type: `let text = item as string;`

When you upload the script to Flipper with `npm start`, it gets transpiled to normal JavaScript and optionally minified (see below). If you're looking to share your script with others, this is what you should give them to run.


## What's next?

You've learned how to run and debug simple JavaScript apps. But how can you access Flipper Zero's hardware from your JS code? For that, you'll need to use JS modules — which we'll cover in the next guide.

**Next step:** [Using JavaScript modules](#js_using_js_modules)
