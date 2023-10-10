/* Implements rudimentary iso7816-3 support for APDU (T=0) */
#include <stdint.h>
#include <string.h>
#include <furi.h>
#include "iso7816_t0_apdu.h"

void iso7816_answer_to_reset(uint8_t* dataBuffer, uint32_t* atrlen) {
    //minimum valid ATR: https://smartcard-atr.apdu.fr/parse?ATR=3B+00
    uint8_t AtrBuffer[2] = {
        0x3B, //TS (direct convention)
        0x00 // T0 (Y(1): b0000, K: 0 (historical bytes))
    };
    *atrlen = 2;

    memcpy(dataBuffer, AtrBuffer, sizeof(uint8_t) * (*atrlen));
}

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

void iso7816_write_response_apdu(
    const struct ISO7816_Response_APDU* response,
    uint8_t* dataBuffer,
    uint32_t* dataLen) {
    dataBuffer[0] = response->SW1;
    dataBuffer[1] = response->SW2;
    *dataLen = 2;
}