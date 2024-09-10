#pragma once

#include <stdint.h>
#include "iso7816_atr.h"
#include "iso7816_t0_apdu.h"

typedef struct {
    void (*iso7816_answer_to_reset)(Iso7816Atr* atr);
    void (*iso7816_process_command)(
        const ISO7816_Command_APDU* command,
        ISO7816_Response_APDU* response);
} Iso7816Handler;

Iso7816Handler* iso7816_handler_alloc();

void iso7816_handler_free(Iso7816Handler* handler);
void iso7816_handler_set_usb_ccid_callbacks();
void iso7816_handler_reset_usb_ccid_callbacks();
