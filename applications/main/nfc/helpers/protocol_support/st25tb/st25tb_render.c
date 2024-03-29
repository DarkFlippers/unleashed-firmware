#include "st25tb_render.h"
#include <nfc/protocols/st25tb/st25tb.h>
#include <machine/endian.h>

void nfc_render_st25tb_info(
    const St25tbData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    furi_string_cat_printf(str, "UID:");

    for(size_t i = 0; i < ST25TB_UID_SIZE; i++) {
        furi_string_cat_printf(str, " %02X", data->uid[i]);
    }

    if(format_type == NfcProtocolFormatTypeFull) {
        furi_string_cat_printf(
            str, "\nSys. OTP: %08lX", (uint32_t)__bswap32(data->system_otp_block));
        furi_string_cat_printf(str, "\n::::::::::::::::::::::[Blocks]::::::::::::::::::::::");
        for(size_t i = 0; i < st25tb_get_block_count(data->type); i += 2) {
            furi_string_cat_printf(
                str,
                "\n %02X   %08lX  %08lX",
                i,
                (uint32_t)__bswap32(data->blocks[i]),
                (uint32_t)__bswap32(data->blocks[i + 1]));
        }
    }
}
