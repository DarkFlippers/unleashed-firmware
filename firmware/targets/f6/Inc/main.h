#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32wbxx_hal.h"

void Error_Handler(void);

#define BUTTON_BACK_EXTI_IRQn EXTI15_10_IRQn
#define BUTTON_BACK_GPIO_Port GPIOC
#define BUTTON_BACK_Pin GPIO_PIN_13
#define BUTTON_DOWN_EXTI_IRQn EXTI6_IRQn
#define BUTTON_DOWN_GPIO_Port GPIOC
#define BUTTON_DOWN_Pin GPIO_PIN_6
#define BUTTON_LEFT_EXTI_IRQn EXTI15_10_IRQn
#define BUTTON_LEFT_GPIO_Port GPIOB
#define BUTTON_LEFT_Pin GPIO_PIN_11
#define BUTTON_OK_EXTI_IRQn EXTI3_IRQn
#define BUTTON_OK_GPIO_Port GPIOH
#define BUTTON_OK_Pin GPIO_PIN_3
#define BUTTON_RIGHT_EXTI_IRQn EXTI15_10_IRQn
#define BUTTON_RIGHT_GPIO_Port GPIOB
#define BUTTON_RIGHT_Pin GPIO_PIN_12
#define BUTTON_UP_EXTI_IRQn EXTI15_10_IRQn
#define BUTTON_UP_GPIO_Port GPIOB
#define BUTTON_UP_Pin GPIO_PIN_10

#define CC1101_CS_GPIO_Port GPIOD
#define CC1101_CS_Pin GPIO_PIN_0
#define CC1101_G0_GPIO_Port GPIOA
#define CC1101_G0_Pin GPIO_PIN_1

#define DISPLAY_CS_GPIO_Port GPIOC
#define DISPLAY_CS_Pin GPIO_PIN_11
#define DISPLAY_DI_GPIO_Port GPIOB
#define DISPLAY_DI_Pin GPIO_PIN_1
#define DISPLAY_RST_GPIO_Port GPIOB
#define DISPLAY_RST_Pin GPIO_PIN_0

#define IR_RX_GPIO_Port GPIOA
#define IR_RX_Pin GPIO_PIN_0
#define IR_TX_GPIO_Port GPIOB
#define IR_TX_Pin GPIO_PIN_9

#define NFC_CS_GPIO_Port GPIOE
#define NFC_CS_Pin GPIO_PIN_4

#define PA4_GPIO_Port GPIOA
#define PA4_Pin GPIO_PIN_4
#define PA6_GPIO_Port GPIOA
#define PA6_Pin GPIO_PIN_6
#define PA7_GPIO_Port GPIOA
#define PA7_Pin GPIO_PIN_7
#define PB2_GPIO_Port GPIOB
#define PB2_Pin GPIO_PIN_2
#define PB3_GPIO_Port GPIOB
#define PB3_Pin GPIO_PIN_3
#define PC0_GPIO_Port GPIOC
#define PC0_Pin GPIO_PIN_0
#define PC1_GPIO_Port GPIOC
#define PC1_Pin GPIO_PIN_1
#define PC3_GPIO_Port GPIOC
#define PC3_Pin GPIO_PIN_3

#define PERIPH_POWER_GPIO_Port GPIOA
#define PERIPH_POWER_Pin GPIO_PIN_3

#define QUARTZ_32MHZ_IN_GPIO_Port GPIOC
#define QUARTZ_32MHZ_IN_Pin GPIO_PIN_14
#define QUARTZ_32MHZ_OUT_GPIO_Port GPIOC
#define QUARTZ_32MHZ_OUT_Pin GPIO_PIN_15

#define RFID_OUT_GPIO_Port GPIOB
#define RFID_OUT_Pin GPIO_PIN_13
#define RFID_PULL_GPIO_Port GPIOA
#define RFID_PULL_Pin GPIO_PIN_2
#define RFID_RF_IN_GPIO_Port GPIOC
#define RFID_RF_IN_Pin GPIO_PIN_5
#define RFID_TUNE_GPIO_Port GPIOA
#define RFID_TUNE_Pin GPIO_PIN_8

#define RF_SW_0_GPIO_Port GPIOC
#define RF_SW_0_Pin GPIO_PIN_4

#define SD_CD_GPIO_Port GPIOC
#define SD_CD_Pin GPIO_PIN_10
#define SD_CS_GPIO_Port GPIOC
#define SD_CS_Pin GPIO_PIN_12

#define SPEAKER_GPIO_Port GPIOB
#define SPEAKER_Pin GPIO_PIN_8

#define VIBRO_GPIO_Port GPIOA
#define VIBRO_Pin GPIO_PIN_15

#define iBTN_GPIO_Port GPIOB
#define iBTN_Pin GPIO_PIN_14

#define USART1_TX_Pin GPIO_PIN_6
#define USART1_TX_Port GPIOB
#define USART1_RX_Pin GPIO_PIN_7
#define USART1_RX_Port GPIOB

#define SPI_D_MISO_GPIO_Port GPIOC
#define SPI_D_MISO_Pin GPIO_PIN_2
#define SPI_D_MOSI_GPIO_Port GPIOB
#define SPI_D_MOSI_Pin GPIO_PIN_15
#define SPI_D_SCK_GPIO_Port GPIOD
#define SPI_D_SCK_Pin GPIO_PIN_1

#define SPI_R_MISO_GPIO_Port GPIOB
#define SPI_R_MISO_Pin GPIO_PIN_4
#define SPI_R_MOSI_GPIO_Port GPIOB
#define SPI_R_MOSI_Pin GPIO_PIN_5
#define SPI_R_SCK_GPIO_Port GPIOA
#define SPI_R_SCK_Pin GPIO_PIN_5

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim16;

#define TIM_A htim1
#define TIM_B htim2
#define TIM_C htim16

#define SPEAKER_TIM htim16
#define SPEAKER_CH TIM_CHANNEL_1

#define LFRFID_TIM htim1
#define LFRFID_CH TIM_CHANNEL_1

#define IRDA_TX_TIM htim1
#define IRDA_TX_CH TIM_CHANNEL_3

// only for reference
// IRDA RX timer dont exist in F2
// and timer need more data to init (NVIC IRQn to set priority)
#define IRDA_RX_TIM htim2
#define IRDA_RX_FALLING_CH TIM_CHANNEL_1
#define IRDA_RX_RISING_CH TIM_CHANNEL_2

#define NFC_IRQ_Pin RFID_PULL_Pin
#define NFC_IRQ_GPIO_Port RFID_PULL_GPIO_Port

#ifdef __cplusplus
}
#endif
