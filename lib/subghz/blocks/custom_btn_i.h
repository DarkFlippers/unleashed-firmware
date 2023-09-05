#pragma once

#include "custom_btn.h"

#define PROG_MODE_OFF (0U)
#define PROG_MODE_KEELOQ_BFT (1U)
#define PROG_MODE_KEELOQ_APRIMATIC (2U)

typedef uint8_t ProgMode;

void subghz_custom_btn_set_original(uint8_t btn_code);

void subghz_custom_btn_set_max(uint8_t b);

void subghz_custom_btn_set_prog_mode(ProgMode prog_mode);

ProgMode subghz_custom_btn_get_prog_mode();
