#pragma once

#include "main.h"
#include <furi.h>

#include <stm32wbxx.h>
#include <stm32wbxx_ll_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POWER_I2C_SCL_Pin LL_GPIO_PIN_9
#define POWER_I2C_SCL_GPIO_Port GPIOA
#define POWER_I2C_SDA_Pin LL_GPIO_PIN_10
#define POWER_I2C_SDA_GPIO_Port GPIOA

#define POWER_I2C I2C1
/** Timing register value is computed with the STM32CubeMX Tool,
  * Standard Mode @100kHz with I2CCLK = 64 MHz,
  * rise time = 0ns, fall time = 0ns
  */
#define POWER_I2C_TIMINGS_100 0x10707DBC

/** Timing register value is computed with the STM32CubeMX Tool,
  * Fast Mode @400kHz with I2CCLK = 64 MHz,
  * rise time = 0ns, fall time = 0ns
  */
#define POWER_I2C_TIMINGS_400 0x00602173

/* Input Related Constants */
#define INPUT_DEBOUNCE_TICKS 20

/* Input Keys */
typedef enum {
    InputKeyUp,
    InputKeyDown,
    InputKeyRight,
    InputKeyLeft,
    InputKeyOk,
    InputKeyBack,
} InputKey;

/* Light */
typedef enum {
    LightRed,
    LightGreen,
    LightBlue,
    LightBacklight,
} Light;

typedef struct {
    const GPIO_TypeDef* port;
    const uint16_t pin;
    const InputKey key;
    const bool inverted;
    const char* name;
} InputPin;

extern const InputPin input_pins[];
extern const size_t input_pins_count;

extern const GpioPin vibro_gpio;
extern const GpioPin ibutton_gpio;

extern const GpioPin gpio_cc1101_g0;
extern const GpioPin gpio_rf_sw_0;

extern const GpioPin gpio_subghz_cs;
extern const GpioPin gpio_display_cs;
extern const GpioPin gpio_display_rst;
extern const GpioPin gpio_display_di;
extern const GpioPin gpio_sdcard_cs;
extern const GpioPin gpio_nfc_cs;

extern const GpioPin gpio_spi_d_miso;
extern const GpioPin gpio_spi_d_mosi;
extern const GpioPin gpio_spi_d_sck;
extern const GpioPin gpio_spi_r_miso;
extern const GpioPin gpio_spi_r_mosi;
extern const GpioPin gpio_spi_r_sck;

extern const GpioPin gpio_ext_pc0;
extern const GpioPin gpio_ext_pc1;
extern const GpioPin gpio_ext_pc3;
extern const GpioPin gpio_ext_pb2;
extern const GpioPin gpio_ext_pb3;
extern const GpioPin gpio_ext_pa4;
extern const GpioPin gpio_ext_pa6;
extern const GpioPin gpio_ext_pa7;

extern const GpioPin gpio_rfid_pull;
extern const GpioPin gpio_rfid_carrier_out;
extern const GpioPin gpio_rfid_data_in;
extern const GpioPin gpio_rfid_carrier;

extern const GpioPin gpio_irda_rx;
extern const GpioPin gpio_irda_tx;

extern const GpioPin gpio_usart_tx;
extern const GpioPin gpio_usart_rx;

#ifdef __cplusplus
}
#endif
