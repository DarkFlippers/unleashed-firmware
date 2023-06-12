#pragma once

#include "u8g2.h"
#include <stdbool.h>

uint8_t u8g2_gpio_and_delay_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

uint8_t u8x8_hw_spi_stm32(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr);

void u8g2_Setup_st756x_flipper(
    u8g2_t* u8g2,
    const u8g2_cb_t* rotation,
    u8x8_msg_cb byte_cb,
    u8x8_msg_cb gpio_and_delay_cb);

void u8x8_d_st756x_init(u8x8_t* u8x8, uint8_t contrast, uint8_t regulation_ratio, bool bias);

void u8x8_d_st756x_set_contrast(u8x8_t* u8x8, int8_t contrast_offset);
