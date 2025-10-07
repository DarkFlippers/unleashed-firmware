#include "felica_i.h"
#include <lib/toolbox/hex.h>

#include <furi.h>

#include <nfc/nfc_common.h>

#define FELICA_PROTOCOL_NAME "FeliCa"
#define FELICA_DEVICE_NAME   "FeliCa"

#define FELICA_DATA_FORMAT_VERSION   "Data format version"
#define FELICA_MANUFACTURE_ID        "Manufacture id"
#define FELICA_MANUFACTURE_PARAMETER "Manufacture parameter"

static const uint32_t felica_data_format_version = 2;

/** @brief This is used in felica_prepare_first_block to define which 
 * type of block needs to be prepared.
*/
typedef enum {
    FelicaMACTypeRead,
    FelicaMACTypeWrite,
} FelicaMACType;

const NfcDeviceBase nfc_device_felica = {
    .protocol_name = FELICA_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)felica_alloc,
    .free = (NfcDeviceFree)felica_free,
    .reset = (NfcDeviceReset)felica_reset,
    .copy = (NfcDeviceCopy)felica_copy,
    .verify = (NfcDeviceVerify)felica_verify,
    .load = (NfcDeviceLoad)felica_load,
    .save = (NfcDeviceSave)felica_save,
    .is_equal = (NfcDeviceEqual)felica_is_equal,
    .get_name = (NfcDeviceGetName)felica_get_device_name,
    .get_uid = (NfcDeviceGetUid)felica_get_uid,
    .set_uid = (NfcDeviceSetUid)felica_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)felica_get_base_data,
};

FelicaData* felica_alloc(void) {
    FelicaData* data = malloc(sizeof(FelicaData));
    furi_check(data);

    data->services = simple_array_alloc(&felica_service_array_cfg);
    data->areas = simple_array_alloc(&felica_area_array_cfg);
    data->public_blocks = simple_array_alloc(&felica_public_block_array_cfg);
    furi_check(data->services);
    furi_check(data->areas);
    furi_check(data->public_blocks);
    return data;
}

void felica_free(FelicaData* data) {
    furi_check(data);

    furi_check(data->services);
    simple_array_free(data->services);

    furi_check(data->areas);
    simple_array_free(data->areas);

    furi_check(data->public_blocks);
    simple_array_free(data->public_blocks);

    free(data);
}

void felica_reset(FelicaData* data) {
    furi_check(data);

    if(data->services) {
        simple_array_reset(data->services);
    }

    if(data->areas) {
        simple_array_reset(data->areas);
    }

    if(data->public_blocks) {
        simple_array_reset(data->public_blocks);
    }

    data->blocks_read = 0;
    data->blocks_total = 0;
    data->workflow_type = FelicaUnknown;
    memset(&data->idm, 0, sizeof(data->idm));
    memset(&data->pmm, 0, sizeof(data->pmm));
    memset(&data->data, 0, sizeof(data->data));
}

void felica_copy(FelicaData* data, const FelicaData* other) {
    furi_check(data);
    furi_check(other);

    felica_reset(data);

    data->idm = other->idm;
    data->pmm = other->pmm;
    data->blocks_total = other->blocks_total;
    data->blocks_read = other->blocks_read;
    data->data = other->data;
    data->workflow_type = other->workflow_type;

    simple_array_copy(data->services, other->services);
    simple_array_copy(data->areas, other->areas);
    simple_array_copy(data->public_blocks, other->public_blocks);
}

bool felica_verify(FelicaData* data, const FuriString* device_type) {
    UNUSED(data);
    UNUSED(device_type);

    return false;
}

