/* Implements rudimentary iso7816-3 support for APDU (T=0) */
#include <stdint.h>
#include <string.h>
#include <furi.h>
#include <furi_hal.h>
#include "iso7816_t0_apdu.h"

//reads dataBuffer with dataLen size, translate it into a ISO7816_Command_APDU type
//extra data will be pointed to commandDataBuffer
uint8_t iso7816_read_command_apdu(
    ISO7816_Command_APDU* command,
    const uint8_t* dataBuffer,
    uint32_t dataLen) {
    command->CLA = dataBuffer[0];
    command->INS = dataBuffer[1];
    command->P1 = dataBuffer[2];
    command->P2 = dataBuffer[3];

    if(dataLen == 4) {
        command->Lc = 0;
        command->Le = 0;
        command->LePresent = false;

        return ISO7816_READ_COMMAND_APDU_OK;
    } else if(dataLen == 5) {
        //short le

        command->Lc = 0;
        command->Le = dataBuffer[4];
        command->LePresent = true;

        return ISO7816_READ_COMMAND_APDU_OK;
    } else if(dataLen > 5 && dataBuffer[4] != 0x00) {
        //short lc

        command->Lc = dataBuffer[4];
        if(command->Lc > 0 && command->Lc < CCID_SHORT_APDU_SIZE) { //-V560
            memcpy(command->Data, &dataBuffer[5], command->Lc);

            //does it have a short le too?
            if(dataLen == (uint32_t)(command->Lc + 5)) {
                command->Le = 0;
                command->LePresent = false;
                return ISO7816_READ_COMMAND_APDU_OK;
            } else if(dataLen == (uint32_t)(command->Lc + 6)) {
                command->Le = dataBuffer[dataLen - 1];
                command->LePresent = true;

                return ISO7816_READ_COMMAND_APDU_OK;
            } else {
                return ISO7816_READ_COMMAND_APDU_ERROR_WRONG_LENGTH;
            }
        } else {
            return ISO7816_READ_COMMAND_APDU_ERROR_WRONG_LENGTH;
        }
    } else {
        return ISO7816_READ_COMMAND_APDU_ERROR_WRONG_LENGTH;
    }
}

//data buffer contains the whole APU response (response + trailer (SW1+SW2))
void iso7816_write_response_apdu(
    const ISO7816_Response_APDU* response,
    uint8_t* readerToPcDataBlock,
    uint32_t* readerToPcDataBlockLen) {
    uint32_t responseDataBufferIndex = 0;

    //response body
    if(response->DataLen > 0) {
        while(responseDataBufferIndex < response->DataLen) {
            readerToPcDataBlock[responseDataBufferIndex] = response->Data[responseDataBufferIndex];
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
