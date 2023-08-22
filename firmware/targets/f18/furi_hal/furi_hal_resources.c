#include <furi_hal_resources.h>
#include <furi_hal_bus.h>
#include <furi.h>

#include <stm32wbxx_ll_rcc.h>
#include <stm32wbxx_ll_pwr.h>

#define TAG "FuriHalResources"

const GpioPin gpio_swdio = {.port = GPIOA, .pin = LL_GPIO_PIN_13};
const GpioPin gpio_swclk = {.port = GPIOA, .pin = LL_GPIO_PIN_14};

const GpioPin gpio_vibro = {.port = GPIOA, .pin = LL_GPIO_PIN_8};
const GpioPin gpio_ibutton = {.port = GPIOB, .pin = LL_GPIO_PIN_14};

const GpioPin gpio_display_cs = {.port = GPIOC, .pin = LL_GPIO_PIN_11};
const GpioPin gpio_display_rst_n = {.port = GPIOB, .pin = LL_GPIO_PIN_0};
const GpioPin gpio_display_di = {.port = GPIOB, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_sdcard_cs = {.port = GPIOC, .pin = LL_GPIO_PIN_12};
const GpioPin gpio_sdcard_cd = {.port = GPIOC, .pin = LL_GPIO_PIN_10};

const GpioPin gpio_button_up = {.port = GPIOB, .pin = LL_GPIO_PIN_10};
const GpioPin gpio_button_down = {.port = GPIOC, .pin = LL_GPIO_PIN_6};
const GpioPin gpio_button_right = {.port = GPIOB, .pin = LL_GPIO_PIN_12};
const GpioPin gpio_button_left = {.port = GPIOB, .pin = LL_GPIO_PIN_11};
const GpioPin gpio_button_ok = {.port = GPIOH, .pin = LL_GPIO_PIN_3};
const GpioPin gpio_button_back = {.port = GPIOC, .pin = LL_GPIO_PIN_13};

const GpioPin gpio_spi_d_miso = {.port = GPIOC, .pin = LL_GPIO_PIN_2};
const GpioPin gpio_spi_d_mosi = {.port = GPIOB, .pin = LL_GPIO_PIN_15};
const GpioPin gpio_spi_d_sck = {.port = GPIOD, .pin = LL_GPIO_PIN_1};

const GpioPin gpio_ext_pc0 = {.port = GPIOC, .pin = LL_GPIO_PIN_0};
const GpioPin gpio_ext_pc1 = {.port = GPIOC, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_ext_pc3 = {.port = GPIOC, .pin = LL_GPIO_PIN_3};
const GpioPin gpio_ext_pb2 = {.port = GPIOB, .pin = LL_GPIO_PIN_2};
const GpioPin gpio_ext_pb3 = {.port = GPIOB, .pin = LL_GPIO_PIN_3};
const GpioPin gpio_ext_pa4 = {.port = GPIOA, .pin = LL_GPIO_PIN_4};
const GpioPin gpio_ext_pa6 = {.port = GPIOA, .pin = LL_GPIO_PIN_6};
const GpioPin gpio_ext_pa7 = {.port = GPIOA, .pin = LL_GPIO_PIN_7};

const GpioPin gpio_ext_pc5 = {.port = GPIOC, .pin = LL_GPIO_PIN_5};
const GpioPin gpio_ext_pc4 = {.port = GPIOC, .pin = LL_GPIO_PIN_4};
const GpioPin gpio_ext_pa5 = {.port = GPIOA, .pin = LL_GPIO_PIN_5};
const GpioPin gpio_ext_pb9 = {.port = GPIOB, .pin = LL_GPIO_PIN_9};
const GpioPin gpio_ext_pa0 = {.port = GPIOA, .pin = LL_GPIO_PIN_0};
const GpioPin gpio_ext_pa1 = {.port = GPIOA, .pin = LL_GPIO_PIN_1};
const GpioPin gpio_ext_pa15 = {.port = GPIOA, .pin = LL_GPIO_PIN_15};
const GpioPin gpio_ext_pe4 = {.port = GPIOE, .pin = LL_GPIO_PIN_4};
const GpioPin gpio_ext_pa2 = {.port = GPIOA, .pin = LL_GPIO_PIN_2};
const GpioPin gpio_ext_pb4 = {.port = GPIOB, .pin = LL_GPIO_PIN_4};
const GpioPin gpio_ext_pb5 = {.port = GPIOB, .pin = LL_GPIO_PIN_5};
const GpioPin gpio_ext_pd0 = {.port = GPIOD, .pin = LL_GPIO_PIN_0};
const GpioPin gpio_ext_pb13 = {.port = GPIOB, .pin = LL_GPIO_PIN_13};

const GpioPin gpio_usart_tx = {.port = GPIOB, .pin = LL_GPIO_PIN_6};
const GpioPin gpio_usart_rx = {.port = GPIOB, .pin = LL_GPIO_PIN_7};

const GpioPin gpio_i2c_power_sda = {.port = GPIOA, .pin = LL_GPIO_PIN_10};
const GpioPin gpio_i2c_power_scl = {.port = GPIOA, .pin = LL_GPIO_PIN_9};

const GpioPin gpio_speaker = {.port = GPIOB, .pin = LL_GPIO_PIN_8};

const GpioPin gpio_periph_power = {.port = GPIOA, .pin = LL_GPIO_PIN_3};

const GpioPin gpio_usb_dm = {.port = GPIOA, .pin = LL_GPIO_PIN_11};
const GpioPin gpio_usb_dp = {.port = GPIOA, .pin = LL_GPIO_PIN_12};

const GpioPinRecord gpio_pins[] = {
    {.pin = &gpio_ext_pa7, .name = "PA7", .debug = false},
    {.pin = &gpio_ext_pa6, .name = "PA6", .debug = false},
    {.pin = &gpio_ext_pa4, .name = "PA4", .debug = false},
    {.pin = &gpio_ext_pb3, .name = "PB3", .debug = false},
    {.pin = &gpio_ext_pb2, .name = "PB2", .debug = false},
    {.pin = &gpio_ext_pc3, .name = "PC3", .debug = false},
    {.pin = &gpio_ext_pc1, .name = "PC1", .debug = false},
    {.pin = &gpio_ext_pc0, .name = "PC0", .debug = false},

    {.pin = &gpio_ext_pc5, .name = "PC5", .debug = false},
    {.pin = &gpio_ext_pc4, .name = "PC4", .debug = false},
    {.pin = &gpio_ext_pa5, .name = "PA5", .debug = false},
    {.pin = &gpio_ext_pb9, .name = "PB9", .debug = false},
    {.pin = &gpio_ext_pa0, .name = "PA0", .debug = false},
    {.pin = &gpio_ext_pa1, .name = "PA1", .debug = false},
    {.pin = &gpio_ext_pa15, .name = "PA15", .debug = false},
    {.pin = &gpio_ext_pe4, .name = "PE4", .debug = false},
    {.pin = &gpio_ext_pa2, .name = "PA2", .debug = false},
    {.pin = &gpio_ext_pb4, .name = "PB4", .debug = false},
    {.pin = &gpio_ext_pb5, .name = "PB5", .debug = false},
    {.pin = &gpio_ext_pd0, .name = "PD0", .debug = false},
    {.pin = &gpio_ext_pb13, .name = "PB13", .debug = false},

    /* Dangerous pins, may damage hardware */
    {.pin = &gpio_usart_rx, .name = "PB7", .debug = true},
    {.pin = &gpio_speaker, .name = "PB8", .debug = true},
};

const size_t gpio_pins_count = sizeof(gpio_pins) / sizeof(GpioPinRecord);

const InputPin input_pins[] = {
    {.gpio = &gpio_button_up, .key = InputKeyUp, .inverted = true, .name = "Up"},
    {.gpio = &gpio_button_down, .key = InputKeyDown, .inverted = true, .name = "Down"},
    {.gpio = &gpio_button_right, .key = InputKeyRight, .inverted = true, .name = "Right"},
    {.gpio = &gpio_button_left, .key = InputKeyLeft, .inverted = true, .name = "Left"},
    {.gpio = &gpio_button_ok, .key = InputKeyOk, .inverted = false, .name = "OK"},
    {.gpio = &gpio_button_back, .key = InputKeyBack, .inverted = true, .name = "Back"},
};

const size_t input_pins_count = sizeof(input_pins) / sizeof(InputPin);

static void furi_hal_resources_init_input_pins(GpioMode mode) {
    for(size_t i = 0; i < input_pins_count; i++) {
        furi_hal_gpio_init(
            input_pins[i].gpio,
            mode,
            (input_pins[i].inverted) ? GpioPullUp : GpioPullDown,
            GpioSpeedLow);
    }
}

void furi_hal_resources_init_early() {
    furi_hal_bus_enable(FuriHalBusGPIOA);
    furi_hal_bus_enable(FuriHalBusGPIOB);
    furi_hal_bus_enable(FuriHalBusGPIOC);
    furi_hal_bus_enable(FuriHalBusGPIOD);
    furi_hal_bus_enable(FuriHalBusGPIOE);
    furi_hal_bus_enable(FuriHalBusGPIOH);

    furi_hal_resources_init_input_pins(GpioModeInput);

    // SD Card stepdown control
    furi_hal_gpio_write(&gpio_periph_power, 1);
    furi_hal_gpio_init(&gpio_periph_power, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);

    // Display pins
    furi_hal_gpio_write(&gpio_display_rst_n, 1);
    furi_hal_gpio_init_simple(&gpio_display_rst_n, GpioModeOutputPushPull);
    furi_hal_gpio_init(&gpio_display_di, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);

    // Pullup display reset pin for shutdown
    SET_BIT(PWR->PUCRB, gpio_display_rst_n.pin);
    CLEAR_BIT(PWR->PDCRB, gpio_display_rst_n.pin);
    SET_BIT(PWR->CR3, PWR_CR3_APC);

    // Hard reset USB
    furi_hal_gpio_write(&gpio_usb_dm, 1);
    furi_hal_gpio_write(&gpio_usb_dp, 1);
    furi_hal_gpio_init_simple(&gpio_usb_dm, GpioModeOutputOpenDrain);
    furi_hal_gpio_init_simple(&gpio_usb_dp, GpioModeOutputOpenDrain);
    furi_hal_gpio_write(&gpio_usb_dm, 0);
    furi_hal_gpio_write(&gpio_usb_dp, 0);
    furi_delay_us(5); // Device Driven disconnect: 2.5us + extra to compensate cables
    furi_hal_gpio_write(&gpio_usb_dm, 1);
    furi_hal_gpio_write(&gpio_usb_dp, 1);
    furi_hal_gpio_init_simple(&gpio_usb_dm, GpioModeAnalog);
    furi_hal_gpio_init_simple(&gpio_usb_dp, GpioModeAnalog);
    furi_hal_gpio_write(&gpio_usb_dm, 0);
    furi_hal_gpio_write(&gpio_usb_dp, 0);

    // External header pins
    furi_hal_gpio_init(&gpio_ext_pc0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&gpio_ext_pc1, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&gpio_ext_pc3, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&gpio_ext_pb2, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&gpio_ext_pb3, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&gpio_ext_pa4, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&gpio_ext_pa6, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(&gpio_ext_pa7, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void furi_hal_resources_deinit_early() {
    furi_hal_resources_init_input_pins(GpioModeAnalog);
    furi_hal_bus_disable(FuriHalBusGPIOA);
    furi_hal_bus_disable(FuriHalBusGPIOB);
    furi_hal_bus_disable(FuriHalBusGPIOC);
    furi_hal_bus_disable(FuriHalBusGPIOD);
    furi_hal_bus_disable(FuriHalBusGPIOE);
    furi_hal_bus_disable(FuriHalBusGPIOH);
}

void furi_hal_resources_init() {
    // Button pins
    furi_hal_resources_init_input_pins(GpioModeInterruptRiseFall);

    // Explicit pulls pins
    LL_PWR_EnablePUPDCfg();
    LL_PWR_EnableGPIOPullDown(LL_PWR_GPIO_B, LL_PWR_GPIO_BIT_8); // gpio_speaker
    LL_PWR_EnableGPIOPullDown(LL_PWR_GPIO_A, LL_PWR_GPIO_BIT_8); // gpio_vibro

    // Display pins
    furi_hal_gpio_init(&gpio_display_rst_n, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_display_rst_n, 0);

    furi_hal_gpio_init(&gpio_display_di, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_display_di, 0);

    // SD pins
    furi_hal_gpio_init(&gpio_sdcard_cd, GpioModeInput, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_sdcard_cd, 0);

    furi_hal_gpio_init(&gpio_ibutton, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI0_IRQn);

    NVIC_SetPriority(EXTI1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI1_IRQn);

    NVIC_SetPriority(EXTI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI2_IRQn);

    NVIC_SetPriority(EXTI3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI3_IRQn);

    NVIC_SetPriority(EXTI4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI4_IRQn);

    NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_EnableIRQ(EXTI15_10_IRQn);

    FURI_LOG_I(TAG, "Init OK");
}

int32_t furi_hal_resources_get_ext_pin_number(const GpioPin* gpio) {
    // TODO FL-3500: describe second ROW
    if(gpio == &gpio_ext_pa7)
        return 2;
    else if(gpio == &gpio_ext_pa6)
        return 3;
    else if(gpio == &gpio_ext_pa4)
        return 4;
    else if(gpio == &gpio_ext_pb3)
        return 5;
    else if(gpio == &gpio_ext_pb2)
        return 6;
    else if(gpio == &gpio_ext_pc3)
        return 7;
    else if(gpio == &gpio_ext_pc1)
        return 15;
    else if(gpio == &gpio_ext_pc0)
        return 16;
    else if(gpio == &gpio_ibutton)
        return 17;
    else
        return -1;
}
