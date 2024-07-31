#pragma once

#include <stdint.h>
#include "iso7816_atr.h"
#include "iso7816_t0_apdu.h"

typedef struct {
    CcidCallbacks ccid_callbacks;
    void (*iso7816_answer_to_reset)(Iso7816Atr* atr);
    void (*iso7816_process_command)(
        const ISO7816_Command_APDU* command,
        ISO7816_Response_APDU* response);

    uint8_t command_apdu_buffer[sizeof(ISO7816_Command_APDU) + CCID_SHORT_APDU_SIZE];
    uint8_t response_apdu_buffer[sizeof(ISO7816_Response_APDU) + CCID_SHORT_APDU_SIZE];
} Iso7816Handler;

Iso7816Handler* iso7816_handler_alloc();
