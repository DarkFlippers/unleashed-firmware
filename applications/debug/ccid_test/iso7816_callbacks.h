#pragma once

#include <stdint.h>
#include "iso7816_atr.h"
#include "iso7816_t0_apdu.h"

typedef struct {
    void (*iso7816_answer_to_reset)(Iso7816Atr* atr);
    void (*iso7816_process_command)(
        const ISO7816_Command_APDU* command,
        ISO7816_Response_APDU* response);
} Iso7816Callbacks;

void iso7816_set_callbacks(Iso7816Callbacks* cb);

void iso7816_icc_power_on_callback(uint8_t* atrBuffer, uint32_t* atrlen);
void iso7816_xfr_datablock_callback(
    const uint8_t* dataBlock,
    uint32_t dataBlockLen,
    uint8_t* responseDataBlock,
    uint32_t* responseDataBlockLen);
