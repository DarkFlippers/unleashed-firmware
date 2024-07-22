#include "st25tb.h"

#include "flipper_format.h"
#include <furi.h>

#include <nfc/nfc_common.h>
#include <nfc/helpers/iso14443_crc.h>

#define ST25TB_PROTOCOL_NAME    "ST25TB"
#define ST25TB_TYPE_KEY         "ST25TB Type"
#define ST25TB_BLOCK_KEY        "Block %d"
#define ST25TB_SYSTEM_BLOCK_KEY "System OTP Block"

typedef struct {
    uint8_t blocks_total;
    bool has_otp;
    const char* full_name;
    const char* type_name;
} St25tbFeatures;

static const St25tbFeatures st25tb_features[St25tbTypeNum] = {
    [St25tbType512At] =
        {
            .blocks_total = 16,
            .has_otp = false,
            .full_name = "ST25TB512-AT/SRI512",
            .type_name = "512AT",
        },
    [St25tbType512Ac] =
        {
            .blocks_total = 16,
            .has_otp = true,
            .full_name = "ST25TB512-AC/SRT512",
            .type_name = "512AC",
        },
    [St25tbTypeX512] =
        {
            .blocks_total = 16,
            .has_otp = true,
            .full_name = "SRIX512",
            .type_name = "X512",
        },
    [St25tbType02k] =
        {
            .blocks_total = 64,
            .has_otp = true,
            .full_name = "ST25TB02K/SRI2K",
            .type_name = "2K",
        },
    [St25tbType04k] =
        {
            .blocks_total = 128,
            .has_otp = true,
            .full_name = "ST25TB04K/SRI4K",
            .type_name = "4K",
        },
    [St25tbTypeX4k] =
        {
            .blocks_total = 128,
            .has_otp = true,
            .full_name = "SRIX4K",
            .type_name = "X4K",
        },
};

const NfcDeviceBase nfc_device_st25tb = {
    .protocol_name = ST25TB_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)st25tb_alloc,
    .free = (NfcDeviceFree)st25tb_free,
    .reset = (NfcDeviceReset)st25tb_reset,
    .copy = (NfcDeviceCopy)st25tb_copy,
    .verify = (NfcDeviceVerify)st25tb_verify,
    .load = (NfcDeviceLoad)st25tb_load,
    .save = (NfcDeviceSave)st25tb_save,
    .is_equal = (NfcDeviceEqual)st25tb_is_equal,
    .get_name = (NfcDeviceGetName)st25tb_get_device_name,
    .get_uid = (NfcDeviceGetUid)st25tb_get_uid,
    .set_uid = (NfcDeviceSetUid)st25tb_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)st25tb_get_base_data,
};

St25tbData* st25tb_alloc(void) {
    St25tbData* data = malloc(sizeof(St25tbData));
    return data;
}

void st25tb_free(St25tbData* data) {
    furi_check(data);

    free(data);
}

void st25tb_reset(St25tbData* data) {
    furi_check(data);
    memset(data, 0, sizeof(St25tbData));
}

void st25tb_copy(St25tbData* data, const St25tbData* other) {
    furi_check(data);
    furi_check(other);

    *data = *other;
}

bool st25tb_verify(St25tbData* data, const FuriString* device_type) {
    furi_check(device_type);
    UNUSED(data);

    return furi_string_equal_str(device_type, ST25TB_PROTOCOL_NAME);
}

bool st25tb_load(St25tbData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    bool parsed = false;

    FuriString* temp_str = furi_string_alloc();

    do {
        if(version < NFC_UNIFIED_FORMAT_VERSION) break;
        if(!flipper_format_read_string(ff, ST25TB_TYPE_KEY, temp_str)) break;

        bool type_parsed = false;
        for(size_t i = 0; i < St25tbTypeNum; i++) {
            if(furi_string_equal_str(temp_str, st25tb_features[i].type_name)) {
                data->type = i;
                type_parsed = true;
            }
        }
        if(!type_parsed) break;

        bool blocks_parsed = true;
        for(uint8_t block = 0; block < st25tb_features[data->type].blocks_total; block++) {
            furi_string_printf(temp_str, ST25TB_BLOCK_KEY, block);
            if(!flipper_format_read_hex(
                   ff, furi_string_get_cstr(temp_str), (uint8_t*)&data->blocks[block], 4)) {
                blocks_parsed = false;
                break;
            }
        }
        if(!blocks_parsed) break;

        if(!flipper_format_read_hex(
               ff, ST25TB_SYSTEM_BLOCK_KEY, (uint8_t*)&data->system_otp_block, 4))
            break;

        parsed = true;
    } while(false);

    furi_string_free(temp_str);

    return parsed;
}

bool st25tb_save(const St25tbData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    FuriString* temp_str = furi_string_alloc();
    bool saved = false;

    do {
        if(!flipper_format_write_comment_cstr(ff, ST25TB_PROTOCOL_NAME " specific data")) break;
        if(!flipper_format_write_string_cstr(
               ff, ST25TB_TYPE_KEY, st25tb_features[data->type].type_name))
            break;

        bool blocks_saved = true;
        for(uint8_t block = 0; block < st25tb_features[data->type].blocks_total; block++) {
            furi_string_printf(temp_str, ST25TB_BLOCK_KEY, block);
            if(!flipper_format_write_hex(
                   ff, furi_string_get_cstr(temp_str), (uint8_t*)&data->blocks[block], 4)) {
                blocks_saved = false;
                break;
            }
        }
        if(!blocks_saved) break;

        if(!flipper_format_write_hex(
               ff, ST25TB_SYSTEM_BLOCK_KEY, (uint8_t*)&data->system_otp_block, 4))
            break;

        saved = true;
    } while(false);

    furi_string_free(temp_str);
    return saved;
}

bool st25tb_is_equal(const St25tbData* data, const St25tbData* other) {
    furi_check(data);
    furi_check(other);

    return memcmp(data, other, sizeof(St25tbData)) == 0; //-V1103
}

uint8_t st25tb_get_block_count(St25tbType type) {
    furi_check(type < St25tbTypeNum);

    return st25tb_features[type].blocks_total;
}

const char* st25tb_get_device_name(const St25tbData* data, NfcDeviceNameType name_type) {
    furi_check(data);
    furi_check(data->type < St25tbTypeNum);

    if(name_type == NfcDeviceNameTypeFull) {
        return st25tb_features[data->type].full_name;
    } else {
        return st25tb_features[data->type].type_name;
    }
}

const uint8_t* st25tb_get_uid(const St25tbData* data, size_t* uid_len) {
    furi_check(data);

    if(uid_len) {
        *uid_len = ST25TB_UID_SIZE;
    }

    return data->uid;
}

bool st25tb_set_uid(St25tbData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);
    furi_check(uid);

    const bool uid_valid = uid_len == ST25TB_UID_SIZE;

    if(uid_valid) {
        memcpy(data->uid, uid, uid_len);
    }

    return uid_valid;
}

St25tbData* st25tb_get_base_data(const St25tbData* data) {
    UNUSED(data);
    furi_crash("No base data");
}

St25tbType st25tb_get_type_from_uid(const uint8_t* uid) {
    furi_check(uid);

    switch(uid[2] >> 2) {
    case 0x0:
    case 0x3:
        return St25tbTypeX4k;
    case 0x4:
        return St25tbTypeX512;
    case 0x6:
        return St25tbType512Ac;
    case 0x7:
        return St25tbType04k;
    case 0xc:
        return St25tbType512At;
    case 0xf:
        return St25tbType02k;
    default:
        furi_crash("unsupported st25tb type");
    }
}
