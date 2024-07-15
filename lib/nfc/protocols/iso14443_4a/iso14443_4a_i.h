#pragma once

#include "iso14443_4a.h"

#define ISO14443_4A_CMD_READ_ATS (0xE0)

// ATS bit definitions
#define ISO14443_4A_ATS_T0_TA1 (1U << 4)
#define ISO14443_4A_ATS_T0_TB1 (1U << 5)
#define ISO14443_4A_ATS_T0_TC1 (1U << 6)

#define ISO14443_4A_ATS_TA1_BOTH_106KBIT         (0U << 0)
#define ISO14443_4A_ATS_TA1_PCD_TO_PICC_212KBIT  (1U << 0)
#define ISO14443_4A_ATS_TA1_PCD_TO_PICC_424KBIT  (1U << 1)
#define ISO14443_4A_ATS_TA1_PCD_TO_PICC_848KBIT  (1U << 2)
#define ISO14443_4A_ATS_TA1_PICC_TO_PCD_212KBIT  (1U << 4)
#define ISO14443_4A_ATS_TA1_PICC_TO_PCD_424KBIT  (1U << 5)
#define ISO14443_4A_ATS_TA1_PICC_TO_PCD_848KBIT  (1U << 6)
#define ISO14443_4A_ATS_TA1_BOTH_SAME_COMPULSORY (1U << 7)

#define ISO14443_4A_ATS_TC1_NAD (1U << 0)
#define ISO14443_4A_ATS_TC1_CID (1U << 1)

bool iso14443_4a_ats_parse(Iso14443_4aAtsData* data, const BitBuffer* buf);

Iso14443_4aError iso14443_4a_process_error(Iso14443_3aError error);