bool felica_load(FelicaData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);

    bool parsed = false;
    FuriString* str_key_buffer = furi_string_alloc();
    FuriString* str_data_buffer = furi_string_alloc();

    // Header
    do {
        if(version < NFC_UNIFIED_FORMAT_VERSION) break;

        uint32_t data_format_version = 0;
        if(!flipper_format_read_uint32(ff, FELICA_DATA_FORMAT_VERSION, &data_format_version, 1))
            break;

        // V1 saving function always treated everything as Felica Lite
        // So we load the blocks as if everything is Felica Lite

        if(!flipper_format_read_hex(ff, FELICA_MANUFACTURE_ID, data->idm.data, FELICA_IDM_SIZE))
            break;
        if(!flipper_format_read_hex(
               ff, FELICA_MANUFACTURE_PARAMETER, data->pmm.data, FELICA_PMM_SIZE))
            break;

        felica_get_workflow_type(data);
        if(data_format_version == 1) {
            data->workflow_type = FelicaLite;
        }
        parsed = true;
    } while(false);

    if(!parsed) {
        furi_string_free(str_key_buffer);
        furi_string_free(str_data_buffer);
        return false;
    }

    switch(data->workflow_type) {
    case FelicaLite:
        // Blocks data
        do {
            uint32_t blocks_total = 0;
            uint32_t blocks_read = 0;
            if(!flipper_format_read_uint32(ff, "Blocks total", &blocks_total, 1)) break;
            if(!flipper_format_read_uint32(ff, "Blocks read", &blocks_read, 1)) break;
            data->blocks_total = (uint8_t)blocks_total;
            data->blocks_read = (uint8_t)blocks_read;

            for(uint8_t i = 0; i < data->blocks_total; i++) {
                furi_string_printf(str_data_buffer, "Block %d", i);
                if(!flipper_format_read_hex(
                       ff,
                       furi_string_get_cstr(str_data_buffer),
                       (&data->data.dump[i * sizeof(FelicaBlock)]),
                       sizeof(FelicaBlock))) {
                    break;
                }
            }
        } while(false);
        break;
    case FelicaStandard:
        // Areas
        do {
            uint32_t area_count = 0;
            if(!flipper_format_read_uint32(ff, "Area found", &area_count, 1)) break;
            simple_array_init(data->areas, area_count);

            furi_string_reset(str_key_buffer);
            furi_string_reset(str_data_buffer);
            for(uint16_t i = 0; i < area_count; i++) {
                furi_string_printf(str_key_buffer, "Area %03X", i);
                if(!flipper_format_read_string(
                       ff, furi_string_get_cstr(str_key_buffer), str_data_buffer)) {
                    break;
                }
                FelicaArea* area = simple_array_get(data->areas, i);
                if(sscanf(
                       furi_string_get_cstr(str_data_buffer),
                       "| Code %04hX | Services #%03hX-#%03hX |",
                       &area->code,
                       &area->first_idx,
                       &area->last_idx) != 3) {
                    break;
                }
            }
        } while(false);

        // Services
        do {
            uint32_t service_count = 0;
            if(!flipper_format_read_uint32(ff, "Service found", &service_count, 1)) break;
            simple_array_init(data->services, service_count);

            furi_string_reset(str_key_buffer);
            furi_string_reset(str_data_buffer);
            for(uint16_t i = 0; i < service_count; i++) {
                furi_string_printf(str_key_buffer, "Service %03X", i);
                if(!flipper_format_read_string(
                       ff, furi_string_get_cstr(str_key_buffer), str_data_buffer)) {
                    break;
                }
                FelicaService* service = simple_array_get(data->services, i);

                // all unread in the beginning. reserved for future block load
                if(!sscanf(
                       furi_string_get_cstr(str_data_buffer), "| Code %04hX |", &service->code)) {
                    break;
                }
                service->attr = service->code & 0x3F;
            }
        } while(false);

        // Public blocks
        do {
            furi_string_reset(str_data_buffer);
            furi_string_reset(str_key_buffer);
            uint32_t public_block_count = 0;
            if(!flipper_format_read_uint32(ff, "Public blocks read", &public_block_count, 1))
                break;
            simple_array_init(data->public_blocks, public_block_count);
            for(uint16_t i = 0; i < public_block_count; i++) {
                furi_string_printf(str_key_buffer, "Block %04X", i);
                if(!flipper_format_read_string(
                       ff, furi_string_get_cstr(str_key_buffer), str_data_buffer)) {
                    break;
                }

                FelicaPublicBlock* public_block = simple_array_get(data->public_blocks, i);
                if(sscanf(
                       furi_string_get_cstr(str_data_buffer),
                       "| Service code %04hX | Block index %02hhX |",
                       &public_block->service_code,
                       &public_block->block_idx) != 2) {
                    break;
                }

                size_t needle = furi_string_search_str(str_data_buffer, "Data: ");
                if(needle == FURI_STRING_FAILURE) {
                    break;
                }
                needle += 6; // length of "Data: " = 6
                furi_string_mid(str_data_buffer, needle, 3 * FELICA_DATA_BLOCK_SIZE);
                furi_string_replace_all(str_data_buffer, " ", "");
                if(!hex_chars_to_uint8(
                       furi_string_get_cstr(str_data_buffer), public_block->block.data)) {
                    break;
                }

                furi_string_reset(str_data_buffer);
                for(size_t j = 0; j < FELICA_DATA_BLOCK_SIZE; j++) {
                    furi_string_cat_printf(str_data_buffer, "%02X ", public_block->block.data[j]);
                }
            }
        } while(false);
        break;
    default:
        break;
    }

    furi_string_free(str_key_buffer);
    furi_string_free(str_data_buffer);

    return parsed;
}

