#include "felica_render.h"

void nfc_render_felica_blocks_count(
    const FelicaData* data,
    FuriString* str,
    bool render_auth_notification) {
    furi_string_cat_printf(str, "\nBlocks Read: %u/%u", data->blocks_read, data->blocks_total);
    if(render_auth_notification && data->blocks_read != data->blocks_total) {
        furi_string_cat_printf(str, "\nAuth-protected blocks!");
    }
}

void nfc_render_felica_idm(
    const FelicaData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    furi_string_cat_printf(str, (format_type == NfcProtocolFormatTypeFull) ? "IDm:\n" : "IDm:");

    for(size_t i = 0; i < FELICA_IDM_SIZE; i++) {
        furi_string_cat_printf(
            str,
            (format_type == NfcProtocolFormatTypeFull) ? "%02X " : " %02X",
            data->idm.data[i]);
    }
}

void nfc_render_felica_info(
    const FelicaData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    if(format_type == NfcProtocolFormatTypeFull) {
        furi_string_cat_printf(str, "Tech: JIS X 6319-4,\nISO 18092 [NFC-F]\n");
    }

    nfc_render_felica_idm(data, format_type, str);

    if(format_type == NfcProtocolFormatTypeFull) {
        furi_string_cat_printf(str, "\nPMm:\n");
        for(size_t i = 0; i < FELICA_PMM_SIZE; ++i) {
            furi_string_cat_printf(str, "%02X ", data->pmm.data[i]);
        }
    }
    nfc_render_felica_blocks_count(data, str, true);
}

static void nfc_render_felica_block_name(
    const char* name,
    FuriString* str,
    uint8_t prefix_separator_cnt,
    uint8_t suffix_separator_cnt) {
    for(uint8_t i = 0; i < prefix_separator_cnt; i++) {
        furi_string_cat_printf(str, ":");
    }
    furi_string_cat_printf(str, "[ %s ]", name);
    for(uint8_t i = 0; i < suffix_separator_cnt; i++) {
        furi_string_cat_printf(str, ":");
    }
}

static void nfc_render_felica_block_data(const FelicaBlock* block, FuriString* str) {
    furi_string_cat_printf(str, "\nSF1=%02X; SF2=%02X\n", block->SF1, block->SF2);
    for(size_t j = 0; j < FELICA_DATA_BLOCK_SIZE; j++) {
        if((j != 0) && (j % 8 == 0)) furi_string_cat_printf(str, "\n");
        furi_string_cat_printf(str, "%02X ", block->data[j]);
    }
    furi_string_cat_printf(str, "\n");
}

static void nfc_render_felica_block(
    const FelicaBlock* block,
    FuriString* str,
    const char* name,
    uint8_t prefix_separator_cnt,
    uint8_t suffix_separator_cnt) {
    nfc_render_felica_block_name(name, str, prefix_separator_cnt, suffix_separator_cnt);
    nfc_render_felica_block_data(block, str);
}

void nfc_render_felica_dump(const FelicaData* data, FuriString* str) {
    FuriString* name = furi_string_alloc();
    for(size_t i = 0; i < 14; i++) {
        furi_string_printf(name, "S_PAD%d", i);
        uint8_t suf_cnt = 18;
        if(i == 1) {
            suf_cnt = 19;
        } else if((i == 10) || (i == 12) || (i == 13)) {
            suf_cnt = 16;
        }
        nfc_render_felica_block(
            &data->data.fs.spad[i], str, furi_string_get_cstr(name), 20, suf_cnt);
    }
    furi_string_free(name);
    nfc_render_felica_block(&data->data.fs.reg, str, "REG", 23, 23);
    nfc_render_felica_block(&data->data.fs.rc, str, "RC", 25, 25);
    nfc_render_felica_block(&data->data.fs.mac, str, "MAC", 23, 23);
    nfc_render_felica_block(&data->data.fs.id, str, "ID", 25, 25);
    nfc_render_felica_block(&data->data.fs.d_id, str, "D_ID", 22, 24);
    nfc_render_felica_block(&data->data.fs.ser_c, str, "SER_C", 20, 21);
    nfc_render_felica_block(&data->data.fs.sys_c, str, "SYS_C", 20, 21);
    nfc_render_felica_block(&data->data.fs.ckv, str, "CKV", 23, 23);
    nfc_render_felica_block(&data->data.fs.ck, str, "CK", 25, 25);
    nfc_render_felica_block(&data->data.fs.mc, str, "MC", 25, 24);
    nfc_render_felica_block(&data->data.fs.wcnt, str, "WCNT", 22, 20);
    nfc_render_felica_block(&data->data.fs.mac_a, str, "MAC_A", 20, 20);
    nfc_render_felica_block(&data->data.fs.state, str, "STATE", 20, 21);
    nfc_render_felica_block(&data->data.fs.crc_check, str, "CRC_CHCK", 15, 17);
}
