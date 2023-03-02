#pragma once

#include "protocol_dallas_base.h"

typedef enum {
    iButtonProtocolDS1990,
    iButtonProtocolDS1992,
    iButtonProtocolDS1996,
    /* Add new 1-Wire protocols here */

    /* Default catch-all 1-Wire protocol */
    iButtonProtocolDSGeneric,
    iButtonProtocolDSMax,
} iButtonProtocolDallas;

extern const iButtonProtocolDallasBase* ibutton_protocols_dallas[];
