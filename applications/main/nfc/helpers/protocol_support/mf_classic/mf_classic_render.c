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
        const uint8_t* block_data = data->block[i].data;
        if(mf_classic_is_block_read(data, i)) {
            if(mf_classic_is_sector_trailer(i)) {
                uint8_t sector = mf_classic_get_sector_by_block(i);
                // Key A
                if(mf_classic_is_key_found(data, sector, MfClassicKeyTypeA)) {
                    furi_string_cat_printf(
                        str,
                        "%02X%02X %02X%02X %02X%02X ",
                        block_data[0],
                        block_data[1],
                        block_data[2],
                        block_data[3],
                        block_data[4],
                        block_data[5]);
                } else {
                    furi_string_cat(str, "???? ???? ???? ");
                }
                // Access bits
                furi_string_cat_printf(
                    str,
                    "%02X%02X %02X%02X ",
                    block_data[6],
                    block_data[7],
                    block_data[8],
                    block_data[9]);
                // Key B
                if(mf_classic_is_key_found(data, sector, MfClassicKeyTypeB)) {
                    furi_string_cat_printf(
                        str,
                        "%02X%02X %02X%02X %02X%02X ",
                        block_data[10],
                        block_data[11],
                        block_data[12],
                        block_data[13],
                        block_data[14],
                        block_data[15]);
                } else {
                    furi_string_cat(str, "???? ???? ???? ");
                }
            } else {
                for(size_t j = 0; j < sizeof(MfClassicBlock); j += 2) {
                    furi_string_cat_printf(str, "%02X%02X ", block_data[j], block_data[j + 1]);
                }
            }
        } else {
            for(size_t j = 0; j < sizeof(MfClassicBlock); j += 2) {
                furi_string_cat(str, "???? ");
            }
        }
    }
}