bool felica_save(const FelicaData* data, FlipperFormat* ff) {
    furi_check(data);

    bool saved = false;
    FuriString* str_data_buffer = furi_string_alloc();
    FuriString* str_key_buffer = furi_string_alloc();
    do {
        // Header
        if(!flipper_format_write_comment_cstr(ff, FELICA_PROTOCOL_NAME " specific data")) break;
        if(!flipper_format_write_uint32(
               ff, FELICA_DATA_FORMAT_VERSION, &felica_data_format_version, 1))
            break;
        if(!flipper_format_write_hex(ff, FELICA_MANUFACTURE_ID, data->idm.data, FELICA_IDM_SIZE))
            break;
        if(!flipper_format_write_hex(
               ff, FELICA_MANUFACTURE_PARAMETER, data->pmm.data, FELICA_PMM_SIZE))
            break;

        saved = true;

        felica_get_ic_name(data, str_data_buffer);
        furi_string_replace_all(str_data_buffer, "\n", " ");
        if(!flipper_format_write_string(ff, "IC Type", str_data_buffer)) break;
        if(!flipper_format_write_empty_line(ff)) break;
    } while(false);

    switch(data->workflow_type) {
    case FelicaLite:
        if(!flipper_format_write_comment_cstr(ff, "Felica Lite specific data")) break;
        // Blocks count
        do {
            uint32_t blocks_total = data->blocks_total;
            uint32_t blocks_read = data->blocks_read;
            if(!flipper_format_write_uint32(ff, "Blocks total", &blocks_total, 1)) break;
            if(!flipper_format_write_uint32(ff, "Blocks read", &blocks_read, 1)) break;

            // Blocks data
            furi_string_reset(str_data_buffer);
            furi_string_reset(str_key_buffer);
            for(uint8_t i = 0; i < blocks_total; i++) {
                furi_string_printf(str_key_buffer, "Block %d", i);
                if(!flipper_format_write_hex(
                       ff,
                       furi_string_get_cstr(str_key_buffer),
                       (&data->data.dump[i * sizeof(FelicaBlock)]),
                       sizeof(FelicaBlock))) {
                    saved = false;
                    break;
                }
            }
        } while(false);
        break;

    case FelicaStandard:
        if(!flipper_format_write_comment_cstr(ff, "Felica Standard specific data")) break;

        do {
            uint32_t area_count = simple_array_get_count(data->areas);
            uint32_t service_count = simple_array_get_count(data->services);
            // Note: The theoretical max area/service count is 2^10
            // So uint16_t is already enough for practical usage
            // The following key index print will use %03X because 12 bits are enough to cover 0-1023

            // Area count
            if(!flipper_format_write_uint32(ff, "Area found", &area_count, 1)) break;

            // Area data
            furi_string_reset(str_data_buffer);
            furi_string_reset(str_key_buffer);
            for(uint16_t i = 0; i < area_count; i++) {
                FelicaArea* area = simple_array_get(data->areas, i);
                furi_string_printf(str_key_buffer, "Area %03X", i);
                furi_string_printf(
                    str_data_buffer,
                    "| Code %04X | Services #%03X-#%03X |",
                    area->code,
                    area->first_idx,
                    area->last_idx);
                if(!flipper_format_write_string(
                       ff, furi_string_get_cstr(str_key_buffer), str_data_buffer))
                    break;
            }
            if(!flipper_format_write_empty_line(ff)) break;

            // Service count
            if(!flipper_format_write_uint32(ff, "Service found", &service_count, 1)) break;

            // Service data
            furi_string_reset(str_data_buffer);
            furi_string_reset(str_key_buffer);
            for(uint16_t i = 0; i < service_count; i++) {
                FelicaService* service = simple_array_get(data->services, i);
                furi_string_printf(str_key_buffer, "Service %03X", i);
                furi_string_printf(
                    str_data_buffer, "| Code %04X | Attrib. %02X ", service->code, service->attr);
                felica_service_get_attribute_string(service, str_data_buffer);
                if(!flipper_format_write_string(
                       ff, furi_string_get_cstr(str_key_buffer), str_data_buffer))
                    break;
            }
            if(!flipper_format_write_empty_line(ff)) break;

            // Directory tree
            furi_string_reset(str_data_buffer);
            furi_string_reset(str_key_buffer);
            furi_string_printf(
                str_data_buffer, "\n::: ... are public services\n||| ... are private services");
            felica_write_directory_tree(data, str_data_buffer);
            furi_string_replace_all(str_data_buffer, ":", "+");
            // We use a clearer marker in saved text files
            if(!flipper_format_write_string(ff, "Directory Tree", str_data_buffer)) break;
        } while(false);

        // Public blocks
        do {
            uint32_t public_block_count = simple_array_get_count(data->public_blocks);
            if(!flipper_format_write_uint32(ff, "Public blocks read", &public_block_count, 1))
                break;
            furi_string_reset(str_data_buffer);
            furi_string_reset(str_key_buffer);
            for(uint16_t i = 0; i < public_block_count; i++) {
                FelicaPublicBlock* public_block = simple_array_get(data->public_blocks, i);
                furi_string_printf(str_key_buffer, "Block %04X", i);
                furi_string_printf(
                    str_data_buffer,
                    "| Service code %04X | Block index %02X | Data: ",
                    public_block->service_code,
                    public_block->block_idx);
                for(uint8_t j = 0; j < FELICA_DATA_BLOCK_SIZE; j++) {
                    furi_string_cat_printf(str_data_buffer, "%02X ", public_block->block.data[j]);
                }
                furi_string_cat_printf(str_data_buffer, "|");
                if(!flipper_format_write_string(
                       ff, furi_string_get_cstr(str_key_buffer), str_data_buffer))
                    break;
            }
        } while(false);
        break;
    default:
        break;
    }

    // Clean up
    furi_string_free(str_data_buffer);
    furi_string_free(str_key_buffer);

    return saved;
}

