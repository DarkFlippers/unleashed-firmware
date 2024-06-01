#ifndef _ISO7816_CALLBACKS_H_
#define _ISO7816_CALLBACKS_H_

#include <stdint.h>
#include "iso7816_atr.h"
#include "iso7816_t0_apdu.h"

typedef struct {
    void (*iso7816_answer_to_reset)(Iso7816Atr* atr);
    void (*iso7816_process_command)(
        const struct ISO7816_Command_APDU* command,
        struct ISO7816_Response_APDU* response,
        const uint8_t* commandApduDataBuffer,
        uint8_t commandApduDataBufferLen,
        uint8_t* responseApduDataBuffer,
        uint8_t* responseApduDataBufferLen);
} Iso7816Callbacks;

void iso7816_set_callbacks(Iso7816Callbacks* cb);

void iso7816_icc_power_on_callback(uint8_t* atrBuffer, uint32_t* atrlen);
void iso7816_xfr_datablock_callback(
    const uint8_t* dataBlock,
    uint32_t dataBlockLen,
    uint8_t* responseDataBlock,
    uint32_t* responseDataBlockLen);

#endif //_ISO7816_CALLBACKS_H_