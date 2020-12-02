/*
Flipper devices inc.

GPIO and HAL implementations
*/

#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPIOA "PA"
#define GPIOB "PB"
#define GPIOC "PC"
#define GPIOD "PD"
#define GPIOE "PE"

#define GPIO_PIN_0 0
#define GPIO_PIN_1 1
#define GPIO_PIN_2 2
#define GPIO_PIN_3 3
#define GPIO_PIN_4 4
#define GPIO_PIN_5 5
#define GPIO_PIN_6 6
#define GPIO_PIN_7 7
#define GPIO_PIN_8 8
#define GPIO_PIN_9 9
#define GPIO_PIN_10 10
#define GPIO_PIN_11 11
#define GPIO_PIN_12 12
#define GPIO_PIN_13 13
#define GPIO_PIN_14 14
#define GPIO_PIN_15 15

#define DISPLAY_RST_GPIO_Port "DISPLAY RST"
#define DISPLAY_DI_Pin 0

#define DISPLAY_DI_GPIO_Port "DISPLAY DI"
#define DISPLAY_RST_Pin 0

#define DISPLAY_CS_GPIO_Port "DISPLAY CS"
#define DISPLAY_CS_Pin 0

#define DISPLAY_BACKLIGHT_GPIO_Port "BACKLIGHT"
#define DISPLAY_BACKLIGHT_Pin 0

typedef const char* SPI_HandleTypeDef;

typedef uint32_t HAL_StatusTypeDef;

HAL_StatusTypeDef
HAL_SPI_Transmit(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t Size, uint32_t Timeout);

#ifdef __cplusplus
}
#endif