bool felica_is_equal(const FelicaData* data, const FelicaData* other) {
    furi_check(data);
    furi_check(other);

    return memcmp(data->idm.data, other->idm.data, sizeof(FelicaIDm)) == 0 &&
           memcmp(data->pmm.data, other->pmm.data, sizeof(FelicaPMm)) == 0 &&
           data->blocks_total == other->blocks_total && data->blocks_read == other->blocks_read &&
           memcmp(&data->data, &other->data, sizeof(data->data)) == 0 &&
           simple_array_is_equal(data->services, other->services) &&
           simple_array_is_equal(data->areas, other->areas) &&
           simple_array_is_equal(data->public_blocks, other->public_blocks);
}

const char* felica_get_device_name(const FelicaData* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);

    return FELICA_DEVICE_NAME;
}

const uint8_t* felica_get_uid(const FelicaData* data, size_t* uid_len) {
    furi_check(data);

    // Consider Manufacturer ID as UID
    if(uid_len) {
        *uid_len = FELICA_IDM_SIZE;
    }

    return data->idm.data;
}

bool felica_set_uid(FelicaData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);

    // Consider Manufacturer ID as UID
    const bool uid_valid = uid_len == FELICA_IDM_SIZE;
    if(uid_valid) {
        memcpy(data->idm.data, uid, uid_len);
    }

    return uid_valid;
}

FelicaData* felica_get_base_data(const FelicaData* data) {
    UNUSED(data);
    furi_crash("No base data");
}

static void felica_reverse_copy_block(const uint8_t* array, uint8_t* reverse_array) {
    furi_assert(array);
    furi_assert(reverse_array);

    for(int i = 0; i < 8; i++) {
        reverse_array[i] = array[7 - i];
    }
}

