# GPS for Flipper Zero

[Original link](https://github.com/ezod/flipperzero-gps)

A simple Flipper Zero application for NMEA 0183 serial GPS modules, such as the [Adafruit Ultimate GPS Breakout].

Heavy lifting (NMEA parsing) provided by [minmea], which is included in this repository.

## Hardware Setup

Connect the GPS module to power and the USART using GPIO pins 9 (3.3V), 11 (GND), 13 (TX), and 14 (RX), as appropriate.

## Building the FAP

1. Clone the [flipperzero-firmware] repository.
2. Create a symbolic link in `applications_user` named `gps`, pointing to this repository.
3. Compile with `./fbt fap_gps`. This will create the `gps.fap` binary.

[Adafruit Ultimate GPS Breakout]: https://www.adafruit.com/product/746
[minmea]: https://github.com/kosma/minmea
[flipperzero-firmware]: https://github.com/flipperdevices/flipperzero-firmware
