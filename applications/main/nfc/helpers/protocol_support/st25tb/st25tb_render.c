#include "st25tb_render.h"
#include <nfc/protocols/st25tb/st25tb.h>

void nfc_render_st25tb_info(
    const St25tbData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    furi_string_cat_printf(str, "UID");

    for(size_t i = 0; i < ST25TB_UID_SIZE; i++) {
        furi_string_cat_printf(str, " %02X", data->uid[i]);
    }

    if(format_type == NfcProtocolFormatTypeFull) {
        furi_string_cat_printf(str, "\nSys. OTP: %08lX", data->system_otp_block);
        furi_string_cat_printf(str, "\nBlocks:");
        for(size_t i = 0; i < st25tb_get_block_count(data->type); i += 2) {
            furi_string_cat_printf(
                str, "\n %02X   %08lX  %08lX", i, data->blocks[i], data->blocks[i + 1]);
        }
    }
}
