#include "iso15693_3.h"
#include "iso15693_3_device_defs.h"

#include <nfc/nfc_common.h>

#define ISO15693_3_PROTOCOL_NAME        "ISO15693-3"
#define ISO15693_3_PROTOCOL_NAME_LEGACY "ISO15693"
#define ISO15693_3_DEVICE_NAME          "ISO15693-3 (Unknown)"

#define ISO15693_3_LOCK_DSFID_LEGACY (1U << 0)
#define ISO15693_3_LOCK_AFI_LEGACY   (1U << 1)

#define ISO15693_3_DSFID_KEY           "DSFID"
#define ISO15693_3_AFI_KEY             "AFI"
#define ISO15693_3_IC_REF_KEY          "IC Reference"
#define ISO15693_3_BLOCK_COUNT_KEY     "Block Count"
#define ISO15693_3_BLOCK_SIZE_KEY      "Block Size"
#define ISO15693_3_DATA_CONTENT_KEY    "Data Content"
#define ISO15693_3_LOCK_DSFID_KEY      "Lock DSFID"
#define ISO15693_3_LOCK_AFI_KEY        "Lock AFI"
#define ISO15693_3_SECURITY_STATUS_KEY "Security Status"

const NfcDeviceBase nfc_device_iso15693_3 = {
    .protocol_name = ISO15693_3_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)iso15693_3_alloc,
    .free = (NfcDeviceFree)iso15693_3_free,
    .reset = (NfcDeviceReset)iso15693_3_reset,
    .copy = (NfcDeviceCopy)iso15693_3_copy,
    .verify = (NfcDeviceVerify)iso15693_3_verify,
    .load = (NfcDeviceLoad)iso15693_3_load,
    .save = (NfcDeviceSave)iso15693_3_save,
    .is_equal = (NfcDeviceEqual)iso15693_3_is_equal,
    .get_name = (NfcDeviceGetName)iso15693_3_get_device_name,
    .get_uid = (NfcDeviceGetUid)iso15693_3_get_uid,
    .set_uid = (NfcDeviceSetUid)iso15693_3_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)iso15693_3_get_base_data,
};

Iso15693_3Data* iso15693_3_alloc(void) {
    Iso15693_3Data* data = malloc(sizeof(Iso15693_3Data));

    data->block_data = simple_array_alloc(&simple_array_config_uint8_t);
    data->block_security = simple_array_alloc(&simple_array_config_uint8_t);

    return data;
}

void iso15693_3_free(Iso15693_3Data* data) {
    furi_check(data);

    simple_array_free(data->block_data);
    simple_array_free(data->block_security);
    free(data);
}

void iso15693_3_reset(Iso15693_3Data* data) {
    furi_check(data);

    memset(data->uid, 0, ISO15693_3_UID_SIZE);
    memset(&data->system_info, 0, sizeof(Iso15693_3SystemInfo));
    memset(&data->settings, 0, sizeof(Iso15693_3Settings));

    simple_array_reset(data->block_data);
    simple_array_reset(data->block_security);
}

void iso15693_3_copy(Iso15693_3Data* data, const Iso15693_3Data* other) {
    furi_check(data);
    furi_check(other);

    memcpy(data->uid, other->uid, ISO15693_3_UID_SIZE);

    data->system_info = other->system_info;
    data->settings = other->settings;

    simple_array_copy(data->block_data, other->block_data);
    simple_array_copy(data->block_security, other->block_security);
}

bool iso15693_3_verify(Iso15693_3Data* data, const FuriString* device_type) {
    UNUSED(data);
    furi_check(device_type);

    return furi_string_equal(device_type, ISO15693_3_PROTOCOL_NAME_LEGACY);
}

static inline bool iso15693_3_load_security_legacy(Iso15693_3Data* data, FlipperFormat* ff) {
    bool loaded = false;
    uint8_t* legacy_data = NULL;

    do {
        uint32_t value_count;
        if(!flipper_format_get_value_count(ff, ISO15693_3_SECURITY_STATUS_KEY, &value_count))
            break;
        if(simple_array_get_count(data->block_security) + 1 != value_count) break;

        legacy_data = malloc(value_count);
        if(!flipper_format_read_hex(ff, ISO15693_3_SECURITY_STATUS_KEY, legacy_data, value_count))
            break;

        // First legacy data byte is lock bits
        data->settings.lock_bits.dsfid = legacy_data[0] & ISO15693_3_LOCK_DSFID_LEGACY;
        data->settings.lock_bits.afi = legacy_data[0] & ISO15693_3_LOCK_AFI_LEGACY;

        // The rest are block security
        memcpy(
            &legacy_data[1],
            simple_array_get_data(data->block_security),
            simple_array_get_count(data->block_security));

        loaded = true;
    } while(false);

    if(legacy_data) free(legacy_data);

    return loaded;
}

