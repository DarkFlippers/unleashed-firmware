#include "iso14443_3b_i.h"

#include <furi.h>
#include <nfc/protocols/nfc_device_base_i.h>

#include <nfc/nfc_common.h>
#include <nfc/helpers/iso14443_crc.h>

#define ISO14443_3B_PROTOCOL_NAME "ISO14443-3B"
#define ISO14443_3B_DEVICE_NAME   "ISO14443-3B (Unknown)"

#define ISO14443_3B_APP_DATA_KEY      "Application data"
#define ISO14443_3B_PROTOCOL_INFO_KEY "Protocol info"

#define ISO14443_3B_FDT_POLL_DEFAULT_FC (ISO14443_3B_FDT_POLL_FC)

const NfcDeviceBase nfc_device_iso14443_3b = {
    .protocol_name = ISO14443_3B_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)iso14443_3b_alloc,
    .free = (NfcDeviceFree)iso14443_3b_free,
    .reset = (NfcDeviceReset)iso14443_3b_reset,
    .copy = (NfcDeviceCopy)iso14443_3b_copy,
    .verify = (NfcDeviceVerify)iso14443_3b_verify,
    .load = (NfcDeviceLoad)iso14443_3b_load,
    .save = (NfcDeviceSave)iso14443_3b_save,
    .is_equal = (NfcDeviceEqual)iso14443_3b_is_equal,
    .get_name = (NfcDeviceGetName)iso14443_3b_get_device_name,
    .get_uid = (NfcDeviceGetUid)iso14443_3b_get_uid,
    .set_uid = (NfcDeviceSetUid)iso14443_3b_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)iso14443_3b_get_base_data,
};

Iso14443_3bData* iso14443_3b_alloc(void) {
    Iso14443_3bData* data = malloc(sizeof(Iso14443_3bData));
    return data;
}

void iso14443_3b_free(Iso14443_3bData* data) {
    furi_check(data);

    free(data);
}

void iso14443_3b_reset(Iso14443_3bData* data) {
    furi_check(data);

    memset(data, 0, sizeof(Iso14443_3bData));
}

void iso14443_3b_copy(Iso14443_3bData* data, const Iso14443_3bData* other) {
    furi_check(data);
    furi_check(other);

    *data = *other;
}

bool iso14443_3b_verify(Iso14443_3bData* data, const FuriString* device_type) {
    UNUSED(data);
    UNUSED(device_type);
    // No support for old ISO14443-3B
    return false;
}

bool iso14443_3b_load(Iso14443_3bData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    bool parsed = false;

    do {
        if(version < NFC_UNIFIED_FORMAT_VERSION) break;

        if(!flipper_format_read_hex(
               ff, ISO14443_3B_APP_DATA_KEY, data->app_data, ISO14443_3B_APP_DATA_SIZE))
            break;
        if(!flipper_format_read_hex(
               ff,
               ISO14443_3B_PROTOCOL_INFO_KEY,
               (uint8_t*)&data->protocol_info,
               sizeof(Iso14443_3bProtocolInfo)))
            break;

        parsed = true;
    } while(false);

    return parsed;
}

bool iso14443_3b_save(const Iso14443_3bData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    bool saved = false;

    do {
        if(!flipper_format_write_comment_cstr(ff, ISO14443_3B_PROTOCOL_NAME " specific data"))
            break;
        if(!flipper_format_write_hex(
               ff, ISO14443_3B_APP_DATA_KEY, data->app_data, ISO14443_3B_APP_DATA_SIZE))
            break;
        if(!flipper_format_write_hex(
               ff,
               ISO14443_3B_PROTOCOL_INFO_KEY,
               (uint8_t*)&data->protocol_info,
               sizeof(Iso14443_3bProtocolInfo)))
            break;
        saved = true;
    } while(false);

    return saved;
}

bool iso14443_3b_is_equal(const Iso14443_3bData* data, const Iso14443_3bData* other) {
    furi_check(data);
    furi_check(other);

    return memcmp(data, other, sizeof(Iso14443_3bData)) == 0;
}

