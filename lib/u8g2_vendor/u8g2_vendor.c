#include "u8g2_support.h"
#include "main.h"
#include "cmsis_os.h"
#include "gpio.h"

#include <stdio.h>

extern SPI_HandleTypeDef hspi1;

// #define DEBUG 1

uint8_t u8g2_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch(msg){
        //Initialize SPI peripheral
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            /* HAL initialization contains all what we need so we can skip this part. */
        break;

        //Function which implements a delay, arg_int contains the amount of ms
        case U8X8_MSG_DELAY_MILLI:
            osDelay(arg_int);
        break;

        //Function which delays 10us
        case U8X8_MSG_DELAY_10MICRO:
            delay_us(10);
        break;

        //Function which delays 100ns
        case U8X8_MSG_DELAY_100NANO:
            asm("nop");
        break;

        //Function to define the logic level of the RESET line
        case U8X8_MSG_GPIO_RESET:
            #ifdef DEBUG
                printf("[u8g2] rst %d\n", arg_int);
            #endif

            HAL_GPIO_WritePin(DISPLAY_RST_GPIO_Port, DISPLAY_RST_Pin, arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        break;

        default:
            #ifdef DEBUG
                printf("[u8g2] unknown io %d\n", msg);
            #endif

            return 0; //A message was received which is not implemented, return 0 to indicate an error
    }

    return 1; // command processed successfully.
}

uint8_t u8x8_hw_spi_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr){
    switch (msg) {
        case U8X8_MSG_BYTE_SEND:
            #ifdef DEBUG
                printf("[u8g2] send %d bytes %02X\n", arg_int, ((uint8_t*)arg_ptr)[0]);
            #endif

            HAL_SPI_Transmit(&hspi1, (uint8_t *)arg_ptr, arg_int, 10000);
        break;

        case U8X8_MSG_BYTE_SET_DC:
            #ifdef DEBUG
                printf("[u8g2] dc %d\n", arg_int);
            #endif

            HAL_GPIO_WritePin(DISPLAY_DI_GPIO_Port, DISPLAY_DI_Pin, arg_int ? GPIO_PIN_SET : GPIO_PIN_RESET);
        break;

        case U8X8_MSG_BYTE_INIT:
            #ifdef DEBUG
                printf("[u8g2] init\n");
            #endif
            HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_RESET);
        break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            #ifdef DEBUG
                printf("[u8g2] start\n");
            #endif

            HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_RESET);
            asm("nop");
        break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            #ifdef DEBUG
                printf("[u8g2] end\n");
            #endif

            asm("nop");
            HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_SET);
        break;

        default:
            #ifdef DEBUG
                printf("[u8g2] unknown xfer %d\n", msg);
            #endif

            return 0;
    }

    return 1;
}
