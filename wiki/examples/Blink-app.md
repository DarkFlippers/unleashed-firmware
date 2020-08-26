First, let's create a simple led blinking application.

## Preparing for launch

We will use integrated LED. Look at the schematic:

![](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/application_examples/leds.png)
![](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/application_examples/gpio_pa8.png)

This led connect between power rail and GPIO PA8 and we should configure this pin as open drain to properly control led behaviour.

You can find GPIO API in `target_*/flipper_hal.h`. Or if you prefer to use Arduino API, you can find bindings in `core/flipper.h`.

For work with pin we should:

1. Create `GpioPin` instance and specify pin and port.
2. Configure mode of pin by `pinMode` function.
3. Control state of pin by `digitalWrite` function.

## Creating application

1. Create new file (for example, `blink.c`) in `applications` folder.
2. Create code like this:

```C
#include "flipper.h"

void application_blink(void* p) {
    // create pin
    GpioPin led = {.pin = GPIO_PIN_8, .port = GPIOA};

    // configure pin
    pinMode(led, GpioModeOpenDrain);

    while(1) {
        digitalWrite(led, HIGH);
        delay(500);
        digitalWrite(led, LOW);
        delay(500);
    }
}
```
3. To start your application on Flipper startup, add it to autorun:
    * in `applications/startup.h` add prototype of main application function:

    ```C
    void application_blink(void* p);
    ```

    * add entry to `FLIPPER_STARTUP` array (pointer to application function and application name):

    ```C
    const FlipperStartupApp FLIPPER_STARTUP[] = {
        #ifdef TEST
        {.app = flipper_test_app, .name = "test app"}
        #endif

        // user applications:

        , {.app = application_blink, .name = "blink"}  
    };
    ```

4. Add your application file to Makefile (for each target, `target_lo/Makefile` and `target_f1/Makefile`, we add one common makefile later):

```
# User application

C_SOURCES += ../applications/blink.c
```

Build and run for linux (target_lo):

`docker-compose exec dev make -C target_lo`

Run:

`docker-compose exec dev target_lo/build/target_lo`.

Linux version has no LED or GPIO, but we can see debug messages how state of GPIO is changing:

![](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/application_examples/example_blink.gif)

_You also run found source of this example in `applications/examples/blink.c` and run by `docker-compose exec dev make -C target_lo example_blink`_

Build for Flipper (board F1):

`docker-compose exec dev make -C target_f1`

Upload to microcontroller:

`./target_f1/deploy.sh`

Blink!

![](https://github.com/Flipper-Zero/flipperzero-firmware-community/raw/master/wiki_static/application_examples/example_blink_hw.gif)

_You also compile by `docker-compose exec dev make -C target_f1 example_blink`_
