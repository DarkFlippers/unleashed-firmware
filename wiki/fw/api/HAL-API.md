# GPIO

GPIO defined as struct `GpioPin`.

GPIO functions:

```C
// Init GPIO
void gpio_init(GpioPin* gpio, GpioMode mode);

typedef enum { GpioModeInput, GpioModeOutput, GpioModeOpenDrain } GpioMode;

// write value to GPIO
void gpio_write(GpioPin* gpio, bool state);

// read value from GPIO, f = LOW, t = HIGH
bool gpio_read(GpioPin* gpio);
```

When application is exited, system place pin to Z-state by calling `gpio_disable`.

```C
// put GPIO to Z-state (used for restore pin state on app exit)
void gpio_disable(ValueMutex* gpio_mutex) {
    GpioPin* gpio = acquire_mutex(gpio_mutex, 0);
    gpio_init(gpio, GpioModeInput);
    release_mutex(gpio_mutex, gpio);
}
```

Available GPIO stored in FURI as `ValueMutex<GpioPin*>`.

```C
inline static ValueMutex* open_gpio_mutex(const char* name) {
    ValueMutex* gpio_mutex = (ValueMutex*)furi_open(name);
    if(gpio_mutex != NULL) flapp_on_exit(gpio_disable, gpio_mutex);

    return gpio_mutex;
}

// helper
inline static GpioPin* open_gpio(const char* name) {
    ValueMutex* gpio_mutex = open_gpio(name);
    return (GpioPin*)acquire_mutex(gpio_mutex, 0);
}
```

## Available GPIO (target F2)

* PA4
* PA5
* PA6
* PA7
* PB2
* PC3
* PC0
* PC1
* PB6
* PB7
* PA13
* PA14
* RFID_PULL
* IR_TX
* IBUTTON
* VIBRO

## Usage example

```C
void gpio_example() {
    GpioPin* pin = open_gpio("PB6");

    if(pin == NULL) {
        printf("pin not available\n");
        return;
    }

    gpio_init(pin, GpioModeOutput);

    while(1) {
        gpio_write(pin, true);
        delay(100);
        gpio_write(pin, false);
        delay(100);
    }
}
```

# PWM

PWM defined as `PwmPin`. To set PWM channel:

```C
void pwm_set(PwmPin* pwm, float value, float freq);
```

When application is exited, system disable pwm by calling `pwm_disable`.

```C
// put GPIO to Z-state (used for restore pin state on app exit)
void pwm_disable(ValueMutex* pwm_mutex) {
    PwmPin* pwm = acquire_mutex(pwm_mutex, 0);
    pwm_set(pwm, 0., 0.);
    release_mutex(pwm_mutex, pwm);
}
```

Available PWM stored in FURI as `ValueMutex<PwmPin*>`.

```C
inline static ValueMutex* open_pwm_mutex(const char* name) {
    ValueMutex* pwm_mutex = (ValueMutex*)furi_open(name);
    if(pwm_mutex != NULL) flapp_on_exit(pwm_disable, pwm_mutex);

    return pwm_mutex;
}

// helper
inline static PwmPin* open_pwm(const char* name) {
    ValueMutex* pwm_mutex = open_gpio(name);
    return (PwmPin*)acquire_mutex(pwm_mutex, 0);
}
```

## Available PWM (target F2)

* SPEAKER
* RFID_OUT

## Usage example

```C
void sound_example() {
    PwmPin* speaker = open_pwm("SPEAKER");

    if(speaker == NULL) {
        printf("speaker not available\n");
        return;
    }

    while(1) {
        pwm_set(speaker, 1000., 0.1);
        delay(2);
        pwm_set(speaker, 110., 0.5);
        delay(198);
        pwm_set(speaker, 330., 0.5);
        delay(200);
    }
}
```

# ADC

Coming soon...

# I2C

Coming soon...
