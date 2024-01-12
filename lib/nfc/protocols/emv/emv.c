//#include "emv_i.h"

#include <core/common_defines.h>
#include "protocols/emv/emv.h"
#include <furi.h>
#include <stdlib.h>
#include <string.h>

#define EMV_PROTOCOL_NAME "EMV"

const NfcDeviceBase nfc_device_emv = {
    .protocol_name = EMV_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)emv_alloc,
    .free = (NfcDeviceFree)emv_free,
    .reset = (NfcDeviceReset)emv_reset,
    .copy = (NfcDeviceCopy)emv_copy,
    .verify = (NfcDeviceVerify)emv_verify,
    .load = (NfcDeviceLoad)emv_load,
    .save = (NfcDeviceSave)emv_save,
    .is_equal = (NfcDeviceEqual)emv_is_equal,
    .get_name = (NfcDeviceGetName)emv_get_device_name,
    .get_uid = (NfcDeviceGetUid)emv_get_uid,
    .set_uid = (NfcDeviceSetUid)emv_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)emv_get_base_data,
};

EmvData* emv_alloc() {
    EmvData* data = malloc(sizeof(EmvData));
    data->iso14443_4a_data = iso14443_4a_alloc();

    return data;
}

void emv_free(EmvData* data) {
    furi_assert(data);

    emv_reset(data);
    iso14443_4a_free(data->iso14443_4a_data);
    free(data);
}

void emv_reset(EmvData* data) {
    furi_assert(data);

    iso14443_4a_reset(data->iso14443_4a_data);

    memset(&data->emv_application, 0, sizeof(EmvApplication));
}

void emv_copy(EmvData* destination, const EmvData* source) {
    furi_assert(destination);
    furi_assert(source);

    emv_reset(destination);

    iso14443_4a_copy(destination->iso14443_4a_data, source->iso14443_4a_data);
    destination->emv_application = source->emv_application;
}

bool emv_verify(EmvData* data, const FuriString* device_type) {
    UNUSED(data);
    return furi_string_equal_str(device_type, EMV_PROTOCOL_NAME);
}

bool emv_load(EmvData* data, FlipperFormat* ff, uint32_t version) {
    furi_assert(data);
    UNUSED(data);
    UNUSED(ff);
    UNUSED(version);

    return false;
}

bool emv_save(const EmvData* data, FlipperFormat* ff) {
    furi_assert(data);
    UNUSED(data);
    UNUSED(ff);

    return false;
}

bool emv_is_equal(const EmvData* data, const EmvData* other) {
    furi_assert(data);
    furi_assert(other);

    return iso14443_4a_is_equal(data->iso14443_4a_data, other->iso14443_4a_data) &&
           memcmp(&data->emv_application, &other->emv_application, sizeof(EmvApplication)) == 0;
}

const char* emv_get_device_name(const EmvData* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);
    return EMV_PROTOCOL_NAME;
}

const uint8_t* emv_get_uid(const EmvData* data, size_t* uid_len) {
    furi_assert(data);

    return iso14443_4a_get_uid(data->iso14443_4a_data, uid_len);
}

bool emv_set_uid(EmvData* data, const uint8_t* uid, size_t uid_len) {
    furi_assert(data);

    return iso14443_4a_set_uid(data->iso14443_4a_data, uid, uid_len);
}

Iso14443_4aData* emv_get_base_data(const EmvData* data) {
    furi_assert(data);

    return data->iso14443_4a_data;
}