const char* iso14443_3b_get_device_name(const Iso14443_3bData* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);

    return ISO14443_3B_DEVICE_NAME;
}

const uint8_t* iso14443_3b_get_uid(const Iso14443_3bData* data, size_t* uid_len) {
    furi_check(data);
    furi_check(uid_len);

    *uid_len = ISO14443_3B_UID_SIZE;
    return data->uid;
}

bool iso14443_3b_set_uid(Iso14443_3bData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);
    furi_check(uid);

    const bool uid_valid = uid_len == ISO14443_3B_UID_SIZE;

    if(uid_valid) {
        memcpy(data->uid, uid, uid_len);
    }

    return uid_valid;
}

Iso14443_3bData* iso14443_3b_get_base_data(const Iso14443_3bData* data) {
    UNUSED(data);
    furi_crash("No base data");
}

bool iso14443_3b_supports_iso14443_4(const Iso14443_3bData* data) {
    furi_check(data);

    return data->protocol_info.protocol_type == 0x01;
}

bool iso14443_3b_supports_bit_rate(const Iso14443_3bData* data, Iso14443_3bBitRate bit_rate) {
    furi_check(data);

    const uint8_t capability = data->protocol_info.bit_rate_capability;

    switch(bit_rate) {
    case Iso14443_3bBitRateBoth106Kbit:
        return capability == ISO14443_3B_BIT_RATE_BOTH_106KBIT;
    case Iso14443_3bBitRatePiccToPcd212Kbit:
        return capability & ISO14443_3B_BIT_RATE_PICC_TO_PCD_212KBIT;
    case Iso14443_3bBitRatePiccToPcd424Kbit:
        return capability & ISO14443_3B_BIT_RATE_PICC_TO_PCD_424KBIT;
    case Iso14443_3bBitRatePiccToPcd848Kbit:
        return capability & ISO14443_3B_BIT_RATE_PICC_TO_PCD_848KBIT;
    case Iso14443_3bBitRatePcdToPicc212Kbit:
        return capability & ISO14443_3B_BIT_RATE_PCD_TO_PICC_212KBIT;
    case Iso14443_3bBitRatePcdToPicc424Kbit:
        return capability & ISO14443_3B_BIT_RATE_PCD_TO_PICC_424KBIT;
    case Iso14443_3bBitRatePcdToPicc848Kbit:
        return capability & ISO14443_3B_BIT_RATE_PCD_TO_PICC_848KBIT;
    default:
        return false;
    }
}

bool iso14443_3b_supports_frame_option(const Iso14443_3bData* data, Iso14443_3bFrameOption option) {
    furi_check(data);

    switch(option) {
    case Iso14443_3bFrameOptionNad:
        return data->protocol_info.fo & ISO14443_3B_FRAME_OPTION_NAD;
    case Iso14443_3bFrameOptionCid:
        return data->protocol_info.fo & ISO14443_3B_FRAME_OPTION_CID;
    default:
        return false;
    }
}

const uint8_t* iso14443_3b_get_application_data(const Iso14443_3bData* data, size_t* data_size) {
    furi_check(data);
    furi_check(data_size);

    *data_size = ISO14443_3B_APP_DATA_SIZE;
    return data->app_data;
}

uint16_t iso14443_3b_get_frame_size_max(const Iso14443_3bData* data) {
    furi_check(data);

    const uint8_t fs_bits = data->protocol_info.max_frame_size;

    if(fs_bits < 5) {
        return fs_bits * 8 + 16;
    } else if(fs_bits == 5) {
        return 64;
    } else if(fs_bits == 6) {
        return 96;
    } else if(fs_bits < 13) {
        return 128U << (fs_bits - 7);
    } else {
        return 0;
    }
}

uint32_t iso14443_3b_get_fwt_fc_max(const Iso14443_3bData* data) {
    furi_check(data);

    const uint8_t fwi = data->protocol_info.fwi;
    return fwi < 0x0F ? 4096UL << fwi : ISO14443_3B_FDT_POLL_DEFAULT_FC;
}
