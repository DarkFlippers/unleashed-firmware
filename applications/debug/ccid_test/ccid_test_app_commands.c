#include "iso7816/iso7816_t0_apdu.h"
#include "iso7816/iso7816_response.h"

//Instruction 1: returns an OK response unconditionally
//APDU example: 0x01:0x01:0x00:0x00
//response: SW1=0x90, SW2=0x00
void handle_instruction_01(ISO7816_Response_APDU* response_apdu) {
    response_apdu->DataLen = 0;
    iso7816_set_response(response_apdu, ISO7816_RESPONSE_OK);
}

//Instruction 2: expect command with no body, replies wit with a body with two bytes
//APDU example: 0x01:0x02:0x00:0x00:0x02
//response: 'bc' (0x62, 0x63) SW1=0x90, SW2=0x00
void handle_instruction_02(
    uint8_t p1,
    uint8_t p2,
    uint16_t lc,
    uint16_t le,
    ISO7816_Response_APDU* response_apdu) {
    if(p1 == 0 && p2 == 0 && lc == 0 && le >= 2) {
        response_apdu->Data[0] = 0x62;
        response_apdu->Data[1] = 0x63;

        response_apdu->DataLen = 2;

        iso7816_set_response(response_apdu, ISO7816_RESPONSE_OK);
    } else if(p1 != 0 || p2 != 0) {
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_WRONG_PARAMETERS_P1_P2);
    } else {
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_WRONG_LENGTH);
    }
}

//Instruction 3: sends a command with a body with two bytes, receives a response with no bytes
//APDU example: 0x01:0x03:0x00:0x00:0x02:CA:FE
//response SW1=0x90, SW2=0x00
void handle_instruction_03(
    uint8_t p1,
    uint8_t p2,
    uint16_t lc,
    ISO7816_Response_APDU* response_apdu) {
    if(p1 == 0 && p2 == 0 && lc == 2) {
        response_apdu->DataLen = 0;
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_OK);
    } else if(p1 != 0 || p2 != 0) {
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_WRONG_PARAMETERS_P1_P2);
    } else {
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_WRONG_LENGTH);
    }
}

//instruction 4: sends a command with a body with 'n' bytes, receives a response with 'n' bytes
//APDU example: 0x01:0x04:0x00:0x00:0x04:0x01:0x02:0x03:0x04:0x04
//receives (0x01, 0x02, 0x03, 0x04) SW1=0x90, SW2=0x00
void handle_instruction_04(
    uint8_t p1,
    uint8_t p2,
    uint16_t lc,
    uint16_t le,
    const uint8_t* command_apdu_data_buffer,
    ISO7816_Response_APDU* response_apdu) {
    if(p1 == 0 && p2 == 0 && lc > 0 && le > 0 && le >= lc) {
        for(uint16_t i = 0; i < lc; i++) {
            response_apdu->Data[i] = command_apdu_data_buffer[i];
        }

        response_apdu->DataLen = lc;

        iso7816_set_response(response_apdu, ISO7816_RESPONSE_OK);
    } else if(p1 != 0 || p2 != 0) {
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_WRONG_PARAMETERS_P1_P2);
    } else {
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_WRONG_LENGTH);
    }
}

void iso7816_answer_to_reset(Iso7816Atr* atr) {
    //minimum valid ATR: https://smartcard-atr.apdu.fr/parse?ATR=3B+00
    atr->TS = 0x3B;
    atr->T0 = 0x00;
}

void iso7816_process_command(
    const ISO7816_Command_APDU* command_apdu,
    ISO7816_Response_APDU* response_apdu) {
    //example 1: sends a command with no body, receives a response with no body
    //sends APDU 0x01:0x01:0x00:0x00
    //receives SW1=0x90, SW2=0x00

    if(command_apdu->CLA == 0x01) {
        switch(command_apdu->INS) {
        case 0x01:
            handle_instruction_01(response_apdu);
            break;
        case 0x02:
            handle_instruction_02(
                command_apdu->P1,
                command_apdu->P2,
                command_apdu->Lc,
                command_apdu->Le,
                response_apdu);
            break;
        case 0x03:
            handle_instruction_03(
                command_apdu->P1, command_apdu->P2, command_apdu->Lc, response_apdu);
            break;
        case 0x04:
            handle_instruction_04(
                command_apdu->P1,
                command_apdu->P2,
                command_apdu->Lc,
                command_apdu->Le,
                command_apdu->Data,
                response_apdu);
            break;
        default:
            iso7816_set_response(response_apdu, ISO7816_RESPONSE_INSTRUCTION_NOT_SUPPORTED);
        }
    } else {
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_CLASS_NOT_SUPPORTED);
    }
}
