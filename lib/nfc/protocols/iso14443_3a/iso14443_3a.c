#include "iso14443_3a.h"

#include <furi.h>
#include <nfc/nfc_common.h>

#define ISO14443A_ATS_BIT (1U << 5)

#define ISO14443_3A_PROTOCOL_NAME_LEGACY "UID"
#define ISO14443_3A_PROTOCOL_NAME        "ISO14443-3A"
#define ISO14443_3A_DEVICE_NAME          "ISO14443-3A (Unknown)"

#define ISO14443_3A_ATQA_KEY "ATQA"
#define ISO14443_3A_SAK_KEY  "SAK"

const NfcDeviceBase nfc_device_iso14443_3a = {
    .protocol_name = ISO14443_3A_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)iso14443_3a_alloc,
    .free = (NfcDeviceFree)iso14443_3a_free,
    .reset = (NfcDeviceReset)iso14443_3a_reset,
    .copy = (NfcDeviceCopy)iso14443_3a_copy,
    .verify = (NfcDeviceVerify)iso14443_3a_verify,
    .load = (NfcDeviceLoad)iso14443_3a_load,
    .save = (NfcDeviceSave)iso14443_3a_save,
    .is_equal = (NfcDeviceEqual)iso14443_3a_is_equal,
    .get_name = (NfcDeviceGetName)iso14443_3a_get_device_name,
    .get_uid = (NfcDeviceGetUid)iso14443_3a_get_uid,
    .set_uid = (NfcDeviceSetUid)iso14443_3a_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)iso14443_3a_get_base_data,
};

Iso14443_3aData* iso14443_3a_alloc(void) {
    Iso14443_3aData* data = malloc(sizeof(Iso14443_3aData));
    return data;
}

void iso14443_3a_free(Iso14443_3aData* data) {
    furi_check(data);

    free(data);
}

void iso14443_3a_reset(Iso14443_3aData* data) {
    furi_check(data);

    memset(data, 0, sizeof(Iso14443_3aData));
}

void iso14443_3a_copy(Iso14443_3aData* data, const Iso14443_3aData* other) {
    furi_check(data);
    furi_check(other);

    *data = *other;
}

bool iso14443_3a_verify(Iso14443_3aData* data, const FuriString* device_type) {
    UNUSED(data);
    furi_check(device_type);

    return furi_string_equal(device_type, ISO14443_3A_PROTOCOL_NAME_LEGACY);
}

bool iso14443_3a_load(Iso14443_3aData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    bool parsed = false;

    do {
        // Common to all format versions
        if(!flipper_format_read_hex(ff, ISO14443_3A_ATQA_KEY, data->atqa, 2)) break;
        if(!flipper_format_read_hex(ff, ISO14443_3A_SAK_KEY, &data->sak, 1)) break;

        if(version > NFC_LSB_ATQA_FORMAT_VERSION) {
            // Swap ATQA bytes for newer versions
            FURI_SWAP(data->atqa[0], data->atqa[1]);
        }

        parsed = true;
    } while(false);

    return parsed;
}

bool iso14443_3a_save(const Iso14443_3aData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    bool saved = false;

    do {
        // Save ATQA in MSB order for correct companion apps display
        const uint8_t atqa[2] = {data->atqa[1], data->atqa[0]};
        if(!flipper_format_write_comment_cstr(ff, ISO14443_3A_PROTOCOL_NAME " specific data"))
            break;

        // Write ATQA and SAK
        if(!flipper_format_write_hex(ff, ISO14443_3A_ATQA_KEY, atqa, 2)) break;
        if(!flipper_format_write_hex(ff, ISO14443_3A_SAK_KEY, &data->sak, 1)) break;
        saved = true;
    } while(false);

    return saved;
}

bool iso14443_3a_is_equal(const Iso14443_3aData* data, const Iso14443_3aData* other) {
    furi_check(data);
    furi_check(other);

    return memcmp(data, other, sizeof(Iso14443_3aData)) == 0;
}

const char* iso14443_3a_get_device_name(const Iso14443_3aData* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);
    return ISO14443_3A_DEVICE_NAME;
}

const uint8_t* iso14443_3a_get_uid(const Iso14443_3aData* data, size_t* uid_len) {
    furi_check(data);

    if(uid_len) {
        *uid_len = data->uid_len;
    }

    return data->uid;
}

bool iso14443_3a_set_uid(Iso14443_3aData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);
    furi_check(uid);

    const bool uid_valid = uid_len == ISO14443_3A_UID_4_BYTES ||
                           uid_len == ISO14443_3A_UID_7_BYTES ||
                           uid_len == ISO14443_3A_UID_10_BYTES;

    if(uid_valid) {
        memcpy(data->uid, uid, uid_len);
        data->uid_len = uid_len;
    }

    return uid_valid;
}

Iso14443_3aData* iso14443_3a_get_base_data(const Iso14443_3aData* data) {
    UNUSED(data);
    furi_crash("No base data");
}

uint32_t iso14443_3a_get_cuid(const Iso14443_3aData* data) {
    furi_check(data);

    uint32_t cuid = 0;
    const uint8_t* cuid_start = data->uid;
    if(data->uid_len == ISO14443_3A_UID_7_BYTES) {
        cuid_start = &data->uid[3];
    }
    cuid = (cuid_start[0] << 24) | (cuid_start[1] << 16) | (cuid_start[2] << 8) | (cuid_start[3]);

    return cuid;
}

bool iso14443_3a_supports_iso14443_4(const Iso14443_3aData* data) {
    furi_check(data);

    return data->sak & ISO14443A_ATS_BIT;
}

uint8_t iso14443_3a_get_sak(const Iso14443_3aData* data) {
    furi_check(data);

    return data->sak;
}

void iso14443_3a_get_atqa(const Iso14443_3aData* data, uint8_t atqa[2]) {
    furi_check(data);
    furi_check(atqa);

    memcpy(atqa, data->atqa, sizeof(data->atqa));
}

void iso14443_3a_set_sak(Iso14443_3aData* data, uint8_t sak) {
    furi_check(data);

    data->sak = sak;
}

void iso14443_3a_set_atqa(Iso14443_3aData* data, const uint8_t atqa[2]) {
    furi_check(data);
    furi_check(atqa);

    memcpy(data->atqa, atqa, sizeof(data->atqa));
}
