#include "felica_render.h"

void nfc_render_felica_blocks_count(
    const FelicaData* data,
    FuriString* str,
    bool render_auth_notification) {
    if(data->workflow_type == FelicaLite) {
        furi_string_cat_printf(str, "Blocks: %u\n", data->blocks_total);

        furi_string_cat_printf(str, "\nBlocks Read: %u/%u", data->blocks_read, data->blocks_total);
        if(render_auth_notification && data->blocks_read != data->blocks_total) {
            furi_string_cat_printf(str, "\nAuth-protected blocks!");
        }
    } else if(data->workflow_type == FelicaStandard) {
        furi_string_cat_printf(
            str, "Public blocks Read: %lu", simple_array_get_count(data->public_blocks));
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

    FuriString* ic_type_str = furi_string_alloc();
    felica_get_ic_name(data, ic_type_str);
    furi_string_cat_printf(str, "IC Type:\n%s\n", furi_string_get_cstr(ic_type_str));
    furi_string_free(ic_type_str);

    nfc_render_felica_idm(data, format_type, str);

    if(format_type == NfcProtocolFormatTypeFull) {
        furi_string_cat_printf(str, "\nPMm:\n");
        for(size_t i = 0; i < FELICA_PMM_SIZE; ++i) {
            furi_string_cat_printf(str, "%02X ", data->pmm.data[i]);
        }
    }

    furi_string_cat_printf(str, "\n");
    furi_string_cat_printf(
        str,
        "Services found: %lu \nAreas found: %lu\n",
        simple_array_get_count(data->services),
        simple_array_get_count(data->areas));

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
    for(size_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i += 2) {
        furi_string_cat_printf(str, "%02X%02X ", block->data[i], block->data[i + 1]);
    }
    furi_string_cat_printf(str, "\n");
}

static void nfc_render_felica_block_data_simple(const FelicaBlock* block, FuriString* str) {
    for(size_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i += 2) {
        furi_string_cat_printf(str, "%02X%02X ", block->data[i], block->data[i + 1]);
    }
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

void nfc_more_info_render_felica_lite_dump(const FelicaData* data, FuriString* str) {
    FuriString* name = furi_string_alloc();

    furi_string_cat_printf(str, "\e#Blocks read:\n");

    furi_string_cat_printf(str, "Blocks: %u\n", data->blocks_total);

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

void nfc_more_info_render_felica_dir(const FelicaData* data, FuriString* str) {
    const size_t area_count = simple_array_get_count(data->areas);
    const size_t service_count = simple_array_get_count(data->services);

    furi_string_cat_printf(str, "\e#Directory Tree:\n");

    if(area_count == 0 || service_count == 0) {
        furi_string_cat_printf(str, "No services or areas found.\n");
    } else {
        furi_string_cat_printf(
            str, "%zu areas found.\n%zu services found.\n\n", area_count, service_count);
        furi_string_cat_printf(
            str, "::: ... are readable services\n||| ... are locked services\n");
    }
    felica_write_directory_tree(data, str);
}

void nfc_more_info_render_felica_blocks(
    const FelicaData* data,
    FuriString* str,
    const uint16_t service_code_key) {
    furi_string_cat_printf(str, "\n");
    if(data->workflow_type == FelicaLite) {
        furi_string_cat_printf(str, "Blocks: %u\n", data->blocks_total);
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

    } else if(data->workflow_type == FelicaStandard) {
        uint32_t public_blocks_count = simple_array_get_count(data->public_blocks);
        for(size_t i = 0; i < public_blocks_count; i++) {
            FelicaPublicBlock* public_block = simple_array_get(data->public_blocks, i);
            if(public_block->service_code != service_code_key) {
                continue; // Skip blocks not matching the requested service code
            }
            furi_string_cat_printf(str, "-----Block 0x%02X-----\n", public_block->block_idx);
            nfc_render_felica_block_data_simple(&public_block->block, str);
            furi_string_cat_printf(str, "\n");
        }
    }
}
