# Bootloader testcase

1. `# Clean flash`
2. `make -C bootloader flash` `# Load bootloader`
3.  `# reboot device`
    * Press right
    * Press left
    * Wait 0.1 s
    * Release left
    * Release right
4. Wait 0.5 s
5. `# Expect no FW`
    * Expect: no uart welcome message
    * Expect: red led on
    * Expect: no USB
6. `# reboot device and go to DFU`
    * Press left 
    * Press right
    * Wait 0.1 s
    * Release left
    * Wait 0.5 s 
    * Release right
7. Wait 0.5 s
8. `# Expect DFU`
    * Expect: blue led on
    * Expect: USB: DFU
9. `target_f2/deploy-dfu.sh` `# load FW`
10. `# reboot device`
    * Press right
    * Press left
    * Wait 0.1 s
    * Release left
    * Release right
11. Wait 0.5 s
12. `# Expect FW`
    * Expect: uart welcome message
    * Expect: USB Flipper CDC

# Input testcase

1. `docker-compose exec dev make -C target_f2 example_input_dump`
2. Flash
3. For x in ```
[
    (Up, "00"),
    (Down, "01"),
    (Right, "02"),
    (Left, "03"),
    (Ok, "04"),
    (Back, "05"),
]
```
    * Press ${x[0]}
    * wait 0.05
    * Expect: Uart: "event: ${x[1]} pressed"
    * wait 0.05
    * Release ${x[0]}
    * wait 0.05
    * Expect: Uart: "event: ${x[1]} released"
    * wait 0.05

TODO: add debouncing check (multiple press and check there is no multiple events)

# General building testcase

Local target:

* `docker-compose exec dev make -C firmware TARGET=local APP_TEST=1 clean`
* `docker-compose exec dev make -C firmware TARGET=local APP_TEST=1 run`
* check tests pass/fail

* `docker-compose exec dev make -C firmware TARGET=local APP_EXAMPLE_BLINK=1 run`
* GPIO on and off

* `docker-compose exec dev make -C firmware TARGET=local APP_EXAMPLE_UART_WRITE=1 run`
* GPIO on/off and `counter: %` writes

* `docker-compose exec dev make -C firmware TARGET=local APP_EXAMPLE_IPC=1 run`
* ASCII display draw

* `docker-compose exec dev make -C firmware TARGET=local APP_EXAMPLE_INPUT_DUMP=1 run` not implemented

* `docker-compose exec dev make -C firmware TARGET=local APP_EXAMPLE_QRCODE=1 run`
* Some writes to display

* `docker-compose exec dev make -C firmware TARGET=local APP_EXAMPLE_DISPLAY=1 run`
* Some writes to display

* `docker-compose exec dev make -C firmware TARGET=local APP_EXAMPLE_FATFS=1 flash`
* TODO: FatFs emulation and test not implemented

F2 target:

* `docker-compose exec dev make -C firmware TARGET=f2 APP_TEST=1 clean`
* `docker-compose exec dev make -C firmware TARGET=f2 APP_TEST=1 flash`
* check UART for test pass/fail

* `docker-compose exec dev make -C firmware TARGET=f2 APP_EXAMPLE_BLINK=1 flash`
* Red LED blink (1s period)

* `docker-compose exec dev make -C firmware TARGET=f2 APP_EXAMPLE_UART_WRITE=1 flash`
* Red LED shortly blinking, `counter: %` writes to UART

* `docker-compose exec dev make -C firmware TARGET=f2 APP_EXAMPLE_IPC=1 flash`
* ASCII display draw in UART

* `docker-compose exec dev make -C firmware TARGET=f2 APP_EXAMPLE_INPUT_DUMP=1 flash`
* Press all buttons, `state` and `event` writes to UART

* `docker-compose exec dev make -C firmware TARGET=f2 APP_EXAMPLE_QRCODE=1 flash`
* QR code show on the screen

* `docker-compose exec dev make -C firmware TARGET=f2 APP_EXAMPLE_DISPLAY=1 flash`
* `Hello world` show on the screen

* `docker-compose exec dev make -C firmware TARGET=f2 APP_EXAMPLE_FATFS=1 flash`
* `Init sd card error` on the screen
* Insert SD-card
* Reboot
* Show file list on the screen
* Scroll by pressing up and down
