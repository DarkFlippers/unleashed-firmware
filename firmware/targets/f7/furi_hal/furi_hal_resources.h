#pragma once

#include <furi.h>

#include <stm32wbxx.h>
#include <stm32wbxx_ll_gpio.h>

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
    const bool debug;
} GpioPinRecord;

extern const InputPin input_pins[];
extern const size_t input_pins_count;

extern const GpioPinRecord gpio_pins[];
extern const size_t gpio_pins_count;

extern const GpioPin vibro_gpio;
extern const GpioPin ibutton_gpio;

extern const GpioPin gpio_cc1101_g0;
extern const GpioPin gpio_rf_sw_0;

extern const GpioPin gpio_subghz_cs;
extern const GpioPin gpio_display_cs;
extern const GpioPin gpio_display_rst_n;
extern const GpioPin gpio_display_di;
extern const GpioPin gpio_sdcard_cs;
extern const GpioPin gpio_sdcard_cd;
extern const GpioPin gpio_nfc_cs;

extern const GpioPin gpio_button_up;
extern const GpioPin gpio_button_down;
extern const GpioPin gpio_button_right;
extern const GpioPin gpio_button_left;
extern const GpioPin gpio_button_ok;
extern const GpioPin gpio_button_back;

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

extern const GpioPin gpio_nfc_irq_rfid_pull;
extern const GpioPin gpio_rfid_carrier_out;
extern const GpioPin gpio_rfid_data_in;
extern const GpioPin gpio_rfid_carrier;

extern const GpioPin gpio_infrared_rx;
extern const GpioPin gpio_infrared_tx;

extern const GpioPin gpio_usart_tx;
extern const GpioPin gpio_usart_rx;
extern const GpioPin gpio_i2c_power_sda;
extern const GpioPin gpio_i2c_power_scl;

extern const GpioPin gpio_speaker;

extern const GpioPin periph_power;

extern const GpioPin gpio_usb_dm;
extern const GpioPin gpio_usb_dp;

#define BUTTON_BACK_GPIO_Port GPIOC
#define BUTTON_BACK_Pin LL_GPIO_PIN_13
#define BUTTON_DOWN_GPIO_Port GPIOC
#define BUTTON_DOWN_Pin LL_GPIO_PIN_6
#define BUTTON_LEFT_GPIO_Port GPIOB
#define BUTTON_LEFT_Pin LL_GPIO_PIN_11
#define BUTTON_OK_GPIO_Port GPIOH
#define BUTTON_OK_Pin LL_GPIO_PIN_3
#define BUTTON_RIGHT_GPIO_Port GPIOB
#define BUTTON_RIGHT_Pin LL_GPIO_PIN_12
#define BUTTON_UP_GPIO_Port GPIOB
#define BUTTON_UP_Pin LL_GPIO_PIN_10

#define CC1101_CS_GPIO_Port GPIOD
#define CC1101_CS_Pin LL_GPIO_PIN_0
#define CC1101_G0_GPIO_Port GPIOA
#define CC1101_G0_Pin LL_GPIO_PIN_1

#define DISPLAY_CS_GPIO_Port GPIOC
#define DISPLAY_CS_Pin LL_GPIO_PIN_11
#define DISPLAY_DI_GPIO_Port GPIOB
#define DISPLAY_DI_Pin LL_GPIO_PIN_1
#define DISPLAY_RST_GPIO_Port GPIOB
#define DISPLAY_RST_Pin LL_GPIO_PIN_0

#define IR_RX_GPIO_Port GPIOA
#define IR_RX_Pin LL_GPIO_PIN_0
#define IR_TX_GPIO_Port GPIOB
#define IR_TX_Pin LL_GPIO_PIN_9

#define NFC_CS_GPIO_Port GPIOE
#define NFC_CS_Pin LL_GPIO_PIN_4

