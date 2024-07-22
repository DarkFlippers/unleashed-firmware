#pragma once

#include "iso14443_3b.h"

#define ISO14443_3B_UID_SIZE      (4U)
#define ISO14443_3B_APP_DATA_SIZE (4U)

#define ISO14443_3B_GUARD_TIME_US    (5000U)
#define ISO14443_3B_FDT_POLL_FC      (9000U)
#define ISO14443_3B_POLL_POLL_MIN_US (1280U)

#define ISO14443_3B_BIT_RATE_BOTH_106KBIT         (0U << 0)
#define ISO14443_3B_BIT_RATE_PCD_TO_PICC_212KBIT  (1U << 0)
#define ISO14443_3B_BIT_RATE_PCD_TO_PICC_424KBIT  (1U << 1)
#define ISO14443_3B_BIT_RATE_PCD_TO_PICC_848KBIT  (1U << 2)
#define ISO14443_3B_BIT_RATE_PICC_TO_PCD_212KBIT  (1U << 4)
#define ISO14443_3B_BIT_RATE_PICC_TO_PCD_424KBIT  (1U << 5)
#define ISO14443_3B_BIT_RATE_PICC_TO_PCD_848KBIT  (1U << 6)
#define ISO14443_3B_BIT_RATE_BOTH_SAME_COMPULSORY (1U << 7)

#define ISO14443_3B_FRAME_OPTION_NAD (1U << 1)
#define ISO14443_3B_FRAME_OPTION_CID (1U << 0)

typedef struct {
    uint8_t bit_rate_capability;
    uint8_t protocol_type  : 4;
    uint8_t max_frame_size : 4;
    uint8_t fo             : 2;
    uint8_t adc            : 2;
    uint8_t fwi            : 4;
} Iso14443_3bProtocolInfo;

struct Iso14443_3bData {
    uint8_t uid[ISO14443_3B_UID_SIZE];
    uint8_t app_data[ISO14443_3B_APP_DATA_SIZE];
    Iso14443_3bProtocolInfo protocol_info;
};
