#include "ntag4xx_i.h"

#include <furi.h>

#define NTAG4XX_PROTOCOL_NAME "NTAG4xx"

#define NTAG4XX_HW_MAJOR_TYPE_413_DNA (0x10)
#define NTAG4XX_HW_MAJOR_TYPE_424_DNA (0x30)

#define NTAG4XX_HW_SUBTYPE_TAGTAMPER_FLAG (0x08)

static const char* ntag4xx_type_strings[] = {
    [Ntag4xxType413DNA] = "NTAG413 DNA",
    [Ntag4xxType424DNA] = "NTAG424 DNA",
    [Ntag4xxType424DNATT] = "NTAG424 DNA TagTamper",
    [Ntag4xxType426QDNA] = "NTAG426Q DNA",
    [Ntag4xxType426QDNATT] = "NTAG426Q DNA TagTamper",
    [Ntag4xxTypeUnknown] = "UNK",
};

const NfcDeviceBase nfc_device_ntag4xx = {
    .protocol_name = NTAG4XX_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)ntag4xx_alloc,
    .free = (NfcDeviceFree)ntag4xx_free,
    .reset = (NfcDeviceReset)ntag4xx_reset,
    .copy = (NfcDeviceCopy)ntag4xx_copy,
    .verify = (NfcDeviceVerify)ntag4xx_verify,
    .load = (NfcDeviceLoad)ntag4xx_load,
    .save = (NfcDeviceSave)ntag4xx_save,
    .is_equal = (NfcDeviceEqual)ntag4xx_is_equal,
    .get_name = (NfcDeviceGetName)ntag4xx_get_device_name,
    .get_uid = (NfcDeviceGetUid)ntag4xx_get_uid,
    .set_uid = (NfcDeviceSetUid)ntag4xx_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)ntag4xx_get_base_data,
};

Ntag4xxData* ntag4xx_alloc(void) {
    Ntag4xxData* data = malloc(sizeof(Ntag4xxData));
    data->iso14443_4a_data = iso14443_4a_alloc();
    data->device_name = furi_string_alloc();
    return data;
}

void ntag4xx_free(Ntag4xxData* data) {
    furi_check(data);

    ntag4xx_reset(data);
    iso14443_4a_free(data->iso14443_4a_data);
    furi_string_free(data->device_name);
    free(data);
}

void ntag4xx_reset(Ntag4xxData* data) {
    furi_check(data);

    iso14443_4a_reset(data->iso14443_4a_data);

    memset(&data->version, 0, sizeof(Ntag4xxVersion));
}

void ntag4xx_copy(Ntag4xxData* data, const Ntag4xxData* other) {
    furi_check(data);
    furi_check(other);

    ntag4xx_reset(data);

    iso14443_4a_copy(data->iso14443_4a_data, other->iso14443_4a_data);

    data->version = other->version;
}

bool ntag4xx_verify(Ntag4xxData* data, const FuriString* device_type) {
    UNUSED(data);
    UNUSED(device_type);

    return false;
}

bool ntag4xx_load(Ntag4xxData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    FuriString* prefix = furi_string_alloc();

    bool success = false;

    do {
        if(!iso14443_4a_load(data->iso14443_4a_data, ff, version)) break;

        if(!ntag4xx_version_load(&data->version, ff)) break;

        success = true;
    } while(false);

    furi_string_free(prefix);
    return success;
}

bool ntag4xx_save(const Ntag4xxData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    FuriString* prefix = furi_string_alloc();

    bool success = false;

    do {
        if(!iso14443_4a_save(data->iso14443_4a_data, ff)) break;

        if(!flipper_format_write_comment_cstr(ff, NTAG4XX_PROTOCOL_NAME " specific data")) break;
        if(!ntag4xx_version_save(&data->version, ff)) break;

        success = true;
    } while(false);

    furi_string_free(prefix);
    return success;
}

bool ntag4xx_is_equal(const Ntag4xxData* data, const Ntag4xxData* other) {
    furi_check(data);
    furi_check(other);

    return iso14443_4a_is_equal(data->iso14443_4a_data, other->iso14443_4a_data) &&
           memcmp(&data->version, &other->version, sizeof(Ntag4xxVersion)) == 0;
}

Ntag4xxType ntag4xx_get_type_from_version(const Ntag4xxVersion* const version) {
    Ntag4xxType type = Ntag4xxTypeUnknown;

    switch(version->hw_major) {
    case NTAG4XX_HW_MAJOR_TYPE_413_DNA:
        type = Ntag4xxType413DNA;
        break;
    case NTAG4XX_HW_MAJOR_TYPE_424_DNA:
        if(version->hw_subtype & NTAG4XX_HW_SUBTYPE_TAGTAMPER_FLAG) {
            type = Ntag4xxType424DNATT;
        } else {
            type = Ntag4xxType424DNA;
        }
        break;
    // TODO: there is no info online or in other implementations (NXP TagInfo, NFC Tools, Proxmark3)
    // about what the HWMajorVersion is supposed to be for NTAG426Q DNA, and they don't seem to be for sale
    // case NTAG4XX_HW_MAJOR_TYPE_426Q_DNA:
    //     if(version->hw_subtype & NTAG4XX_HW_SUBTYPE_TAGTAMPER_FLAG) {
    //         type = Ntag4xxType426QDNATT;
    //     } else {
    //         type = Ntag4xxType426QDNA;
    //     }
    //     break;
    default:
        break;
    }

    return type;
}

const char* ntag4xx_get_device_name(const Ntag4xxData* data, NfcDeviceNameType name_type) {
    furi_check(data);

    const Ntag4xxType type = ntag4xx_get_type_from_version(&data->version);

    if(type == Ntag4xxTypeUnknown) {
        furi_string_printf(data->device_name, "Unknown %s", NTAG4XX_PROTOCOL_NAME);
    } else {
        furi_string_printf(data->device_name, "%s", ntag4xx_type_strings[type]);
        if(name_type == NfcDeviceNameTypeShort) {
            furi_string_replace(data->device_name, "TagTamper", "TT");
        }
    }

    return furi_string_get_cstr(data->device_name);
}

const uint8_t* ntag4xx_get_uid(const Ntag4xxData* data, size_t* uid_len) {
    furi_check(data);
    furi_check(uid_len);

    return iso14443_4a_get_uid(data->iso14443_4a_data, uid_len);
}

bool ntag4xx_set_uid(Ntag4xxData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);

    return iso14443_4a_set_uid(data->iso14443_4a_data, uid, uid_len);
}

Iso14443_4aData* ntag4xx_get_base_data(const Ntag4xxData* data) {
    furi_check(data);

    return data->iso14443_4a_data;
}
