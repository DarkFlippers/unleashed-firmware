[Device schematic (pdf)](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/F1B1C0.0.pdf)

## Errata

* NAND reset IC drive reset net by push-pull output may cause MCU fault and disable internal MCU reset feature.
* System going to boot-loop state when reset by reset circuit
* No series resistors on external gpio for protecting MCU
* Wrong value LED series resistor
* Wrong footprint for CMOS transistors
* Wrong value IR TX series resistor
* No need capacitor on IR RX
* Wrong value IR RX pullup resistor
* 125 kHz RFID + iButton wrong schematic
* It seems bad to place main VCC on external GPIO pins. Overcurrent can damage main regulator and cause device broken. User can apply wrong voltage on this pin.

### New RFID + iButton schematic

after many experiments we found good schematic solution for work with RFID and auxiliary follow iButton level (for non-TTL protocol like Cyphral)

![new RFID + iButton schematic](https://github.com/Flipper-Zero/wiki/raw/master/images/new-rfid-ibutton-sch.png)
