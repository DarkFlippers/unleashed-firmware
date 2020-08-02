# Hardware version 1.0

[device schematic](flipper_zero_rev_1_0.pdf)

## Errata

* NAND reset IC drive reset net by push-pull output may cause MCU fault and disable internal MCU reset feature.
* System going to boot-loop state when reset by reset circuit
* No resistors on external gpio for protecting MCU
* Wrong value LED series resistor
* Wrong footprint for CMOS transistors
* Wrong value IR TX series resistor
* No need capacitor on IR RX
* Wrong value IR RX pullup resistor
* 125 kHz RFID + iButton wrong schematic
* It seems bad to place main VCC on external GPIO pins. Overcurrent can damage main regulator and cause device broken. User can apply wrong voltage on this pin.

## New RFID + iButton schematic

after many experiments we found good schematic solution for work with RFID and auxiliary follow iButton level (for non-TTL protocol like Cyphral)

