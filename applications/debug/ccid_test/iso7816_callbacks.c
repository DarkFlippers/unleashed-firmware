// transforms low level calls such as XFRCallback or ICC Power on to a structured one
// an application can register these calls and listen for the callbacks defined in Iso7816Callbacks

#include <stdint.h>
#include <stddef.h>
#include <furi.h>
#include <furi_hal.h>

#include "iso7816_t0_apdu.h"
#include "iso7816_atr.h"
#include "iso7816_callbacks.h"
#include "iso7816_response.h"

static Iso7816Callbacks* callbacks = NULL;

static uint8_t commandApduBuffer[sizeof(ISO7816_Command_APDU) + CCID_SHORT_APDU_SIZE];
static uint8_t responseApduBuffer[sizeof(ISO7816_Response_APDU) + CCID_SHORT_APDU_SIZE];

void iso7816_set_callbacks(Iso7816Callbacks* cb) {
    callbacks = cb;
}

void iso7816_icc_power_on_callback(uint8_t* atrBuffer, uint32_t* atrlen) {
    Iso7816Atr atr;
    callbacks->iso7816_answer_to_reset(&atr);

    furi_assert(atr.T0 == 0x00);

    uint8_t AtrBuffer[2] = {atr.TS, atr.T0};

    *atrlen = 2;

    memcpy(atrBuffer, AtrBuffer, sizeof(uint8_t) * (*atrlen));
}

//dataBlock points to the buffer
//dataBlockLen tells reader how nany bytes should be read
void iso7816_xfr_datablock_callback(
    const uint8_t* pcToReaderDataBlock,
    uint32_t pcToReaderDataBlockLen,
    uint8_t* readerToPcDataBlock,
    uint32_t* readerToPcDataBlockLen) {
    ISO7816_Response_APDU* responseAPDU = (ISO7816_Response_APDU*)&responseApduBuffer;

    if(callbacks != NULL) {
        ISO7816_Command_APDU* commandAPDU = (ISO7816_Command_APDU*)&commandApduBuffer;

        uint8_t result =
            iso7816_read_command_apdu(commandAPDU, pcToReaderDataBlock, pcToReaderDataBlockLen);

        if(result == ISO7816_READ_COMMAND_APDU_OK) {
            callbacks->iso7816_process_command(commandAPDU, responseAPDU);

            furi_assert(responseAPDU->DataLen < CCID_SHORT_APDU_SIZE);
        } else if(result == ISO7816_READ_COMMAND_APDU_ERROR_WRONG_LE) {
            iso7816_set_response(responseAPDU, ISO7816_RESPONSE_WRONG_LE);
        } else if(result == ISO7816_READ_COMMAND_APDU_ERROR_WRONG_LENGTH) {
            iso7816_set_response(responseAPDU, ISO7816_RESPONSE_WRONG_LENGTH);
        }
    } else {
        iso7816_set_response(responseAPDU, ISO7816_RESPONSE_INTERNAL_EXCEPTION);
    }

    iso7816_write_response_apdu(responseAPDU, readerToPcDataBlock, readerToPcDataBlockLen);
}
