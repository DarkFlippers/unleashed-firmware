#include <furi.h>
#include <api-hal-gpio.h>
#include <api-hal-version.h>

#define GET_SYSCFG_EXTI_PORT(gpio)                \
    (((gpio) == (GPIOA)) ? LL_SYSCFG_EXTI_PORTA : \
     ((gpio) == (GPIOB)) ? LL_SYSCFG_EXTI_PORTB : \
     ((gpio) == (GPIOC)) ? LL_SYSCFG_EXTI_PORTC : \
     ((gpio) == (GPIOD)) ? LL_SYSCFG_EXTI_PORTD : \
     ((gpio) == (GPIOE)) ? LL_SYSCFG_EXTI_PORTE : \
                           LL_SYSCFG_EXTI_PORTH)

#define GPIO_PIN_MAP(pin, prefix)               \
    (((pin) == (LL_GPIO_PIN_0))  ? prefix##0 :  \
     ((pin) == (LL_GPIO_PIN_1))  ? prefix##1 :  \
     ((pin) == (LL_GPIO_PIN_2))  ? prefix##2 :  \
     ((pin) == (LL_GPIO_PIN_3))  ? prefix##3 :  \
     ((pin) == (LL_GPIO_PIN_4))  ? prefix##4 :  \
     ((pin) == (LL_GPIO_PIN_5))  ? prefix##5 :  \
     ((pin) == (LL_GPIO_PIN_6))  ? prefix##6 :  \
     ((pin) == (LL_GPIO_PIN_7))  ? prefix##7 :  \
     ((pin) == (LL_GPIO_PIN_8))  ? prefix##8 :  \
     ((pin) == (LL_GPIO_PIN_9))  ? prefix##9 :  \
     ((pin) == (LL_GPIO_PIN_10)) ? prefix##10 : \
     ((pin) == (LL_GPIO_PIN_11)) ? prefix##11 : \
     ((pin) == (LL_GPIO_PIN_12)) ? prefix##12 : \
     ((pin) == (LL_GPIO_PIN_13)) ? prefix##13 : \
     ((pin) == (LL_GPIO_PIN_14)) ? prefix##14 : \
                                   prefix##15)

#define GET_SYSCFG_EXTI_LINE(pin) GPIO_PIN_MAP(pin, LL_SYSCFG_EXTI_LINE)
#define GET_EXTI_LINE(pin) GPIO_PIN_MAP(pin, LL_EXTI_LINE_)

static volatile GpioInterrupt gpio_interrupt[GPIO_NUMBER];

static uint8_t hal_gpio_get_pin_num(const GpioPin* gpio) {
    uint8_t pin_num = 0;
    for(pin_num = 0; pin_num < GPIO_NUMBER; pin_num++) {
        if(gpio->pin & (1 << pin_num)) break;
    }
    return pin_num;
}

void hal_gpio_init(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed) {
    uint32_t sys_exti_port = GET_SYSCFG_EXTI_PORT(gpio->port);
    uint32_t sys_exti_line = GET_SYSCFG_EXTI_LINE(gpio->pin);
    uint32_t exti_line = GET_EXTI_LINE(gpio->pin);

    // Configure gpio with interrupts disabled
    __disable_irq();
    // Set gpio speed
    if(speed == GpioSpeedLow) {
        LL_GPIO_SetPinSpeed(gpio->port, gpio->pin, LL_GPIO_SPEED_FREQ_LOW);
    } else if(speed == GpioSpeedMedium) {
        LL_GPIO_SetPinSpeed(gpio->port, gpio->pin, LL_GPIO_SPEED_FREQ_MEDIUM);
    } else if(speed == GpioSpeedHigh) {
        LL_GPIO_SetPinSpeed(gpio->port, gpio->pin, LL_GPIO_SPEED_FREQ_HIGH);
    } else {
        LL_GPIO_SetPinSpeed(gpio->port, gpio->pin, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    }
    // Set gpio pull mode
    if(pull == GpioPullNo) {
        LL_GPIO_SetPinPull(gpio->port, gpio->pin, LL_GPIO_PULL_NO);
    } else if(pull == GpioPullUp) {
        LL_GPIO_SetPinPull(gpio->port, gpio->pin, LL_GPIO_PULL_UP);
    } else {
        LL_GPIO_SetPinPull(gpio->port, gpio->pin, LL_GPIO_PULL_DOWN);
    }
    // Set gpio mode
    if(mode >= GpioModeInterruptRise) {
        // Set pin in interrupt mode
        LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_INPUT);
        LL_SYSCFG_SetEXTISource(sys_exti_port, sys_exti_line);
        if(mode == GpioModeInterruptRise || mode == GpioModeInterruptRiseFall) {
            LL_EXTI_EnableIT_0_31(exti_line);
            LL_EXTI_EnableRisingTrig_0_31(exti_line);
        }
        if(mode == GpioModeInterruptFall || mode == GpioModeInterruptRiseFall) {
            LL_EXTI_EnableIT_0_31(exti_line);
            LL_EXTI_EnableFallingTrig_0_31(exti_line);
        }
        if(mode == GpioModeEventRise || mode == GpioModeInterruptRiseFall) {
            LL_EXTI_EnableEvent_0_31(exti_line);
            LL_EXTI_EnableRisingTrig_0_31(exti_line);
        }
        if(mode == GpioModeEventFall || mode == GpioModeInterruptRiseFall) {
            LL_EXTI_EnableEvent_0_31(exti_line);
            LL_EXTI_EnableFallingTrig_0_31(exti_line);
        }
    } else {
        // Disable interrupt if it was set
        if(LL_SYSCFG_GetEXTISource(sys_exti_line) == sys_exti_port &&
           LL_EXTI_IsEnabledIT_0_31(exti_line)) {
            LL_EXTI_DisableIT_0_31(exti_line);
            LL_EXTI_DisableRisingTrig_0_31(exti_line);
            LL_EXTI_DisableFallingTrig_0_31(exti_line);
        }
        // Set not interrupt pin modes
        if(mode == GpioModeInput) {
            LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_INPUT);
        } else if(mode == GpioModeOutputPushPull || mode == GpioModeAltFunctionPushPull) {
            LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_OUTPUT);
            LL_GPIO_SetPinOutputType(gpio->port, gpio->pin, LL_GPIO_OUTPUT_PUSHPULL);
        } else if(mode == GpioModeOutputOpenDrain || mode == GpioModeAltFunctionOpenDrain) {
            LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_OUTPUT);
            LL_GPIO_SetPinOutputType(gpio->port, gpio->pin, LL_GPIO_OUTPUT_OPENDRAIN);
        } else if(mode == GpioModeAnalog) {
            LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_ANALOG);
        }
    }
    __enable_irq();
}

