#include "iso14443_4b_i.h" // IWYU pragma: keep

#include <furi.h>
#include <nfc/protocols/nfc_device_base_i.h>

#define ISO14443_4B_PROTOCOL_NAME "ISO14443-4B"
#define ISO14443_4B_DEVICE_NAME   "ISO14443-4B (Unknown)"

const NfcDeviceBase nfc_device_iso14443_4b = {
    .protocol_name = ISO14443_4B_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)iso14443_4b_alloc,
    .free = (NfcDeviceFree)iso14443_4b_free,
    .reset = (NfcDeviceReset)iso14443_4b_reset,
    .copy = (NfcDeviceCopy)iso14443_4b_copy,
    .verify = (NfcDeviceVerify)iso14443_4b_verify,
    .load = (NfcDeviceLoad)iso14443_4b_load,
    .save = (NfcDeviceSave)iso14443_4b_save,
    .is_equal = (NfcDeviceEqual)iso14443_4b_is_equal,
    .get_name = (NfcDeviceGetName)iso14443_4b_get_device_name,
    .get_uid = (NfcDeviceGetUid)iso14443_4b_get_uid,
    .set_uid = (NfcDeviceSetUid)iso14443_4b_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)iso14443_4b_get_base_data,
};

Iso14443_4bData* iso14443_4b_alloc(void) {
    Iso14443_4bData* data = malloc(sizeof(Iso14443_4bData));

    data->iso14443_3b_data = iso14443_3b_alloc();
    return data;
}

void iso14443_4b_free(Iso14443_4bData* data) {
    furi_check(data);

    iso14443_3b_free(data->iso14443_3b_data);
    free(data);
}

void iso14443_4b_reset(Iso14443_4bData* data) {
    furi_check(data);

    iso14443_3b_reset(data->iso14443_3b_data);
}

void iso14443_4b_copy(Iso14443_4bData* data, const Iso14443_4bData* other) {
    furi_check(data);
    furi_check(other);

    iso14443_3b_copy(data->iso14443_3b_data, other->iso14443_3b_data);
}

bool iso14443_4b_verify(Iso14443_4bData* data, const FuriString* device_type) {
    UNUSED(data);
    UNUSED(device_type);

    // Empty, unified file format only
    return false;
}

bool iso14443_4b_load(Iso14443_4bData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);
    return iso14443_3b_load(data->iso14443_3b_data, ff, version);
}

bool iso14443_4b_save(const Iso14443_4bData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);
    return iso14443_3b_save(data->iso14443_3b_data, ff);
}

bool iso14443_4b_is_equal(const Iso14443_4bData* data, const Iso14443_4bData* other) {
    furi_check(data);
    furi_check(other);

    return iso14443_3b_is_equal(data->iso14443_3b_data, other->iso14443_3b_data);
}

const char* iso14443_4b_get_device_name(const Iso14443_4bData* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);
    return ISO14443_4B_DEVICE_NAME;
}

const uint8_t* iso14443_4b_get_uid(const Iso14443_4bData* data, size_t* uid_len) {
    furi_check(data);
    furi_check(uid_len);

    return iso14443_3b_get_uid(data->iso14443_3b_data, uid_len);
}

bool iso14443_4b_set_uid(Iso14443_4bData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);
    furi_check(uid);

    return iso14443_3b_set_uid(data->iso14443_3b_data, uid, uid_len);
}

Iso14443_3bData* iso14443_4b_get_base_data(const Iso14443_4bData* data) {
    furi_check(data);

    return data->iso14443_3b_data;
}
