/* Implements rudimentary iso7816-3 support for APDU (T=0) */
#include <stdint.h>
#include <string.h>
#include <furi.h>
#include "iso7816_t0_apdu.h"

//reads dataBuffer with dataLen size, translate it into a ISO7816_Command_APDU type
//extra data will be pointed to commandDataBuffer
void iso7816_read_command_apdu(
    struct ISO7816_Command_APDU* command,
    const uint8_t* dataBuffer,
    uint32_t dataLen) {
    UNUSED(dataLen);

    command->CLA = dataBuffer[0];
    command->INS = dataBuffer[1];
    command->P1 = dataBuffer[2];
    command->P2 = dataBuffer[3];
    command->Lc = dataBuffer[4];
}

//data buffer countains the whole APU response (response + trailer (SW1+SW2))
void iso7816_write_response_apdu(
    const struct ISO7816_Response_APDU* response,
    uint8_t* readerToPcDataBlock,
    uint32_t* readerToPcDataBlockLen,
    uint8_t* responseDataBuffer,
    uint32_t responseDataLen) {
    uint32_t responseDataBufferIndex = 0;

    //response body
    if(responseDataLen > 0) {
        while(responseDataBufferIndex < responseDataLen) {
            readerToPcDataBlock[responseDataBufferIndex] =
                responseDataBuffer[responseDataBufferIndex];
            responseDataBufferIndex++;
        }
    }

    //trailer
    readerToPcDataBlock[responseDataBufferIndex] = response->SW1;
    responseDataBufferIndex++;

    readerToPcDataBlock[responseDataBufferIndex] = response->SW2;
    responseDataBufferIndex++;

    *readerToPcDataBlockLen = responseDataBufferIndex;
}