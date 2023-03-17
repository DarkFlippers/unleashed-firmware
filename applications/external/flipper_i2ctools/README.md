# flipperzero-i2ctools

[Original link](https://github.com/NaejEL/flipperzero-i2ctools)
Set of i2c tools for Flipper Zero

![Preview](i2ctools.gif)

## Wiring

C0 -> SCL

C1 -> SDA

GND -> GND

>/!\ Target must use 3v3 logic levels. If you not sure use an i2c isolator like ISO1541

## Tools

### Scanner

Look for i2c peripherals adresses

### Sniffer

Spy i2c traffic

### Sender

Send command to i2c peripherals and read result 

## TODO

- [ ] Read more than 2 bytes in sender mode
- [ ] Add 10-bits adresses support
- [ ] Test with rate > 100khz
- [ ] Save records
- [ ] Play from files
- [ ] Kicad module
- [ ] Improve UI
- [ ] Refactor Event Management Code
- [ ] Add Documentation
- [ ] Remove max data size
- [ ] Remove max frames read size