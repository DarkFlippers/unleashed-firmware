// transforms low level calls such as XFRCallback or ICC Power on to a structured one
// an application can register these calls and listen for the callbacks defined in Iso7816Callbacks

#include <stdint.h>
#include <stddef.h>
#include <furi.h>
#include <furi_hal.h>

#include "iso7816_t0_apdu.h"
#include "iso7816_atr.h"
#include "iso7816_handler.h"
#include "iso7816_response.h"

void iso7816_icc_power_on_callback(uint8_t* atr_data, uint32_t* atr_data_len, void* context) {
    furi_check(context);

    Iso7816Handler* handler = (Iso7816Handler*)context;

    Iso7816Atr iso7816_atr;
    handler->iso7816_answer_to_reset(&iso7816_atr);

    furi_assert(iso7816_atr.T0 == 0x00);

    uint8_t atr_buffer[2] = {iso7816_atr.TS, iso7816_atr.T0};

    *atr_data_len = 2;

    memcpy(atr_data, atr_buffer, sizeof(uint8_t) * (*atr_data_len));
}

//dataBlock points to the buffer
//dataBlockLen tells reader how nany bytes should be read
void iso7816_xfr_datablock_callback(
    const uint8_t* pc_to_reader_datablock,
    uint32_t pc_to_reader_datablock_len,
    uint8_t* reader_to_pc_datablock,
    uint32_t* reader_to_pc_datablock_len,
    void* context) {
    furi_check(context);

    Iso7816Handler* handler = (Iso7816Handler*)context;

    ISO7816_Response_APDU* response_apdu = (ISO7816_Response_APDU*)&handler->response_apdu_buffer;

    ISO7816_Command_APDU* command_apdu = (ISO7816_Command_APDU*)&handler->command_apdu_buffer;

    uint8_t result = iso7816_read_command_apdu(
        command_apdu, pc_to_reader_datablock, pc_to_reader_datablock_len);

    if(result == ISO7816_READ_COMMAND_APDU_OK) {
        handler->iso7816_process_command(command_apdu, response_apdu);

        furi_assert(response_apdu->DataLen < CCID_SHORT_APDU_SIZE);
    } else if(result == ISO7816_READ_COMMAND_APDU_ERROR_WRONG_LE) {
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_WRONG_LE);
    } else if(result == ISO7816_READ_COMMAND_APDU_ERROR_WRONG_LENGTH) {
        iso7816_set_response(response_apdu, ISO7816_RESPONSE_WRONG_LENGTH);
    }

    iso7816_write_response_apdu(response_apdu, reader_to_pc_datablock, reader_to_pc_datablock_len);
}

Iso7816Handler* iso7816_handler_alloc() {
    Iso7816Handler* handler = malloc(sizeof(Iso7816Handler));
    handler->ccid_callbacks.icc_power_on_callback = iso7816_icc_power_on_callback;
    handler->ccid_callbacks.xfr_datablock_callback = iso7816_xfr_datablock_callback;
    return handler;
}