#define PA4_GPIO_Port GPIOA
#define PA4_Pin LL_GPIO_PIN_4
#define PA6_GPIO_Port GPIOA
#define PA6_Pin LL_GPIO_PIN_6
#define PA7_GPIO_Port GPIOA
#define PA7_Pin LL_GPIO_PIN_7
#define PB2_GPIO_Port GPIOB
#define PB2_Pin LL_GPIO_PIN_2
#define PB3_GPIO_Port GPIOB
#define PB3_Pin LL_GPIO_PIN_3
#define PC0_GPIO_Port GPIOC
#define PC0_Pin LL_GPIO_PIN_0
#define PC1_GPIO_Port GPIOC
#define PC1_Pin LL_GPIO_PIN_1
#define PC3_GPIO_Port GPIOC
#define PC3_Pin LL_GPIO_PIN_3

#define QUARTZ_32MHZ_IN_GPIO_Port GPIOC
#define QUARTZ_32MHZ_IN_Pin LL_GPIO_PIN_14
#define QUARTZ_32MHZ_OUT_GPIO_Port GPIOC
#define QUARTZ_32MHZ_OUT_Pin LL_GPIO_PIN_15

#define RFID_OUT_GPIO_Port GPIOB
#define RFID_OUT_Pin LL_GPIO_PIN_13
#define RFID_PULL_GPIO_Port GPIOA
#define RFID_PULL_Pin LL_GPIO_PIN_2
#define RFID_RF_IN_GPIO_Port GPIOC
#define RFID_RF_IN_Pin LL_GPIO_PIN_5
#define RFID_CARRIER_GPIO_Port GPIOA
#define RFID_CARRIER_Pin LL_GPIO_PIN_15

#define RF_SW_0_GPIO_Port GPIOC
#define RF_SW_0_Pin LL_GPIO_PIN_4

#define SD_CD_GPIO_Port GPIOC
#define SD_CD_Pin LL_GPIO_PIN_10
#define SD_CS_GPIO_Port GPIOC
#define SD_CS_Pin LL_GPIO_PIN_12

#define SPEAKER_GPIO_Port GPIOB
#define SPEAKER_Pin LL_GPIO_PIN_8

#define VIBRO_GPIO_Port GPIOA
#define VIBRO_Pin LL_GPIO_PIN_8

#define iBTN_GPIO_Port GPIOB
#define iBTN_Pin LL_GPIO_PIN_14

#define USART1_TX_Pin LL_GPIO_PIN_6
#define USART1_TX_Port GPIOB
#define USART1_RX_Pin LL_GPIO_PIN_7
#define USART1_RX_Port GPIOB

#define SPI_D_MISO_GPIO_Port GPIOC
#define SPI_D_MISO_Pin LL_GPIO_PIN_2
#define SPI_D_MOSI_GPIO_Port GPIOB
#define SPI_D_MOSI_Pin LL_GPIO_PIN_15
#define SPI_D_SCK_GPIO_Port GPIOD
#define SPI_D_SCK_Pin LL_GPIO_PIN_1

#define SPI_R_MISO_GPIO_Port GPIOB
#define SPI_R_MISO_Pin LL_GPIO_PIN_4
#define SPI_R_MOSI_GPIO_Port GPIOB
#define SPI_R_MOSI_Pin LL_GPIO_PIN_5
#define SPI_R_SCK_GPIO_Port GPIOA
#define SPI_R_SCK_Pin LL_GPIO_PIN_5

#define NFC_IRQ_Pin RFID_PULL_Pin
#define NFC_IRQ_GPIO_Port RFID_PULL_GPIO_Port

void furi_hal_resources_init_early();

void furi_hal_resources_deinit_early();

void furi_hal_resources_init();

/**
 * Get a corresponding external connector pin number for a gpio
 * @param gpio GpioPin
 * @return pin number or -1 if gpio is not on the external connector
 */
int32_t furi_hal_resources_get_ext_pin_number(const GpioPin* gpio);

#ifdef __cplusplus
}
#endif
