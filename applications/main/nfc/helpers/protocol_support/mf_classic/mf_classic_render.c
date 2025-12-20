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

static void
    mf_classic_render_raw_data(const uint8_t* data, size_t size, bool data_read, FuriString* str) {
    furi_assert((size % 2) == 0);

    for(size_t i = 0; i < size; i += 2) {
        if(data_read) {
            furi_string_cat_printf(str, "%02X%02X ", data[i], data[i + 1]);
        } else {
            furi_string_cat_printf(str, "???? ");
        }
    }
}

static void
    mf_classic_render_block(const MfClassicData* data, uint8_t block_num, FuriString* str) {
    if(mf_classic_is_sector_trailer(block_num)) {
        uint8_t sec_num = mf_classic_get_sector_by_block(block_num);
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, sec_num);

        // Render key A
        bool key_read = mf_classic_is_key_found(data, sec_num, MfClassicKeyTypeA);
        mf_classic_render_raw_data(sec_tr->key_a.data, sizeof(MfClassicKey), key_read, str);

        // Render access bits
        bool access_bits_read = mf_classic_is_block_read(data, block_num);
        mf_classic_render_raw_data(
            sec_tr->access_bits.data, sizeof(MfClassicAccessBits), access_bits_read, str);

        // Render key B
        key_read = mf_classic_is_key_found(data, sec_num, MfClassicKeyTypeB);
        mf_classic_render_raw_data(sec_tr->key_b.data, sizeof(MfClassicKey), key_read, str);
    } else {
        const uint8_t* block_data = data->block[block_num].data;
        bool block_read = mf_classic_is_block_read(data, block_num);
        mf_classic_render_raw_data(block_data, sizeof(MfClassicBlock), block_read, str);
    }
}

void nfc_render_mf_classic_dump(const MfClassicData* data, FuriString* str) {
    uint16_t total_blocks = mf_classic_get_total_block_num(data->type);

    for(size_t i = 0; i < total_blocks; i++) {
        mf_classic_render_block(data, i, str);
    }
}
