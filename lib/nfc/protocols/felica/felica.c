#include "felica.h"

#include <furi.h>

#include <nfc/nfc_common.h>

#define FELICA_PROTOCOL_NAME "FeliCa"
#define FELICA_DEVICE_NAME   "FeliCa"

#define FELICA_DATA_FORMAT_VERSION   "Data format version"
#define FELICA_MANUFACTURE_ID        "Manufacture id"
#define FELICA_MANUFACTURE_PARAMETER "Manufacture parameter"

static const uint32_t felica_data_format_version = 1;

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
    return data;
}

void felica_free(FelicaData* data) {
    furi_check(data);
    free(data);
}

void felica_reset(FelicaData* data) {
    furi_check(data);
    memset(data, 0, sizeof(FelicaData));
}

void felica_copy(FelicaData* data, const FelicaData* other) {
    furi_check(data);
    furi_check(other);

    *data = *other;
}

bool felica_verify(FelicaData* data, const FuriString* device_type) {
    UNUSED(data);
    UNUSED(device_type);

    return false;
}

bool felica_load(FelicaData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);

    bool parsed = false;

    do {
        if(version < NFC_UNIFIED_FORMAT_VERSION) break;

        uint32_t data_format_version = 0;
        if(!flipper_format_read_uint32(ff, FELICA_DATA_FORMAT_VERSION, &data_format_version, 1))
            break;
        if(data_format_version != felica_data_format_version) break;
        if(!flipper_format_read_hex(ff, FELICA_MANUFACTURE_ID, data->idm.data, FELICA_IDM_SIZE))
            break;
        if(!flipper_format_read_hex(
               ff, FELICA_MANUFACTURE_PARAMETER, data->pmm.data, FELICA_PMM_SIZE))
            break;

        parsed = true;
        uint32_t blocks_total = 0;
        uint32_t blocks_read = 0;
        if(!flipper_format_read_uint32(ff, "Blocks total", &blocks_total, 1)) break;
        if(!flipper_format_read_uint32(ff, "Blocks read", &blocks_read, 1)) break;
        data->blocks_total = (uint8_t)blocks_total;
        data->blocks_read = (uint8_t)blocks_read;

        FuriString* temp_str = furi_string_alloc();
        for(uint8_t i = 0; i < data->blocks_total; i++) {
            furi_string_printf(temp_str, "Block %d", i);
            if(!flipper_format_read_hex(
                   ff,
                   furi_string_get_cstr(temp_str),
                   (&data->data.dump[i * sizeof(FelicaBlock)]),
                   sizeof(FelicaBlock))) {
                parsed = false;
                break;
            }
        }
    } while(false);

    return parsed;
}

bool felica_save(const FelicaData* data, FlipperFormat* ff) {
    furi_check(data);

    bool saved = false;

    do {
        if(!flipper_format_write_comment_cstr(ff, FELICA_PROTOCOL_NAME " specific data")) break;
        if(!flipper_format_write_uint32(
               ff, FELICA_DATA_FORMAT_VERSION, &felica_data_format_version, 1))
            break;
        if(!flipper_format_write_hex(ff, FELICA_MANUFACTURE_ID, data->idm.data, FELICA_IDM_SIZE))
            break;
        if(!flipper_format_write_hex(
               ff, FELICA_MANUFACTURE_PARAMETER, data->pmm.data, FELICA_PMM_SIZE))
            break;

        uint32_t blocks_total = data->blocks_total;
        uint32_t blocks_read = data->blocks_read;
        if(!flipper_format_write_uint32(ff, "Blocks total", &blocks_total, 1)) break;
        if(!flipper_format_write_uint32(ff, "Blocks read", &blocks_read, 1)) break;

        saved = true;
        FuriString* temp_str = furi_string_alloc();
        for(uint8_t i = 0; i < blocks_total; i++) {
            furi_string_printf(temp_str, "Block %d", i);
            if(!flipper_format_write_hex(
                   ff,
                   furi_string_get_cstr(temp_str),
                   (&data->data.dump[i * sizeof(FelicaBlock)]),
                   sizeof(FelicaBlock))) {
                saved = false;
                break;
            }
        }
        furi_string_free(temp_str);
    } while(false);

    return saved;
}

bool felica_is_equal(const FelicaData* data, const FelicaData* other) {
    furi_check(data);
    furi_check(other);

    return memcmp(data, other, sizeof(FelicaData)) == 0;
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
