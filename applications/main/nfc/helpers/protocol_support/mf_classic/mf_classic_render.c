#include "mf_classic_render.h"

#include "../iso14443_3a/iso14443_3a_render.h"

void nfc_render_mf_classic_info(
    const MfClassicData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    nfc_render_iso14443_3a_info(data->iso14443_3a_data, format_type, str);

    uint8_t sectors_total = mf_classic_get_total_sectors_num(data->type);
    uint8_t keys_total = sectors_total * 2;
    uint8_t keys_found = 0;
    uint8_t sectors_read = 0;
    mf_classic_get_read_sectors_and_keys(data, &sectors_read, &keys_found);

    furi_string_cat_printf(str, "\nKeys Found: %u/%u", keys_found, keys_total);
    furi_string_cat_printf(str, "\nSectors Read: %u/%u", sectors_read, sectors_total);
}

void nfc_render_mf_classic_dump(const MfClassicData* data, FuriString* str) {
    uint16_t total_blocks = mf_classic_get_total_block_num(data->type);

    for(size_t i = 0; i < total_blocks; i++) {
        for(size_t j = 0; j < sizeof(MfClassicBlock); j += 2) {
            furi_string_cat_printf(
                str, "%02X%02X ", data->block[i].data[j], data->block[i].data[j + 1]);
        }
    }
}
