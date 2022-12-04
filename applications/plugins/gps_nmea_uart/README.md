# GPS for Flipper Zero

A simple Flipper Zero application for NMEA 0183 serial GPS modules, such as the

[Original link](https://github.com/ezod/flipperzero-gps)
[Adafruit Ultimate GPS Breakout].

[Original link](https://github.com/ezod/flipperzero-gps)

![ui](ui.png)

Heavy lifting (NMEA parsing) provided by [minmea], which is included in this
repository.

## Hardware Setup

Connect the GPS module to power and the USART using GPIO pins 9 (3.3V), 11
(GND), 13 (TX), and 14 (RX), as appropriate.

![wiring](wiring.png)


## Contributing

This project was a learning exercise and is more or less "complete" from my
perspective, but I will happily accept pull requests that improve and enhance
the functionality for others.

Currently, the app only parses RMC and GGA sentences, and displays a subset of
the data that fits on the screen. The UART is also hard-coded to 9600 baud.
These limitations are largely driven by the GPS module I have to work with. A
more elaborate UI with scrolling or multiple screens, as well as a configurable
baud rate, may be useful for other GPS modules.

[Adafruit Ultimate GPS Breakout]: https://www.adafruit.com/product/746
[minmea]: https://github.com/kosma/minmea
[flipperzero-firmware]: https://github.com/flipperdevices/flipperzero-firmware
[qFlipper]: https://flipperzero.one/update
