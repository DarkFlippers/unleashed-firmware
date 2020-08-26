In this example we try to use FURI for interacting between user application and core subsystem.

First of all, we open FURI record by name "tty". This record is used for send some debug/logging info and interact with user by kind-of-TTY (like UART or USB CDC). By default on Flipper target all writes to tty record handled by debug UART (configured by `DEBUG_UART` define). On local target all writes simply prints to stdout.

Open record:

```C
FuriRecordSubscriber* log = get_default_log();
```

This is just wrapper on common FURI method:

```C
furi_open("tty", false, false, NULL, NULL);
```

"tty" is FURI pipe record. It means that there is no "data" hold in record, it only manage callbacks: when you call `furi_write`, all subscriber's callback is called. You can find default implementation in `core/tty_uart.c`.

Let's get a look at full example code:

```C
#include "flipper.h"
#include <string.h>
#include "log.h"

void application_uart_write(void* p) {
    // Red led for showing progress
    GpioPin led = {.pin = GPIO_PIN_8, .port = GPIOA};
    pinMode(led, GpioModeOpenDrain);

    // get_default_log open "tty" record
    FuriRecordSubscriber* log = get_default_log();

    // create buffer
    const char test_string[] = "test\n";
    furi_write(log, test_string, strlen(test_string));

    // for example, create counter and show its value
    uint8_t counter = 0;

    while(1) {
        // continously write it to UART
        fuprintf(log, "counter: %d\n", counter);
        counter++;

        // flash at every send
        digitalWrite(led, LOW);
        delay(50);
        digitalWrite(led, HIGH);

        // delay with overall perion of 1s
        delay(950);
    }
}
```

This code demonstrates two way to work with record:

1. Directly writes some data by `furi_write`
2. Uses `fuprintf` wrapper on `printf`.

For creating application and set it to autorun, read [Blink example](Blink-app).

_You can also find source of this example in `applications/examples/uart_write.c` and run it by `docker-compose exec dev make -C target_lo example_uart_write`_

![](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/application_examples/example_uart_write.gif)

_Code for target F1 can be compiled by `docker-compose exec dev make -C target_f1 example_uart_write`_

![](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/application_examples/example_uart_write_hw.gif)

