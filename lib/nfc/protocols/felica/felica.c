#include "felica.h"

#include <furi.h>

#include <nfc/nfc_common.h>

#define FELICA_PROTOCOL_NAME "FeliCa"
#define FELICA_DEVICE_NAME "FeliCa"

#define FELICA_DATA_FORMAT_VERSION "Data format version"
#define FELICA_MANUFACTURE_ID "Manufacture id"
#define FELICA_MANUFACTURE_PARAMETER "Manufacture parameter"

static const uint32_t felica_data_format_version = 1;

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

FelicaData* felica_alloc() {
    FelicaData* data = malloc(sizeof(FelicaData));
    return data;
}

void felica_free(FelicaData* data) {
    furi_assert(data);

    free(data);
}

void felica_reset(FelicaData* data) {
    memset(data, 0, sizeof(FelicaData));
}

void felica_copy(FelicaData* data, const FelicaData* other) {
    furi_assert(data);
    furi_assert(other);

    *data = *other;
}

bool felica_verify(FelicaData* data, const FuriString* device_type) {
    UNUSED(data);
    UNUSED(device_type);

    return false;
}

bool felica_load(FelicaData* data, FlipperFormat* ff, uint32_t version) {
    furi_assert(data);

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
    } while(false);

    return parsed;
}

bool felica_save(const FelicaData* data, FlipperFormat* ff) {
    furi_assert(data);

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

        saved = true;
    } while(false);

    return saved;
}

bool felica_is_equal(const FelicaData* data, const FelicaData* other) {
    furi_assert(data);
    furi_assert(other);

    return memcmp(data, other, sizeof(FelicaData)) == 0;
}

const char* felica_get_device_name(const FelicaData* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);

    return FELICA_DEVICE_NAME;
}

const uint8_t* felica_get_uid(const FelicaData* data, size_t* uid_len) {
    furi_assert(data);

    // Consider Manufacturer ID as UID
    if(uid_len) {
        *uid_len = FELICA_IDM_SIZE;
    }

    return data->idm.data;
}

bool felica_set_uid(FelicaData* data, const uint8_t* uid, size_t uid_len) {
    furi_assert(data);

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
