#pragma once

#include <stdint.h>

typedef enum {
    MifareTypeUnknown,
    MifareTypeUltralight,
    MifareTypeClassic,
    MifareTypeDesfire,
} MifareType;

MifareType mifare_common_get_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK);
