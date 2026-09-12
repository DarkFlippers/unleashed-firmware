# Debugging FAPs {#app_dev_debugging_faps}

In the [previous guide](#app_dev_your_first_app_in_c), you learned how to create and run basic applications written in C on the Flipper Zero. 

Now, let's explore real-time debugging. It allows launching an application, pausing execution at any point, stepping through code line by line, and monitoring variable states and execution flow. This approach helps identify and fix significantly more bugs than testing without real-time debugging.

> [!note]
> Using in-circuit debugging is optional — you can still debug applications by loading and running them over USB.

## Requirements

The Flipper Zero microcontroller’s SWD (Serial Wire Debug) interface is accessible through the device’s expansion port. You can connect a supported SWD probe to this interface to control program execution.

Therefore, you will need the following:

- Flipper Zero
- Windows, Linux or macOS computer
- One of the supported SWD probes

## Supported SWD probes

To use real-time debugging, you need a device that functions as a debug probe. This can be:

- Another Flipper Zero: simply run the [Dap LINK app](https://lab.flipper.net/apps/dap_link) to enable SWD probe functionality
- [WiFi Devboard for Flipper Zero](https://shop.flipperzero.one/products/wifi-devboard) with stock firmware
- CMSIS-DAP compatible probes (from [Raspberry Pi](https://www.raspberrypi.com/products/debug-probe/), [l-tek](https://www.l-tek.com/web-shop/cmsis-dap-debug-probe/), [makerdiary](https://makerdiary.com/products/pitaya-link), and others)
- [Blackmagic compatible probes](https://black-magic.org/hardware.html)
- Other compatible probes: [ST-LINK V2](https://www.st.com/en/development-tools/st-link-v2.html), [ST-LINK V3](https://www.st.com/en/development-tools/stlink-v3set.html), etc.

## Prerequisites

Your computer must have the necessary tools and libraries installed to compile and run applications on Flipper Zero. We recommend using a preconfigured package that automates this setup — **uFBT (micro Flipper Build Tool)**. Install it by following the instructions in the [uFBT repo](https://github.com/flipperdevices/flipperzero-ufbt).

Debugging must also be enabled on your Flipper Zero. To do this, set **Settings > System > Debug** to **ON**.

## Step-by-step guide

1\. **Connect the SWD probe to the Flipper Zero expansion port.** If you are using a probe not specially designed for Flipper Zero, you need to connect 3 pins: SWCLK (pin 10), SWDIO (pin 12), and GND (pin 8/11/18). If your SWD probe provides a UART interface, it is helpful to connect it to the Flipper Zero USART (pins 13, 14). This serial port outputs logs and information about critical program crashes when they occur, aiding in debugging.

2\. **Connect the SWD probe to your computer.** Most debuggers connect via USB cable, but the WiFi Devboard supports connection via USB or WiFi (see [official documentation](#dev_board) for details).
    
    
    \image html app_dev_flipperzero_app_wifi_devboard_debug.webp width=700

3\. **In Visual Studio Code, switch to debugging mode** by clicking the debug icon in the left menu or use the keyboard shortcut `Ctrl+Shift+D` on Windows/Linux or `Cmd+Shift+D` on macOS. Select the debugger model and press the **Play** button.

> [!note]
> Starting a debug session halts firmware execution. To resume, click the **Continue** button on the toolbar at the top of your VS Code window.

4\. **Use the debug control buttons** (reset, pause, run, step in, step out, and disconnect).

5\. **Set breakpoints** by clicking to the left of the code line. Code execution will automatically pause when it reaches a line with a breakpoint.

6\. **View the call stack tree.** When code execution is paused, you can view the call stack tree in the Call Stack window, showing the functions the processor has entered at the moment of the pause.

7\. **View variable values** using the Variables, Watch, or Live Watch panels.

Watch the demo below to see how to debug Flipper Zero FAPs in VS Code:

\htmlonly

<div style="display: flex; justify-content: center;">
  <video style="width: 100%; max-width: 700px; height: auto;" controls>
    <source src="https://cdn.flipperzero.one/app_dev_flipper_zero_debug_demo.mp4" type="video/mp4">
  </video>
</div>

\endhtmlonly