static inline bool iso15693_3_load_security(Iso15693_3Data* data, FlipperFormat* ff) {
    bool loaded = false;

    do {
        uint32_t value_count;
        if(!flipper_format_get_value_count(ff, ISO15693_3_SECURITY_STATUS_KEY, &value_count))
            break;
        if(simple_array_get_count(data->block_security) != value_count) break;
        if(!flipper_format_read_hex(
               ff,
               ISO15693_3_SECURITY_STATUS_KEY,
               simple_array_get_data(data->block_security),
               simple_array_get_count(data->block_security)))
            break;

        loaded = true;
    } while(false);

    return loaded;
}

bool iso15693_3_load(Iso15693_3Data* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);
    UNUSED(version);

    bool loaded = false;

    do {
        if(flipper_format_key_exist(ff, ISO15693_3_DSFID_KEY)) {
            if(!flipper_format_read_hex(ff, ISO15693_3_DSFID_KEY, &data->system_info.dsfid, 1))
                break;
            data->system_info.flags |= ISO15693_3_SYSINFO_FLAG_DSFID;
        }

        if(flipper_format_key_exist(ff, ISO15693_3_AFI_KEY)) {
            if(!flipper_format_read_hex(ff, ISO15693_3_AFI_KEY, &data->system_info.afi, 1)) break;
            data->system_info.flags |= ISO15693_3_SYSINFO_FLAG_AFI;
        }

        if(flipper_format_key_exist(ff, ISO15693_3_IC_REF_KEY)) {
            if(!flipper_format_read_hex(ff, ISO15693_3_IC_REF_KEY, &data->system_info.ic_ref, 1))
                break;
            data->system_info.flags |= ISO15693_3_SYSINFO_FLAG_IC_REF;
        }

        const bool has_lock_bits = flipper_format_key_exist(ff, ISO15693_3_LOCK_DSFID_KEY) &&
                                   flipper_format_key_exist(ff, ISO15693_3_LOCK_AFI_KEY);
        if(has_lock_bits) {
            Iso15693_3LockBits* lock_bits = &data->settings.lock_bits;
            if(!flipper_format_read_bool(ff, ISO15693_3_LOCK_DSFID_KEY, &lock_bits->dsfid, 1))
                break;
            if(!flipper_format_read_bool(ff, ISO15693_3_LOCK_AFI_KEY, &lock_bits->afi, 1)) break;
        }

        if(flipper_format_key_exist(ff, ISO15693_3_BLOCK_COUNT_KEY) &&
           flipper_format_key_exist(ff, ISO15693_3_BLOCK_SIZE_KEY)) {
            uint32_t block_count;
            if(!flipper_format_read_uint32(ff, ISO15693_3_BLOCK_COUNT_KEY, &block_count, 1)) break;

            data->system_info.block_count = block_count;
            data->system_info.flags |= ISO15693_3_SYSINFO_FLAG_MEMORY;

            if(!flipper_format_read_hex(
                   ff, ISO15693_3_BLOCK_SIZE_KEY, &(data->system_info.block_size), 1))
                break;

            simple_array_init(
                data->block_data, data->system_info.block_size * data->system_info.block_count);

            if(!flipper_format_read_hex(
                   ff,
                   ISO15693_3_DATA_CONTENT_KEY,
                   simple_array_get_data(data->block_data),
                   simple_array_get_count(data->block_data)))
                break;

            if(flipper_format_key_exist(ff, ISO15693_3_SECURITY_STATUS_KEY)) {
                simple_array_init(data->block_security, data->system_info.block_count);

                const bool security_loaded = has_lock_bits ?
                                                 iso15693_3_load_security(data, ff) :
                                                 iso15693_3_load_security_legacy(data, ff);
                if(!security_loaded) break;
            }
        }

        loaded = true;
    } while(false);

    return loaded;
}

