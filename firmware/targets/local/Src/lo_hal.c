/*
Flipper devices inc.

Dummy hal for local fw build
*/

#include <stdio.h>
#include "main.h"
#include <unistd.h>

UART_HandleTypeDef DEBUG_UART = 0;

uint16_t
HAL_UART_Transmit(UART_HandleTypeDef* handle, uint8_t* bufer, uint16_t size, uint32_t wait_ms) {
    uint16_t res = write(1, (const char*)bufer, size);
    return res;
}

uint8_t BSP_SD_Init() {
    return 0;
}
