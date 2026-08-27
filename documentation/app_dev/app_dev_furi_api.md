# Using Furi API {#app_dev_furi_api}

To fully leverage Flipper Zero's capabilities — such as the screen, speaker, radio transceiver, USB, or NFC — you need to understand how to interact with its hardware components. For this purpose, Flipper Zero includes Furi API, a set of libraries designed to provide developers with a simple interface for hardware interaction and common system tasks.

This page provides an overview of Flipper Zero's hardware and software architecture, laying the foundation for a clearer understanding of the system's internals as you develop your applications.

## Hardware

Inside Flipper Zero is an STM32WB55 microcontroller (MCU), which features two cores, RAM, flash memory, and several dozen peripheral blocks (timers, RTC, BLE, GPIO, etc.). The operation of these blocks are detailed in the MCU's Reference Manual.

In addition to the MCU, Flipper Zero includes several peripheral (external to the MCU) chips. To learn how work with these components, refer to their respective datasheets and documentation.

Below is the hardware structure of Flipper Zero, where:

- **Purple** — Blocks within the MCU
- **Blue** — Chips external to the MCU
- **Orange** — Other peripheral components that do not require complex interaction with chips

\image html flipper_zero_hardware.png "Hardware structure of Flipper Zero" width=700

The Flipper Zero firmware is open-source, which means you can access the hardware directly from your software by implementing your own low level drivers.

## Software

To simplify development, Flipper Zero includes a dedicated software abstraction layer between the bare-metal hardware and application-level code — **Furi** (<strong>F</strong>lipper <strong>U</strong>niversal <strong>R</strong>egistry <strong>I</strong>mplementation). Furi is a set of libraries included in the Flipper Zero firmware. It provides a straightforward API for interacting with hardware components and handling common system tasks such as thread management, dynamic memory allocation, and resource access control.

With Furi, developers don't need to read hundreds of pages of datasheets, analyze schematics, or write low-level drivers. Learning the Furi API is sufficient. This approach saves development time and reduces code duplication across common tasks implemented by different developers.

When added onto the hardware structure, the resulting architecture looks like this:

\image html flipper_zero_architecture.png "Hardware + software structure of Flipper Zero" width=700

As shown, Furi spans the full width of the diagram, illustrating how it effectively separates and abstracts the application-level code (services and applications) from the underlying hardware.

> The primary purpose of Furi libraries is to provide Flipper Zero developers with a simple API for hardware interaction and common system tasks.

## Examples

Furi is used in all Flipper Zero applications, making it easy to find examples of interacting with any hardware component. We recommend starting with the example applications in the [official Flipper Zero firmware repository](https://github.com/flipperdevices/flipperzero-firmware). You can also take a look at community-developed applications in the [Apps Catalog](https://lab.flipper.net/apps) (link to the source code can be found at the bottom of each app description), focusing on apps whose functionality closely matches what you aim to develop.

## Furi source code

Furi Core: [https://github.com/flipperdevices/flipperzero-firmware/tree/dev/furi](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/furi)

Furi HAL (Hardware Abstraction Layer): [https://github.com/flipperdevices/flipperzero-firmware/tree/dev/targets](https://github.com/flipperdevices/flipperzero-firmware/tree/dev/targets)