void felica_calculate_session_key(
    mbedtls_des3_context* ctx,
    const uint8_t* ck,
    const uint8_t* rc,
    uint8_t* out) {
    furi_check(ctx);
    furi_check(ck);
    furi_check(rc);
    furi_check(out);

    uint8_t iv[8];
    memset(iv, 0, 8);

    uint8_t ck_reversed[16];
    felica_reverse_copy_block(ck, ck_reversed);
    felica_reverse_copy_block(ck + 8, ck_reversed + 8);

    uint8_t rc_reversed[16];
    felica_reverse_copy_block(rc, rc_reversed);
    felica_reverse_copy_block(rc + 8, rc_reversed + 8);

    mbedtls_des3_set2key_enc(ctx, ck_reversed);
    mbedtls_des3_crypt_cbc(ctx, MBEDTLS_DES_ENCRYPT, FELICA_DATA_BLOCK_SIZE, iv, rc_reversed, out);
}

static bool felica_calculate_mac(
    mbedtls_des3_context* ctx,
    const uint8_t* session_key,
    const uint8_t* rc,
    const uint8_t* first_block,
    const uint8_t* data,
    const size_t length,
    uint8_t* mac) {
    furi_check((length % 8) == 0);

    uint8_t reverse_data[8];
    uint8_t iv[8];
    uint8_t out[8];
    mbedtls_des3_set2key_enc(ctx, session_key);

    felica_reverse_copy_block(rc, iv);
    felica_reverse_copy_block(first_block, reverse_data);
    uint8_t i = 0;
    bool error = false;
    do {
        if(mbedtls_des3_crypt_cbc(ctx, MBEDTLS_DES_ENCRYPT, 8, iv, reverse_data, out) == 0) {
            memcpy(iv, out, sizeof(iv));
            felica_reverse_copy_block(data + i, reverse_data);
            i += 8;
        } else {
            error = true;
            break;
        }
    } while(i <= length);

    if(!error) {
        felica_reverse_copy_block(out, mac);
    }
    return !error;
}

static void felica_prepare_first_block(
    FelicaMACType operation_type,
    const uint8_t* blocks,
    const uint8_t block_count,
    uint8_t* out) {
    furi_check(blocks);
    furi_check(out);
    if(operation_type == FelicaMACTypeRead) {
        memset(out, 0xFF, 8);
        for(uint8_t i = 0, j = 0; i < block_count; i++, j += 2) {
            out[j] = blocks[i];
            out[j + 1] = 0;
        }
    } else {
        furi_check(block_count == 4);
        memset(out, 0, 8);
        out[0] = blocks[0];
        out[1] = blocks[1];
        out[2] = blocks[2];
        out[4] = blocks[3];
        out[6] = FELICA_BLOCK_INDEX_MAC_A;
    }
}

bool felica_check_mac(
    mbedtls_des3_context* ctx,
    const uint8_t* session_key,
    const uint8_t* rc,
    const uint8_t* blocks,
    const uint8_t block_count,
    uint8_t* data) {
    furi_check(ctx);
    furi_check(session_key);
    furi_check(rc);
    furi_check(blocks);
    furi_check(data);

    uint8_t mac[8];
    felica_calculate_mac_read(ctx, session_key, rc, blocks, block_count, data, mac);

    uint8_t mac_offset = FELICA_DATA_BLOCK_SIZE * (block_count - 1);
    uint8_t* mac_ptr = data + mac_offset;
    return !memcmp(mac, mac_ptr, 8);
}

void felica_calculate_mac_read(
    mbedtls_des3_context* ctx,
    const uint8_t* session_key,
    const uint8_t* rc,
    const uint8_t* blocks,
    const uint8_t block_count,
    const uint8_t* data,
    uint8_t* mac) {
    furi_check(ctx);
    furi_check(session_key);
    furi_check(rc);
    furi_check(blocks);
    furi_check(data);
    furi_check(mac);

    uint8_t first_block[8];
    felica_prepare_first_block(FelicaMACTypeRead, blocks, block_count, first_block);
    uint8_t data_size_without_mac = FELICA_DATA_BLOCK_SIZE * (block_count - 1);
    felica_calculate_mac(ctx, session_key, rc, first_block, data, data_size_without_mac, mac);
}