bool iso15693_3_save(const Iso15693_3Data* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    bool saved = false;

    do {
        if(!flipper_format_write_comment_cstr(ff, ISO15693_3_PROTOCOL_NAME " specific data"))
            break;

        if(data->system_info.flags & ISO15693_3_SYSINFO_FLAG_DSFID) {
            if(!flipper_format_write_comment_cstr(ff, "Data Storage Format Identifier")) break;
            if(!flipper_format_write_hex(ff, ISO15693_3_DSFID_KEY, &data->system_info.dsfid, 1))
                break;
        }

        if(data->system_info.flags & ISO15693_3_SYSINFO_FLAG_AFI) {
            if(!flipper_format_write_comment_cstr(ff, "Application Family Identifier")) break;
            if(!flipper_format_write_hex(ff, ISO15693_3_AFI_KEY, &data->system_info.afi, 1)) break;
        }

        if(data->system_info.flags & ISO15693_3_SYSINFO_FLAG_IC_REF) {
            if(!flipper_format_write_comment_cstr(ff, "IC Reference - Vendor specific meaning"))
                break;
            if(!flipper_format_write_hex(ff, ISO15693_3_IC_REF_KEY, &data->system_info.ic_ref, 1))
                break;
        }

        if(!flipper_format_write_comment_cstr(ff, "Lock Bits")) break;
        if(!flipper_format_write_bool(
               ff, ISO15693_3_LOCK_DSFID_KEY, &data->settings.lock_bits.dsfid, 1))
            break;
        if(!flipper_format_write_bool(
               ff, ISO15693_3_LOCK_AFI_KEY, &data->settings.lock_bits.afi, 1))
            break;

        if(data->system_info.flags & ISO15693_3_SYSINFO_FLAG_MEMORY) {
            const uint32_t block_count = data->system_info.block_count;
            if(!flipper_format_write_comment_cstr(
                   ff, "Number of memory blocks, valid range = 1..256"))
                break;
            if(!flipper_format_write_uint32(ff, ISO15693_3_BLOCK_COUNT_KEY, &block_count, 1))
                break;

            if(!flipper_format_write_comment_cstr(
                   ff, "Size of a single memory block, valid range = 01...20 (hex)"))
                break;
            if(!flipper_format_write_hex(
                   ff, ISO15693_3_BLOCK_SIZE_KEY, &data->system_info.block_size, 1))
                break;

            if(!flipper_format_write_hex(
                   ff,
                   ISO15693_3_DATA_CONTENT_KEY,
                   simple_array_cget_data(data->block_data),
                   simple_array_get_count(data->block_data)))
                break;

            if(!flipper_format_write_comment_cstr(
                   ff, "Block Security Status: 01 = locked, 00 = not locked"))
                break;
            if(!flipper_format_write_hex(
                   ff,
                   ISO15693_3_SECURITY_STATUS_KEY,
                   simple_array_cget_data(data->block_security),
                   simple_array_get_count(data->block_security)))
                break;
        }
        saved = true;
    } while(false);

    return saved;
}

bool iso15693_3_is_equal(const Iso15693_3Data* data, const Iso15693_3Data* other) {
    furi_check(data);
    furi_check(other);

    return memcmp(data->uid, other->uid, ISO15693_3_UID_SIZE) == 0 &&
           memcmp(&data->settings, &other->settings, sizeof(Iso15693_3Settings)) == 0 &&
           memcmp( //-V1103
               &data->system_info,
               &other->system_info,
               sizeof(Iso15693_3SystemInfo)) == 0 &&
           simple_array_is_equal(data->block_data, other->block_data) &&
           simple_array_is_equal(data->block_security, other->block_security);
}

const char* iso15693_3_get_device_name(const Iso15693_3Data* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);

    return ISO15693_3_DEVICE_NAME;
}

const uint8_t* iso15693_3_get_uid(const Iso15693_3Data* data, size_t* uid_len) {
    furi_check(data);

    if(uid_len) *uid_len = ISO15693_3_UID_SIZE;
    return data->uid;
}

bool iso15693_3_set_uid(Iso15693_3Data* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);
    furi_check(uid);

    bool uid_valid = uid_len == ISO15693_3_UID_SIZE;

    if(uid_valid) {
        memcpy(data->uid, uid, uid_len);
        // All ISO15693-3 cards must have this as first UID byte
        data->uid[0] = 0xe0;
    }

    return uid_valid;
}

Iso15693_3Data* iso15693_3_get_base_data(const Iso15693_3Data* data) {
    UNUSED(data);
    furi_crash("No base data");
}

bool iso15693_3_is_block_locked(const Iso15693_3Data* data, uint8_t block_index) {
    furi_check(data);
    furi_check(block_index < data->system_info.block_count);

    return *(const uint8_t*)simple_array_cget(data->block_security, block_index);
}

uint8_t iso15693_3_get_manufacturer_id(const Iso15693_3Data* data) {
    furi_check(data);

    return data->uid[1];
}

uint16_t iso15693_3_get_block_count(const Iso15693_3Data* data) {
    furi_check(data);

    return data->system_info.block_count;
}

uint8_t iso15693_3_get_block_size(const Iso15693_3Data* data) {
    furi_check(data);

    return data->system_info.block_size;
}

const uint8_t* iso15693_3_get_block_data(const Iso15693_3Data* data, uint8_t block_index) {
    furi_check(data);
    furi_check(data->system_info.block_count > block_index);

    return (const uint8_t*)simple_array_cget(
        data->block_data, block_index * data->system_info.block_size);
}
