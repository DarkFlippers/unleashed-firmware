#include <api-hal-resources.h>
#include "main.h"
#include <furi.h>

const InputPin input_pins[] = {
    {.port = BUTTON_UP_GPIO_Port, .pin = BUTTON_UP_Pin, .key = InputKeyUp, .inverted = true},
    {.port = BUTTON_DOWN_GPIO_Port,
     .pin = BUTTON_DOWN_Pin,
     .key = InputKeyDown,
     .inverted = true},
    {.port = BUTTON_RIGHT_GPIO_Port,
     .pin = BUTTON_RIGHT_Pin,
     .key = InputKeyRight,
     .inverted = true},
    {.port = BUTTON_LEFT_GPIO_Port,
     .pin = BUTTON_LEFT_Pin,
     .key = InputKeyLeft,
     .inverted = true},
    {.port = BUTTON_OK_GPIO_Port, .pin = BUTTON_OK_Pin, .key = InputKeyOk, .inverted = false},
    {.port = BUTTON_BACK_GPIO_Port,
     .pin = BUTTON_BACK_Pin,
     .key = InputKeyBack,
     .inverted = true},
};

const size_t input_pins_count = sizeof(input_pins) / sizeof(InputPin);

const GpioPin vibro_gpio = {VIBRO_GPIO_Port, VIBRO_Pin};
const GpioPin ibutton_gpio = {iBTN_GPIO_Port, iBTN_Pin};
const GpioPin cc1101_g0_gpio = {CC1101_G0_GPIO_Port, CC1101_G0_Pin};

const GpioPin gpio_subghz_cs = { .port=CC1101_CS_GPIO_Port, .pin=CC1101_CS_Pin };
const GpioPin gpio_display_cs = { .port=DISPLAY_CS_GPIO_Port, .pin=DISPLAY_CS_Pin };
const GpioPin gpio_display_rst = { .port=DISPLAY_RST_GPIO_Port, .pin=DISPLAY_RST_Pin };
const GpioPin gpio_display_di = { .port=DISPLAY_DI_GPIO_Port, .pin=DISPLAY_DI_Pin };
const GpioPin gpio_sdcard_cs = { .port=SD_CS_GPIO_Port, .pin=SD_CS_Pin };
const GpioPin gpio_nfc_cs = { .port=NFC_CS_GPIO_Port, .pin=NFC_CS_Pin };

const GpioPin gpio_spi_d_miso = { .port=SPI_D_MISO_GPIO_Port, .pin=SPI_D_MISO_Pin };
const GpioPin gpio_spi_d_mosi = { .port=SPI_D_MOSI_GPIO_Port, .pin=SPI_D_MOSI_Pin };
const GpioPin gpio_spi_d_sck = { .port=SPI_D_SCK_GPIO_Port, .pin=SPI_D_SCK_Pin };
const GpioPin gpio_spi_r_miso = { .port=SPI_R_MISO_GPIO_Port, .pin=SPI_R_MISO_Pin };
const GpioPin gpio_spi_r_mosi = { .port=SPI_R_MOSI_GPIO_Port, .pin=SPI_R_MOSI_Pin };
const GpioPin gpio_spi_r_sck = { .port=SPI_R_SCK_GPIO_Port, .pin=SPI_R_SCK_Pin };

// external gpio's
const GpioPin ext_pc0_gpio = {.port = GPIOC, .pin = GPIO_PIN_0};
const GpioPin ext_pc1_gpio = {.port = GPIOC, .pin = GPIO_PIN_1};
const GpioPin ext_pc3_gpio = {.port = GPIOC, .pin = GPIO_PIN_3};
const GpioPin ext_pb2_gpio = {.port = GPIOB, .pin = GPIO_PIN_2};
const GpioPin ext_pb3_gpio = {.port = GPIOB, .pin = GPIO_PIN_3};
const GpioPin ext_pa4_gpio = {.port = GPIOA, .pin = GPIO_PIN_4};
const GpioPin ext_pa6_gpio = {.port = GPIOA, .pin = GPIO_PIN_6};
const GpioPin ext_pa7_gpio = {.port = GPIOA, .pin = GPIO_PIN_7};