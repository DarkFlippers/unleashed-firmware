#include "mf_plus_i.h"

#include <bit_lib/bit_lib.h>
#include <furi.h>

#define MF_PLUS_PROTOCOL_NAME "Mifare Plus"

static const char* mf_plus_type_strings[] = {
    [MfPlusTypeS] = "Plus S",
    [MfPlusTypeX] = "Plus X",
    [MfPlusTypeSE] = "Plus SE",
    [MfPlusTypeEV1] = "Plus EV1",
    [MfPlusTypeEV2] = "Plus EV2",
    [MfPlusTypePlus] = "Plus",
    [MfPlusTypeUnknown] = "Unknown",
};

static const char* mf_plus_size_strings[] = {
    [MfPlusSize1K] = "1K",
    [MfPlusSize2K] = "2K",
    [MfPlusSize4K] = "4K",
    [MfPlusSizeUnknown] = "Unknown",
};

static const char* mf_plus_security_level_strings[] = {
    [MfPlusSecurityLevel0] = "SL0",
    [MfPlusSecurityLevel1] = "SL1",
    [MfPlusSecurityLevel2] = "SL2",
    [MfPlusSecurityLevel3] = "SL3",
    [MfPlusSecurityLevelUnknown] = "Unknown",
};

const NfcDeviceBase nfc_device_mf_plus = {
    .protocol_name = MF_PLUS_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)mf_plus_alloc,
    .free = (NfcDeviceFree)mf_plus_free,
    .reset = (NfcDeviceReset)mf_plus_reset,
    .copy = (NfcDeviceCopy)mf_plus_copy,
    .verify = (NfcDeviceVerify)mf_plus_verify,
    .load = (NfcDeviceLoad)mf_plus_load,
    .save = (NfcDeviceSave)mf_plus_save,
    .is_equal = (NfcDeviceEqual)mf_plus_is_equal,
    .get_name = (NfcDeviceGetName)mf_plus_get_device_name,
    .get_uid = (NfcDeviceGetUid)mf_plus_get_uid,
    .set_uid = (NfcDeviceSetUid)mf_plus_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)mf_plus_get_base_data,
};

MfPlusData* mf_plus_alloc(void) {
    MfPlusData* data = malloc(sizeof(MfPlusData));
    data->device_name = furi_string_alloc();
    data->iso14443_4a_data = iso14443_4a_alloc();

    data->type = MfPlusTypeUnknown;
    data->security_level = MfPlusSecurityLevelUnknown;
    data->size = MfPlusSizeUnknown;

    return data;
}

void mf_plus_free(MfPlusData* data) {
    furi_check(data);

    furi_string_free(data->device_name);
    iso14443_4a_free(data->iso14443_4a_data);
    free(data);
}

void mf_plus_reset(MfPlusData* data) {
    furi_check(data);

    iso14443_4a_reset(data->iso14443_4a_data);
    memset(&data->version, 0, sizeof(data->version));
    furi_string_reset(data->device_name);
    data->type = MfPlusTypeUnknown;
    data->security_level = MfPlusSecurityLevelUnknown;
    data->size = MfPlusSizeUnknown;
}

void mf_plus_copy(MfPlusData* data, const MfPlusData* other) {
    furi_check(data);
    furi_check(other);

    iso14443_4a_copy(data->iso14443_4a_data, other->iso14443_4a_data);
    data->version = other->version;
    data->type = other->type;
    data->security_level = other->security_level;
    data->size = other->size;
}

bool mf_plus_verify(MfPlusData* data, const FuriString* device_type) {
    UNUSED(data);
    return furi_string_equal_str(device_type, MF_PLUS_PROTOCOL_NAME);
}

bool mf_plus_load(MfPlusData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);

    bool success = false;
    do {
        if(!iso14443_4a_load(data->iso14443_4a_data, ff, version)) break;
        if(!mf_plus_version_load(&data->version, ff)) break;
        if(!mf_plus_type_load(&data->type, ff)) break;
        if(!mf_plus_security_level_load(&data->security_level, ff)) break;
        if(!mf_plus_size_load(&data->size, ff)) break;
        success = true;
    } while(false);

    return success;
}

bool mf_plus_save(const MfPlusData* data, FlipperFormat* ff) {
    furi_check(data);

    bool success = false;
    do {
        if(!iso14443_4a_save(data->iso14443_4a_data, ff)) break;
        if(!flipper_format_write_comment_cstr(ff, MF_PLUS_PROTOCOL_NAME " specific data")) break;
        if(!mf_plus_version_save(&data->version, ff)) break;
        if(!mf_plus_type_save(&data->type, ff)) break;
        if(!mf_plus_security_level_save(&data->security_level, ff)) break;
        if(!mf_plus_size_save(&data->size, ff)) break;
        success = true;
    } while(false);

    return success;
}

bool mf_plus_is_equal(const MfPlusData* data, const MfPlusData* other) {
    furi_check(data);
    furi_check(other);

    bool equal = false;
    do {
        if(!iso14443_4a_is_equal(data->iso14443_4a_data, other->iso14443_4a_data)) break;
        if(memcmp(&data->version, &other->version, sizeof(data->version)) != 0) break;
        if(data->security_level != other->security_level) break;
        if(data->type != other->type) break;
        if(data->size != other->size) break;
        equal = true;
    } while(false);

    return equal;
}

const char* mf_plus_get_device_name(const MfPlusData* data, NfcDeviceNameType name_type) {
    furi_check(data);

    if(name_type == NfcDeviceNameTypeFull) {
        furi_string_printf(
            data->device_name,
            "Mifare %s %s %s",
            mf_plus_type_strings[data->type], // Includes "Plus" for regular Mifare Plus cards
            mf_plus_size_strings[data->size],
            mf_plus_security_level_strings[data->security_level]);
    } else if(name_type == NfcDeviceNameTypeShort) {
        furi_string_set_str(data->device_name, MF_PLUS_PROTOCOL_NAME);
    } else {
        furi_crash("Unexpected name type");
    }

    return furi_string_get_cstr(data->device_name);
}

const uint8_t* mf_plus_get_uid(const MfPlusData* data, size_t* uid_len) {
    furi_check(data);

    return iso14443_4a_get_uid(data->iso14443_4a_data, uid_len);
}

bool mf_plus_set_uid(MfPlusData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);

    return iso14443_4a_set_uid(data->iso14443_4a_data, uid, uid_len);
}
Iso14443_4aData* mf_plus_get_base_data(const MfPlusData* data) {
    furi_check(data);

    return data->iso14443_4a_data;
}
