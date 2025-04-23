# Your first JavaScript app {#js_your_first_js_app}

In this guide, we'll create a simple script that outputs ordinal numbers with a delay and learn how to run it on Flipper Zero in different ways. All you need is your Flipper Zero, a PC, and a USB cable.

## Step 1. Create the script file

Create a new text file `first_app.js`. Paste the code below into it and save the file:

\code{.js}
print("start");
delay(1000);
print("1");
delay(500);
print("2");
delay(500);
print("3");
delay(500);
print("end");
\endcode

What the code does:
* Outputs the text **start**, waits 1 second
* Outputs the numbers **1**, **2** and **3**, with a 0.5-second pause after each number
* Outputs the text **end**

The `print()` function is used to output text. The string to be output is in the brackets. This is a built-in function, so you don't need to include additional JS modules. You can use this function anywhere in your application.

Another built-in function, `delay()`, implements delay. The delay time in milliseconds is given in the brackets. Since 1000 milliseconds equals 1 second, a 1-second delay is written as 1000, and a 0.5-second delay as 500.

> [!note]
> Find the list of built-in functions in [Built-in functions](#js_builtin).

## Step 2. Copy the file to Flipper Zero

To copy the JavaScript file to Flipper Zero, follow these steps:
1. Connect your Flipper Zero to your PC via USB.
2. Open the **qFlipper** application.
3. Go to the **File manager** tab and open the path `SD Card/apps/Scripts/`.
4. Drag and drop the file into the qFlipper window.

> [!note]
> To learn more about qFlipper, visit the [dedicated section in our user documentation](https://docs.flipper.net/qflipper).

Your script is now ready to run on Flipper Zero.

## Step 3. Run your script

You can launch your app in two ways:

* From the Flipper Zero menu
* Remotely from your PC using the CLI (command-line interface)

Let's explore them both.

### How to run a script from Flipper Zero's menu

1. Go to **Apps → Scripts** in your Flipper Zero's menu. Here, you'll see a list of scripts located in the `SD Card/apps/Scripts/` folder.
2. Select the script you want to run.
3. Press the **OK** button to run the script.

\image html js_first_app_on_fz.jpg width=500

The Flipper Zero screen will display the strings with the specified delay, as defined by the `print()` and `delay()` functions.

### How to run script using CLI

The command-line interface (CLI) is a text-based interface that lets you control your Flipper Zero from your computer, including running scripts. Running JavaScript apps via CLI is useful for debugging, as it lets you write and test code remotely, without switching between your PC and the device.

To run the script via CLI:

1. Connect your Flipper Zero to your PC via USB.
2. Access the CLI using one of the [recommended methods](https://docs.flipper.net/development/cli#HfXTy).
3. Enter the `js path` command, replacing `path` with the path to the script file on your Flipper Zero:

\code{.sh}
js /ext/apps/Scripts/first_app.js
\endcode

\image html js_first_app_on_cli.jpg width=700

As you can see, unlike running JavaScript apps from the Flipper Zero UI, all output from the `print()` function is sent to the CLI, not the device screen.

**Next step:** [Developing apps using JavaScript SDK](#js_developing_apps_using_js_sdk)