void hal_gpio_init_alt(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed,
    const GpioAltFn alt_fn) {
    hal_gpio_init(gpio, mode, pull, speed);

    __disable_irq();
    // enable alternate mode
    LL_GPIO_SetPinMode(gpio->port, gpio->pin, LL_GPIO_MODE_ALTERNATE);

    // set alternate function
    if(hal_gpio_get_pin_num(gpio) < 8) {
        LL_GPIO_SetAFPin_0_7(gpio->port, gpio->pin, alt_fn);
    } else {
        LL_GPIO_SetAFPin_8_15(gpio->port, gpio->pin, alt_fn);
    }
    __enable_irq();
}

void hal_gpio_add_int_callback(const GpioPin* gpio, GpioExtiCallback cb, void* ctx) {
    furi_assert(gpio);
    furi_assert(cb);

    __disable_irq();
    uint8_t pin_num = hal_gpio_get_pin_num(gpio);
    gpio_interrupt[pin_num].callback = cb;
    gpio_interrupt[pin_num].context = ctx;
    gpio_interrupt[pin_num].ready = true;
    __enable_irq();
}

void hal_gpio_enable_int_callback(const GpioPin* gpio) {
    furi_assert(gpio);

    __disable_irq();
    uint8_t pin_num = hal_gpio_get_pin_num(gpio);
    if(gpio_interrupt[pin_num].callback) {
        gpio_interrupt[pin_num].ready = true;
    }
    __enable_irq();
}

void hal_gpio_disable_int_callback(const GpioPin* gpio) {
    furi_assert(gpio);

    __disable_irq();
    uint8_t pin_num = hal_gpio_get_pin_num(gpio);
    gpio_interrupt[pin_num].ready = false;
    __enable_irq();
}

void hal_gpio_remove_int_callback(const GpioPin* gpio) {
    furi_assert(gpio);

    __disable_irq();
    uint8_t pin_num = hal_gpio_get_pin_num(gpio);
    gpio_interrupt[pin_num].callback = NULL;
    gpio_interrupt[pin_num].context = NULL;
    gpio_interrupt[pin_num].ready = false;
    __enable_irq();
}

static void hal_gpio_int_call(uint16_t pin_num) {
    if(gpio_interrupt[pin_num].callback && gpio_interrupt[pin_num].ready) {
        gpio_interrupt[pin_num].callback(gpio_interrupt[pin_num].context);
    }
}

/* Interrupt handlers */
void EXTI0_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
        hal_gpio_int_call(0);
    }
}

void EXTI1_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_1)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);
        hal_gpio_int_call(1);
    }
}

void EXTI2_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
        hal_gpio_int_call(2);
    }
}

void EXTI3_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_3)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_3);
        hal_gpio_int_call(3);
    }
}

void EXTI4_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_4)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_4);
        hal_gpio_int_call(4);
    }
}

void EXTI9_5_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_5)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_5);
        hal_gpio_int_call(5);
    }
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_6)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_6);
        hal_gpio_int_call(6);
    }
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_7)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_7);
        hal_gpio_int_call(7);
    }
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_8)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_8);
        hal_gpio_int_call(8);
    }
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_9)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_9);
        hal_gpio_int_call(9);
    }
}

void EXTI15_10_IRQHandler(void) {
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_10)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_10);
        hal_gpio_int_call(10);
    }
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_11)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_11);
        hal_gpio_int_call(11);
    }
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_12)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_12);
        hal_gpio_int_call(12);
    }
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        hal_gpio_int_call(13);
    }
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_14)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_14);
        hal_gpio_int_call(14);
    }
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_15)) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_15);
        hal_gpio_int_call(15);
    }
}

extern COMP_HandleTypeDef hcomp1;

bool hal_gpio_get_rfid_in_level() {
    bool value = false;
    if(api_hal_version_get_hw_version() > 7) {
        value = (HAL_COMP_GetOutputLevel(&hcomp1) == COMP_OUTPUT_LEVEL_LOW);
    } else {
        value = (HAL_COMP_GetOutputLevel(&hcomp1) == COMP_OUTPUT_LEVEL_HIGH);
    }

#ifdef INVERT_RFID_IN
    return !value;
#else
    return value;
#endif
}
