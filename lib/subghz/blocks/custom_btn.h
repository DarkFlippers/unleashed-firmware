#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

void subghz_custom_btn_set(uint8_t b);

uint8_t subghz_custom_btn_get();

void subghz_custom_btn_set_original(uint8_t b);

uint8_t subghz_custom_btn_get_original();

void subghz_custom_btn_set_max(uint8_t b);

void subghz_custom_btns_reset();