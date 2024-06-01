// transforms low level calls such as XFRCallback or ICC Power on to a structured one
// an application can register these calls and listen for the callbacks defined in Iso7816Callbacks

#include "iso7816_t0_apdu.h"
#include "iso7816_atr.h"
#include "iso7816_callbacks.h"
#include <stdint.h>
#include <stddef.h>
#include <furi.h>

#define ISO7816_RESPONSE_BUFFER_SIZE 255

static Iso7816Callbacks* callbacks = NULL;

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
    struct ISO7816_Response_APDU responseAPDU;
    uint8_t responseApduDataBuffer[ISO7816_RESPONSE_BUFFER_SIZE];
    uint8_t responseApduDataBufferLen = 0;

    if(callbacks != NULL) {
        struct ISO7816_Command_APDU commandAPDU;

        const uint8_t* commandApduDataBuffer = NULL;
        uint8_t commandApduDataBufferLen = 0;

        iso7816_read_command_apdu(&commandAPDU, pcToReaderDataBlock, pcToReaderDataBlockLen);

        if(commandAPDU.Lc > 0) {
            commandApduDataBufferLen = commandAPDU.Lc;
            commandApduDataBuffer = &pcToReaderDataBlock[5];
        }

        callbacks->iso7816_process_command(
            &commandAPDU,
            &responseAPDU,
            commandApduDataBuffer,
            commandApduDataBufferLen,
            responseApduDataBuffer,
            &responseApduDataBufferLen);

    } else {
        //class not supported
        responseAPDU.SW1 = 0x6E;
        responseAPDU.SW2 = 0x00;
    }

    iso7816_write_response_apdu(
        &responseAPDU,
        readerToPcDataBlock,
        readerToPcDataBlockLen,
        responseApduDataBuffer,
        responseApduDataBufferLen);
}