void felica_calculate_mac_write(
    mbedtls_des3_context* ctx,
    const uint8_t* session_key,
    const uint8_t* rc,
    const uint8_t* wcnt,
    const uint8_t* data,
    uint8_t* mac) {
    furi_check(ctx);
    furi_check(session_key);
    furi_check(rc);
    furi_check(wcnt);
    furi_check(data);
    furi_check(mac);

    const uint8_t WCNT_length = 3;
    uint8_t block_data[WCNT_length + 1];
    uint8_t first_block[8];

    memcpy(block_data, wcnt, WCNT_length);
    block_data[3] = FELICA_BLOCK_INDEX_STATE;
    felica_prepare_first_block(FelicaMACTypeWrite, block_data, WCNT_length + 1, first_block);

    uint8_t session_swapped[FELICA_DATA_BLOCK_SIZE];
    memcpy(session_swapped, session_key + 8, 8);
    memcpy(session_swapped + 8, session_key, 8);
    felica_calculate_mac(ctx, session_swapped, rc, first_block, data, FELICA_DATA_BLOCK_SIZE, mac);
}

void felica_write_directory_tree(const FelicaData* data, FuriString* str) {
    furi_check(data);
    furi_check(str);

    furi_string_cat_str(str, "\n");

    uint16_t area_last_stack[8];
    uint8_t depth = 0;

    size_t area_iter = 0;
    const size_t area_count = simple_array_get_count(data->areas);
    const size_t service_count = simple_array_get_count(data->services);

    for(size_t svc_idx = 0; svc_idx < service_count; ++svc_idx) {
        while(area_iter < area_count) {
            const FelicaArea* next_area = simple_array_get(data->areas, area_iter);
            if(next_area->first_idx != svc_idx) break;

            for(uint8_t i = 0; i < depth - 1; ++i)
                furi_string_cat_printf(str, "| ");
            furi_string_cat_printf(str, depth ? "|" : "");
            furi_string_cat_printf(str, "- AREA_%04X/\n", next_area->code >> 6);

            area_last_stack[depth++] = next_area->last_idx;
            area_iter++;
        }

        const FelicaService* service = simple_array_get(data->services, svc_idx);
        bool is_public = (service->attr & FELICA_SERVICE_ATTRIBUTE_UNAUTH_READ) != 0;

        for(uint8_t i = 0; i < depth - 1; ++i)
            furi_string_cat_printf(str, is_public ? ": " : "| ");
        furi_string_cat_printf(str, is_public ? ":" : "|");
        furi_string_cat_printf(str, "- serv_%04X\n", service->code);

        if(depth && svc_idx >= area_last_stack[depth - 1]) depth--;
    }
}

void felica_get_workflow_type(FelicaData* data) {
    // Reference: Proxmark3 repo
    uint8_t rom_type = data->pmm.data[0];
    uint8_t workflow_type = data->pmm.data[1];
    if(workflow_type <= 0x48) {
        // More liberal check because most of these should be treated as FeliCa Standard, regardless of mobile or not.
        data->workflow_type = FelicaStandard;
    } else {
        switch(workflow_type) {
        case 0xA2:
            data->workflow_type = FelicaStandard;
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2: // 0xF2 => FeliCa Link RC-S967 in Lite-S Mode or Lite-S HT Mode
            data->workflow_type = FelicaLite;
            break;
        case 0xE1: // Felica Link
        case 0xE0: // Felica Plug
            data->workflow_type = FelicaUnknown;
            break;
        case 0xFF:
            if(rom_type == 0xFF) {
                data->workflow_type = FelicaUnknown; // Felica Link
            }
            break;
        default:
            data->workflow_type = FelicaUnknown;
            break;
        }
    }
}

