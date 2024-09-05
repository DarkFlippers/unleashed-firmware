/* Implements rudimentary iso7816-3 support for APDU (T=0) */
#include <stdint.h>
#include <string.h>
#include "iso7816_t0_apdu.h"

//reads pc_to_reader_datablock_len with pc_to_reader_datablock_len size, translate it into a ISO7816_Command_APDU type
//extra data will be pointed to commandDataBuffer
uint8_t iso7816_read_command_apdu(
    ISO7816_Command_APDU* command,
    const uint8_t* pc_to_reader_datablock,
    uint32_t pc_to_reader_datablock_len,
    uint32_t max_apdu_size) {
    command->CLA = pc_to_reader_datablock[0];
    command->INS = pc_to_reader_datablock[1];
    command->P1 = pc_to_reader_datablock[2];
    command->P2 = pc_to_reader_datablock[3];

    if(pc_to_reader_datablock_len == 4) {
        command->Lc = 0;
        command->Le = 0;
        command->LePresent = false;

        return ISO7816_READ_COMMAND_APDU_OK;
    } else if(pc_to_reader_datablock_len == 5) {
        //short le

        command->Lc = 0;
        command->Le = pc_to_reader_datablock[4];
        command->LePresent = true;

        return ISO7816_READ_COMMAND_APDU_OK;
    } else if(pc_to_reader_datablock_len > 5 && pc_to_reader_datablock[4] != 0x00) {
        //short lc

        command->Lc = pc_to_reader_datablock[4];
        if(command->Lc > 0 && command->Lc < max_apdu_size) { //-V560
            memcpy(command->Data, &pc_to_reader_datablock[5], command->Lc);

            //does it have a short le too?
            if(pc_to_reader_datablock_len == (uint32_t)(command->Lc + 5)) {
                command->Le = 0;
                command->LePresent = false;
                return ISO7816_READ_COMMAND_APDU_OK;
            } else if(pc_to_reader_datablock_len == (uint32_t)(command->Lc + 6)) {
                command->Le = pc_to_reader_datablock[pc_to_reader_datablock_len - 1];
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
    uint8_t* reader_to_pc_datablock,
    uint32_t* reader_to_pc_datablock_len) {
    uint32_t responseDataBufferIndex = 0;

    //response body
    if(response->DataLen > 0) {
        while(responseDataBufferIndex < response->DataLen) {
            reader_to_pc_datablock[responseDataBufferIndex] =
                response->Data[responseDataBufferIndex];
            responseDataBufferIndex++;
        }
    }

    //trailer
    reader_to_pc_datablock[responseDataBufferIndex] = response->SW1;
    responseDataBufferIndex++;

    reader_to_pc_datablock[responseDataBufferIndex] = response->SW2;
    responseDataBufferIndex++;

    *reader_to_pc_datablock_len = responseDataBufferIndex;
}
