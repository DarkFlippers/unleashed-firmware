#pragma once

#include <furi.h>
#include <furi_hal_adc.h>
#include <furi_hal_pwm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Input Related Constants */
#define INPUT_DEBOUNCE_TICKS 4

/* Input Keys */
typedef enum {
    InputKeyUp,
    InputKeyDown,
    InputKeyRight,
    InputKeyLeft,
    InputKeyOk,
    InputKeyBack,
    InputKeyMAX, /**< Special value */
} InputKey;

/* Light */
typedef enum {
    LightRed = (1 << 0),
    LightGreen = (1 << 1),
    LightBlue = (1 << 2),
    LightBacklight = (1 << 3),
} Light;

typedef struct {
    const GpioPin* gpio;
    const InputKey key;
    const bool inverted;
    const char* name;
} InputPin;

typedef struct {
    const GpioPin* pin;
    const char* name;
    const FuriHalAdcChannel channel;
    const FuriHalPwmOutputId pwm_output;
    const uint8_t number;
    const bool debug;
} GpioPinRecord;

extern const InputPin input_pins[];
extern const size_t input_pins_count;

extern const GpioPinRecord gpio_pins[];
extern const size_t gpio_pins_count;

extern const GpioPin gpio_swdio;
extern const GpioPin gpio_swclk;

extern const GpioPin gpio_vibro;
extern const GpioPin gpio_ibutton;

extern const GpioPin gpio_display_cs;
extern const GpioPin gpio_display_rst_n;
extern const GpioPin gpio_display_di;
extern const GpioPin gpio_sdcard_cs;
extern const GpioPin gpio_sdcard_cd;

extern const GpioPin gpio_button_up;
extern const GpioPin gpio_button_down;
extern const GpioPin gpio_button_right;
extern const GpioPin gpio_button_left;
extern const GpioPin gpio_button_ok;
extern const GpioPin gpio_button_back;

extern const GpioPin gpio_spi_d_miso;
extern const GpioPin gpio_spi_d_mosi;
extern const GpioPin gpio_spi_d_sck;

extern const GpioPin gpio_ext_pc0;
extern const GpioPin gpio_ext_pc1;
extern const GpioPin gpio_ext_pc3;
extern const GpioPin gpio_ext_pb2;
extern const GpioPin gpio_ext_pb3;
extern const GpioPin gpio_ext_pa4;
extern const GpioPin gpio_ext_pa6;
extern const GpioPin gpio_ext_pa7;

extern const GpioPin gpio_ext_pc5;
extern const GpioPin gpio_ext_pc4;
extern const GpioPin gpio_ext_pa5;
extern const GpioPin gpio_ext_pb9;
extern const GpioPin gpio_ext_pa0;
extern const GpioPin gpio_ext_pa1;
extern const GpioPin gpio_ext_pa15;
extern const GpioPin gpio_ext_pe4;
extern const GpioPin gpio_ext_pa2;
extern const GpioPin gpio_ext_pb4;
extern const GpioPin gpio_ext_pb5;
extern const GpioPin gpio_ext_pd0;
extern const GpioPin gpio_ext_pb13;

extern const GpioPin gpio_usart_tx;
extern const GpioPin gpio_usart_rx;
extern const GpioPin gpio_i2c_power_sda;
extern const GpioPin gpio_i2c_power_scl;

extern const GpioPin gpio_speaker;

extern const GpioPin gpio_periph_power;

extern const GpioPin gpio_usb_dm;
extern const GpioPin gpio_usb_dp;

void furi_hal_resources_init_early(void);

void furi_hal_resources_deinit_early(void);

void furi_hal_resources_init(void);

/**
 * Get a corresponding external connector pin number for a gpio
 * @param gpio GpioPin
 * @return pin number or -1 if gpio is not on the external connector
 */
int32_t furi_hal_resources_get_ext_pin_number(const GpioPin* gpio);

/**
 * @brief Finds a pin by its name
 * 
 * @param name case-insensitive pin name to look for (e.g. `"Pc3"`, `"pA4"`)
 * 
 * @return a pointer to the corresponding `GpioPinRecord` if such a pin exists,
 *         `NULL` otherwise.
 */
const GpioPinRecord* furi_hal_resources_pin_by_name(const char* name);

/**
 * @brief Finds a pin by its number
 * 
 * @param name pin number to look for (e.g. `7`, `4`)
 * 
 * @return a pointer to the corresponding `GpioPinRecord` if such a pin exists,
 *         `NULL` otherwise.
 */
const GpioPinRecord* furi_hal_resources_pin_by_number(uint8_t number);

#ifdef __cplusplus
}
#endif