void felica_get_ic_name(const FelicaData* data, FuriString* ic_name) {
    // Reference: Proxmark3 repo
    uint8_t rom_type = data->pmm.data[0];
    uint8_t ic_type = data->pmm.data[1];

    switch(ic_type) {
        // FeliCa Standard Products:
        // odd findings
    case 0x00:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S830");
        break;
    case 0x01:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S915");
        break;
    case 0x02:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S919");
        break;
    case 0x06:
    case 0x07:
        furi_string_set_str(ic_name, "FeliCa Mobile IC,\nChip V1.0");
        break;
    case 0x08:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S952");
        break;
    case 0x09:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S953");
        break;
    case 0x0B:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S9X4,\nJapan Transit IC");
        break;
    case 0x0C:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S954");
        break;
    case 0x0D:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S960");
        break;
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
        furi_string_set_str(ic_name, "FeliCa Mobile IC,\nChip V2.0");
        break;
    case 0x14:
    case 0x15:
        furi_string_set_str(ic_name, "FeliCa Mobile IC,\nChip V3.0");
        break;
    case 0x16:
        furi_string_set_str(ic_name, "FeliCa Mobile IC,\nJapan Transit IC");
        break;
    case 0x17:
        furi_string_set_str(ic_name, "FeliCa Mobile IC,\nChip V4.0");
        break;
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:
        furi_string_set_str(ic_name, "FeliCa Mobile IC,\nChip V4.1");
        break;
    case 0x20:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S962");
        // RC-S962 has been extensively found in Japan Transit ICs, despite model number not ending in 4
        break;
    case 0x31:
        furi_string_set_str(ic_name, "FeliCa Standard RC-S104,\nJapan Transit IC");
        break;
    case 0x32:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA00/1");
        break;
    case 0x33:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA00/2");
        break;
    case 0x34:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA01/1");
        break;
    case 0x35:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA01/2");
        break;
    case 0x36:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA04/1,\nJapan Transit IC");
        break;
    case 0x3E:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA08/1");
        break;
    case 0x43:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA24/1");
        break;
    case 0x44:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA20/1");
        break;
    case 0x45:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA20/2");
        break;
    case 0x46:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA21/2");
        break;
    case 0x47:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA24/1x1");
        break;
    case 0x48:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA21/2x1");
        break;
    case 0xA2:
        furi_string_set_str(ic_name, "FeliCa Standard RC-SA14");
        break;
    // NFC Dynamic Tag (FeliCa Plug) Products:
    case 0xE0:
        furi_string_set_str(ic_name, "FeliCa Plug RC-S926,\nNFC Dynamic Tag");
        break;
    case 0xE1:
        furi_string_set_str(ic_name, "FeliCa Link RC-S967,\nPlug Mode");
        break;
    case 0xF0:
        furi_string_set_str(ic_name, "FeliCa Lite RC-S965");
        break;
    case 0xF1:
        furi_string_set_str(ic_name, "FeliCa Lite-S RC-S966");
        break;
    case 0xF2:
        furi_string_set_str(ic_name, "FeliCa Link RC-S967,\nLite-S Mode or Lite-S HT Mode");
        break;
    case 0xFF:
        if(rom_type == 0xFF) { // from FeliCa Link User's Manual
            furi_string_set_str(ic_name, "FeliCa Link RC-S967,\nNFC-DEP Mode");
        }
        break;
    default:
        furi_string_printf(
            ic_name,
            "Unknown IC %02X ROM %02X:\nPlease submit an issue on\nGitHub and help us identify.",
            ic_type,
            rom_type);
        break;
    }
}

void felica_service_get_attribute_string(const FelicaService* service, FuriString* str) {
    furi_check(service);
    furi_check(str);

    bool is_public = (service->attr & FELICA_SERVICE_ATTRIBUTE_UNAUTH_READ) != 0;
    furi_string_cat_str(str, is_public ? "| Public  " : "| Private ");

    bool is_purse = (service->attr & FELICA_SERVICE_ATTRIBUTE_PURSE) != 0;
    // Subfield bitwise attributes are applicable depending on is PURSE or not

    if(is_purse) {
        furi_string_cat_str(str, "| Purse  |");
        switch((service->attr & FELICA_SERVICE_ATTRIBUTE_PURSE_SUBFIELD) >> 1) {
        case 0:
            furi_string_cat_str(str, " Direct     |");
            break;
        case 1:
            furi_string_cat_str(str, " Cashback   |");
            break;
        case 2:
            furi_string_cat_str(str, " Decrement  |");
            break;
        case 3:
            furi_string_cat_str(str, " Read Only  |");
            break;
        default:
            furi_string_cat_str(str, " Unknown    |");
            break;
        }
    } else {
        bool is_random = (service->attr & FELICA_SERVICE_ATTRIBUTE_RANDOM_ACCESS) != 0;
        furi_string_cat_str(str, is_random ? "| Random |" : "| Cyclic |");
        bool is_readonly = (service->attr & FELICA_SERVICE_ATTRIBUTE_READ_ONLY) != 0;
        furi_string_cat_str(str, is_readonly ? " Read Only  |" : " Read/Write |");
    }
}
