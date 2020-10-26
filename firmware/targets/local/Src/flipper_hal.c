/*
Flipper devices inc.

GPIO and HAL implementations
*/

#include "main.h"
#include "flipper_hal.h"
#include <stdio.h>

HAL_StatusTypeDef
HAL_SPI_Transmit(SPI_HandleTypeDef* hspi, uint8_t* pData, uint16_t size, uint32_t Timeout) {
    printf("[SPI] write %d to %s: ", size, *hspi);
    for(size_t i = 0; i < size; i++) {
        printf("%02X ", pData[i]);
    }
    printf("\n");

    return 0;
}

SPI_HandleTypeDef hspi1 = "spi1";
