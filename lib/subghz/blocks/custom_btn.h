#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Default btn ID
#define SUBGHZ_CUSTOM_BTN_OK (0U)
#define SUBGHZ_CUSTOM_BTN_UP (1U)
#define SUBGHZ_CUSTOM_BTN_DOWN (2U)
#define SUBGHZ_CUSTOM_BTN_LEFT (3U)
#define SUBGHZ_CUSTOM_BTN_RIGHT (4U)

bool subghz_custom_btn_set(uint8_t btn_id);

uint8_t subghz_custom_btn_get();

uint8_t subghz_custom_btn_get_original();

void subghz_custom_btns_reset();

bool subghz_custom_btn_is_allowed();

#ifdef __cplusplus
}
#endif