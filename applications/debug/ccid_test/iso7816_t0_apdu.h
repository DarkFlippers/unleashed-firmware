#pragma once

#include <stdint.h>
#include "iso7816_atr.h"
#include "core/common_defines.h"

#define ISO7816_READ_COMMAND_APDU_OK                 0
#define ISO7816_READ_COMMAND_APDU_ERROR_WRONG_LE     1
#define ISO7816_READ_COMMAND_APDU_ERROR_WRONG_LENGTH 2

typedef struct {
    //header
    uint8_t CLA;
    uint8_t INS;
    uint8_t P1;
    uint8_t P2;

    //body
    uint16_t Lc; //data length
    uint16_t Le; //maximum response data length expected by client

    //Le can have value of 0x00, which actually meand 0x100 = 256
    bool LePresent;
    uint8_t Data[0];
} FURI_PACKED ISO7816_Command_APDU;

typedef struct {
    uint8_t SW1;
    uint8_t SW2;
    uint16_t DataLen;
    uint8_t Data[0];
} FURI_PACKED ISO7816_Response_APDU;

void iso7816_answer_to_reset(Iso7816Atr* atr);
uint8_t iso7816_read_command_apdu(
    ISO7816_Command_APDU* command,
    const uint8_t* pcToReaderDataBlock,
    uint32_t pcToReaderDataBlockLen);
void iso7816_write_response_apdu(
    const ISO7816_Response_APDU* response,
    uint8_t* readerToPcDataBlock,
    uint32_t* readerToPcDataBlockLen);
