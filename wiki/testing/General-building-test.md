Local target:

* `docker-compose exec dev make -C firmware TARGET=local APP_TEST=1 clean`
* `docker-compose exec dev make -C firmware TARGET=local APP_TEST=1 run`
* check tests pass/fail (by exitcode == 0)

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
* blue led when test is running
* green led if test is passed
* red led if test is failed

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