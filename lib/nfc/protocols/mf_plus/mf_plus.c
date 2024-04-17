#include "mf_plus.h"

#include <bit_lib/bit_lib.h>
#include <furi.h>

#define MF_PLUS_PROTOCOL_NAME "Mifare Plus"

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

MfPlusData* mf_plus_alloc() {
    MfPlusData* data = malloc(sizeof(MfPlusData));
    data->iso14443_4a_data = iso14443_4a_alloc();
    return data;
}

void mf_plus_free(MfPlusData* data) {
    furi_check(data);
    iso14443_4a_free(data->iso14443_4a_data);
    free(data);
}

void mf_plus_reset(MfPlusData* data) {
    furi_check(data);
    iso14443_4a_reset(data->iso14443_4a_data);
}

void mf_plus_copy(MfPlusData* data, const MfPlusData* other) {
    furi_check(data);
    furi_check(other);
    iso14443_4a_copy(data->iso14443_4a_data, other->iso14443_4a_data);
}

bool mf_plus_verify(MfPlusData* data, const FuriString* device_type) {
    UNUSED(data);
    return furi_string_equal_str(device_type, MF_PLUS_PROTOCOL_NAME);
}

bool mf_plus_load(MfPlusData* data, FlipperFormat* ff, uint32_t version) {
    // TODO
    UNUSED(data);
    UNUSED(ff);
    UNUSED(version);
    return true;
}

bool mf_plus_save(const MfPlusData* data, FlipperFormat* ff) {
    // TODO
    UNUSED(data);
    UNUSED(ff);
    return true;
}

bool mf_plus_is_equal(const MfPlusData* data, const MfPlusData* other) {
    furi_check(data);
    furi_check(other);

    bool equal = false;

    do {
        if(!iso14443_4a_is_equal(data->iso14443_4a_data, other->iso14443_4a_data)) break;

        equal = true;
    } while(false);

    return equal;
}

const char* mf_plus_get_device_name(const MfPlusData* data, NfcDeviceNameType name_type) {
    furi_check(data);

    const char* name = NULL;

    do {
        if(name_type == NfcDeviceNameTypeFull) {
            name = "Mifare Plus";
        } else if(name_type == NfcDeviceNameTypeShort) {
            name = "Mifare Plus";
        } else {
            break;
        }
    } while(false);

    return name;
}

const uint8_t* mf_plus_get_uid(const MfPlusData* data, size_t* uid_len) {
    furi_assert(data);

    return iso14443_4a_get_uid(data->iso14443_4a_data, uid_len);
}

bool mf_plus_set_uid(MfPlusData* data, const uint8_t* uid, size_t uid_len) {
    furi_assert(data);

    return iso14443_4a_set_uid(data->iso14443_4a_data, uid, uid_len);
}
Iso14443_4aData* mf_plus_get_base_data(const MfPlusData* data) {
    furi_check(data);
    return data->iso14443_4a_data;
}