#include "mf_ultralight.h"

#include <bit_lib/bit_lib.h>
#include <furi.h>

#define MF_ULTRALIGHT_PROTOCOL_NAME "NTAG/Ultralight"

#define MF_ULTRALIGHT_FORMAT_VERSION_KEY  "Data format version"
#define MF_ULTRALIGHT_TYPE_KEY            MF_ULTRALIGHT_PROTOCOL_NAME " type"
#define MF_ULTRALIGHT_SIGNATURE_KEY       "Signature"
#define MF_ULTRALIGHT_MIFARE_VERSION_KEY  "Mifare version"
#define MF_ULTRALIGHT_COUNTER_KEY         "Counter"
#define MF_ULTRALIGHT_TEARING_KEY         "Tearing"
#define MF_ULTRALIGHT_PAGES_TOTAL_KEY     "Pages total"
#define MF_ULTRALIGHT_PAGES_READ_KEY      "Pages read"
#define MF_ULTRALIGHT_PAGE_KEY            "Page"
#define MF_ULTRALIGHT_FAILED_ATTEMPTS_KEY "Failed authentication attempts"

typedef struct {
    const char* device_name;
    uint16_t total_pages;
    uint16_t config_page;
    uint32_t feature_set;
} MfUltralightFeatures;

static const uint32_t mf_ultralight_data_format_version = 2;

static const MfUltralightFeatures mf_ultralight_features[MfUltralightTypeNum] = {
    [MfUltralightTypeOrigin] =
        {
            .device_name = "Mifare Ultralight",
            .total_pages = 16,
            .config_page = 0,
            .feature_set = MfUltralightFeatureSupportCompatibleWrite,
        },
    [MfUltralightTypeMfulC] =
        {
            .device_name = "Mifare Ultralight C",
            .total_pages = 48,
            .config_page = 0,
            .feature_set = MfUltralightFeatureSupportCompatibleWrite |
                           MfUltralightFeatureSupportAuthenticate,
        },
    [MfUltralightTypeNTAG203] =
        {
            .device_name = "NTAG203",
            .total_pages = 42,
            .config_page = 0,
            .feature_set = MfUltralightFeatureSupportCompatibleWrite |
                           MfUltralightFeatureSupportCounterInMemory,
        },
    [MfUltralightTypeUL11] =
        {
            .device_name = "Mifare Ultralight 11",
            .total_pages = 20,
            .config_page = 16,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter |
                MfUltralightFeatureSupportCheckTearingFlag | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportIncCounter | MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportPasswordAuth | MfUltralightFeatureSupportVcsl,
        },
    [MfUltralightTypeUL21] =
        {
            .device_name = "Mifare Ultralight 21",
            .total_pages = 41,
            .config_page = 37,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter |
                MfUltralightFeatureSupportCheckTearingFlag | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportIncCounter | MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportPasswordAuth | MfUltralightFeatureSupportVcsl |
                MfUltralightFeatureSupportDynamicLock,
        },
    [MfUltralightTypeNTAG213] =
        {
            .device_name = "NTAG213",
            .total_pages = 45,
            .config_page = 41,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportPasswordAuth | MfUltralightFeatureSupportSingleCounter |
                MfUltralightFeatureSupportAsciiMirror | MfUltralightFeatureSupportDynamicLock,
        },
    [MfUltralightTypeNTAG215] =
        {
            .device_name = "NTAG215",
            .total_pages = 135,
            .config_page = 131,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportPasswordAuth | MfUltralightFeatureSupportSingleCounter |
                MfUltralightFeatureSupportAsciiMirror | MfUltralightFeatureSupportDynamicLock,
        },
    [MfUltralightTypeNTAG216] =
        {
            .device_name = "NTAG216",
            .total_pages = 231,
            .config_page = 227,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportPasswordAuth | MfUltralightFeatureSupportSingleCounter |
                MfUltralightFeatureSupportAsciiMirror | MfUltralightFeatureSupportDynamicLock,
        },
    [MfUltralightTypeNTAGI2C1K] =
        {
            .device_name = "NTAG I2C 1K",
            .total_pages = 231,
            .config_page = 0,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportSectorSelect | MfUltralightFeatureSupportDynamicLock,
        },
    [MfUltralightTypeNTAGI2C2K] =
        {
            .device_name = "NTAG I2C 2K",
            .total_pages = 485,
            .config_page = 0,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportSectorSelect | MfUltralightFeatureSupportDynamicLock,
        },
    [MfUltralightTypeNTAGI2CPlus1K] =
        {
            .device_name = "NTAG I2C Plus 1K",
            .total_pages = 236,
            .config_page = 227,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportFastRead | MfUltralightFeatureSupportPasswordAuth |
                MfUltralightFeatureSupportSectorSelect | MfUltralightFeatureSupportFastWrite |
                MfUltralightFeatureSupportDynamicLock,
        },
    [MfUltralightTypeNTAGI2CPlus2K] =
        {
            .device_name = "NTAG I2C Plus 2K",
            .total_pages = 492,
            .config_page = 227,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportFastRead | MfUltralightFeatureSupportPasswordAuth |
                MfUltralightFeatureSupportSectorSelect | MfUltralightFeatureSupportFastWrite |
                MfUltralightFeatureSupportDynamicLock,
        },
};

const NfcDeviceBase nfc_device_mf_ultralight = {
    .protocol_name = MF_ULTRALIGHT_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)mf_ultralight_alloc,
    .free = (NfcDeviceFree)mf_ultralight_free,
    .reset = (NfcDeviceReset)mf_ultralight_reset,
    .copy = (NfcDeviceCopy)mf_ultralight_copy,
    .verify = (NfcDeviceVerify)mf_ultralight_verify,
    .load = (NfcDeviceLoad)mf_ultralight_load,
    .save = (NfcDeviceSave)mf_ultralight_save,
    .is_equal = (NfcDeviceEqual)mf_ultralight_is_equal,
    .get_name = (NfcDeviceGetName)mf_ultralight_get_device_name,
    .get_uid = (NfcDeviceGetUid)mf_ultralight_get_uid,
    .set_uid = (NfcDeviceSetUid)mf_ultralight_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)mf_ultralight_get_base_data,
};

MfUltralightData* mf_ultralight_alloc(void) {
    MfUltralightData* data = malloc(sizeof(MfUltralightData));
    data->iso14443_3a_data = iso14443_3a_alloc();
    return data;
}

void mf_ultralight_free(MfUltralightData* data) {
    furi_check(data);

    iso14443_3a_free(data->iso14443_3a_data);
    free(data);
}

void mf_ultralight_reset(MfUltralightData* data) {
    furi_check(data);

    iso14443_3a_reset(data->iso14443_3a_data);
}

void mf_ultralight_copy(MfUltralightData* data, const MfUltralightData* other) {
    furi_check(data);
    furi_check(other);

    iso14443_3a_copy(data->iso14443_3a_data, other->iso14443_3a_data);
    for(size_t i = 0; i < COUNT_OF(data->counter); i++) {
        data->counter[i] = other->counter[i];
    }
    for(size_t i = 0; i < COUNT_OF(data->tearing_flag); i++) {
        data->tearing_flag[i] = other->tearing_flag[i];
    }
    for(size_t i = 0; i < COUNT_OF(data->page); i++) {
        data->page[i] = other->page[i];
    }

    data->type = other->type;
    data->version = other->version;
    data->signature = other->signature;

    data->pages_read = other->pages_read;
    data->pages_total = other->pages_total;
    data->auth_attempts = other->auth_attempts;
}

static const char*
    mf_ultralight_get_device_name_by_type(MfUltralightType type, NfcDeviceNameType name_type) {
    if(name_type == NfcDeviceNameTypeShort &&
       (type == MfUltralightTypeUL11 || type == MfUltralightTypeUL21)) {
        type = MfUltralightTypeOrigin;
    }

    return mf_ultralight_features[type].device_name;
}

bool mf_ultralight_verify(MfUltralightData* data, const FuriString* device_type) {
    furi_check(data);
    furi_check(device_type);

    bool verified = false;

    for(MfUltralightType i = 0; i < MfUltralightTypeNum; i++) {
        const char* name = mf_ultralight_get_device_name_by_type(i, NfcDeviceNameTypeFull);
        verified = furi_string_equal(device_type, name);
        if(verified) {
            data->type = i;
            break;
        }
    }

    return verified;
}

bool mf_ultralight_load(MfUltralightData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    FuriString* temp_str = furi_string_alloc();
    bool parsed = false;

    do {
        // Read ISO14443_3A data
        if(!iso14443_3a_load(data->iso14443_3a_data, ff, version)) break;

        // Read Ultralight specific data
        // Read Mifare Ultralight format version
        uint32_t data_format_version = 0;
        if(!flipper_format_read_uint32(
               ff, MF_ULTRALIGHT_FORMAT_VERSION_KEY, &data_format_version, 1)) {
            if(!flipper_format_rewind(ff)) break;
        }

        // Read Mifare Ultralight type
        if(data_format_version > 1) {
            if(!flipper_format_read_string(ff, MF_ULTRALIGHT_TYPE_KEY, temp_str)) break;
            if(!mf_ultralight_verify(data, temp_str)) break;
        }

        // Read signature
        if(!flipper_format_read_hex(
               ff,
               MF_ULTRALIGHT_SIGNATURE_KEY,
               data->signature.data,
               sizeof(MfUltralightSignature)))
            break;
        // Read Mifare version
        if(!flipper_format_read_hex(
               ff,
               MF_ULTRALIGHT_MIFARE_VERSION_KEY,
               (uint8_t*)&data->version,
               sizeof(MfUltralightVersion)))
            break;
        // Read counters and tearing flags
        bool counters_parsed = true;
        for(size_t i = 0; i < 3; i++) {
            furi_string_printf(temp_str, "%s %d", MF_ULTRALIGHT_COUNTER_KEY, i);
            if(!flipper_format_read_uint32(
                   ff, furi_string_get_cstr(temp_str), &data->counter[i].counter, 1)) {
                counters_parsed = false;
                break;
            }
            furi_string_printf(temp_str, "%s %d", MF_ULTRALIGHT_TEARING_KEY, i);
            if(!flipper_format_read_hex(
                   ff, furi_string_get_cstr(temp_str), &data->tearing_flag[i].data, 1)) {
                counters_parsed = false;
                break;
            }
        }
        if(!counters_parsed) break;
        // Read pages
        uint32_t pages_total = 0;
        if(!flipper_format_read_uint32(ff, MF_ULTRALIGHT_PAGES_TOTAL_KEY, &pages_total, 1)) break;
        uint32_t pages_read = 0;
        if(data_format_version < mf_ultralight_data_format_version) { //-V547
            pages_read = pages_total;
        } else {
            if(!flipper_format_read_uint32(ff, MF_ULTRALIGHT_PAGES_READ_KEY, &pages_read, 1))
                break;
        }
        data->pages_total = pages_total;
        data->pages_read = pages_read;

        if((pages_read > MF_ULTRALIGHT_MAX_PAGE_NUM) || (pages_total > MF_ULTRALIGHT_MAX_PAGE_NUM))
            break;

        bool pages_parsed = true;
        for(size_t i = 0; i < pages_total; i++) {
            furi_string_printf(temp_str, "%s %d", MF_ULTRALIGHT_PAGE_KEY, i);
            if(!flipper_format_read_hex(
                   ff,
                   furi_string_get_cstr(temp_str),
                   data->page[i].data,
                   sizeof(MfUltralightPage))) {
                pages_parsed = false;
                break;
            }
        }
        if(!pages_parsed) break;

        // Read authentication counter
        if(!flipper_format_read_uint32(
               ff, MF_ULTRALIGHT_FAILED_ATTEMPTS_KEY, &data->auth_attempts, 1)) {
            data->auth_attempts = 0;
        }

        parsed = true;
    } while(false);

    furi_string_free(temp_str);

    return parsed;
}

bool mf_ultralight_save(const MfUltralightData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    FuriString* temp_str = furi_string_alloc();
    bool saved = false;

    do {
        if(!iso14443_3a_save(data->iso14443_3a_data, ff)) break;

        if(!flipper_format_write_comment_cstr(ff, MF_ULTRALIGHT_PROTOCOL_NAME " specific data"))
            break;
        if(!flipper_format_write_uint32(
               ff, MF_ULTRALIGHT_FORMAT_VERSION_KEY, &mf_ultralight_data_format_version, 1))
            break;

        const char* device_type_name =
            mf_ultralight_get_device_name_by_type(data->type, NfcDeviceNameTypeFull);
        if(!flipper_format_write_string_cstr(ff, MF_ULTRALIGHT_TYPE_KEY, device_type_name)) break;
        if(!flipper_format_write_hex(
               ff,
               MF_ULTRALIGHT_SIGNATURE_KEY,
               data->signature.data,
               sizeof(MfUltralightSignature)))
            break;
        if(!flipper_format_write_hex(
               ff,
               MF_ULTRALIGHT_MIFARE_VERSION_KEY,
               (uint8_t*)&data->version,
               sizeof(MfUltralightVersion)))
            break;

        // Write conters and tearing flags data
        bool counters_saved = true;
        for(size_t i = 0; i < 3; i++) {
            furi_string_printf(temp_str, "%s %d", MF_ULTRALIGHT_COUNTER_KEY, i);
            if(!flipper_format_write_uint32(
                   ff, furi_string_get_cstr(temp_str), &data->counter[i].counter, 1)) {
                counters_saved = false;
                break;
            }
            furi_string_printf(temp_str, "%s %d", MF_ULTRALIGHT_TEARING_KEY, i);
            if(!flipper_format_write_hex(
                   ff, furi_string_get_cstr(temp_str), &data->tearing_flag[i].data, 1)) {
                counters_saved = false;
                break;
            }
        }
        if(!counters_saved) break;

        // Write pages data
        uint32_t pages_total = data->pages_total;
        uint32_t pages_read = data->pages_read;
        if(!flipper_format_write_uint32(ff, MF_ULTRALIGHT_PAGES_TOTAL_KEY, &pages_total, 1)) break;
        if(!flipper_format_write_uint32(ff, MF_ULTRALIGHT_PAGES_READ_KEY, &pages_read, 1)) break;
        bool pages_saved = true;
        for(size_t i = 0; i < data->pages_total; i++) {
            furi_string_printf(temp_str, "%s %d", MF_ULTRALIGHT_PAGE_KEY, i);
            if(!flipper_format_write_hex(
                   ff,
                   furi_string_get_cstr(temp_str),
                   data->page[i].data,
                   sizeof(MfUltralightPage))) {
                pages_saved = false;
                break;
            }
        }
        if(!pages_saved) break;

        // Write authentication counter
        if(!flipper_format_write_uint32(
               ff, MF_ULTRALIGHT_FAILED_ATTEMPTS_KEY, &data->auth_attempts, 1))
            break;

        saved = true;
    } while(false);

    furi_string_free(temp_str);

    return saved;
}

bool mf_ultralight_is_equal(const MfUltralightData* data, const MfUltralightData* other) {
    furi_check(data);
    furi_check(other);

    bool is_equal = false;
    bool data_array_is_equal = true;

    do {
        if(!iso14443_3a_is_equal(data->iso14443_3a_data, other->iso14443_3a_data)) break;
        if(data->type != other->type) break;
        if(data->pages_read != other->pages_read) break;
        if(data->pages_total != other->pages_total) break;
        if(data->auth_attempts != other->auth_attempts) break;

        if(memcmp(&data->version, &other->version, sizeof(data->version)) != 0) break;
        if(memcmp(&data->signature, &other->signature, sizeof(data->signature)) != 0) break;

        for(size_t i = 0; i < COUNT_OF(data->counter); i++) {
            if(memcmp(&data->counter[i], &other->counter[i], sizeof(data->counter[i])) != 0) {
                data_array_is_equal = false;
                break;
            }
        }
        if(!data_array_is_equal) break;

        for(size_t i = 0; i < COUNT_OF(data->tearing_flag); i++) {
            if(memcmp(
                   &data->tearing_flag[i],
                   &other->tearing_flag[i],
                   sizeof(data->tearing_flag[i])) != 0) {
                data_array_is_equal = false;
                break;
            }
        }
        if(!data_array_is_equal) break;

        for(size_t i = 0; i < COUNT_OF(data->page); i++) {
            if(memcmp(&data->page[i], &other->page[i], sizeof(data->page[i])) != 0) {
                data_array_is_equal = false;
                break;
            }
        }
        if(!data_array_is_equal) break;

        is_equal = true;
    } while(false);

    return is_equal;
}

const char*
    mf_ultralight_get_device_name(const MfUltralightData* data, NfcDeviceNameType name_type) {
    furi_check(data);
    furi_check(data->type < MfUltralightTypeNum);

    return mf_ultralight_get_device_name_by_type(data->type, name_type);
}

const uint8_t* mf_ultralight_get_uid(const MfUltralightData* data, size_t* uid_len) {
    furi_check(data);

    return iso14443_3a_get_uid(data->iso14443_3a_data, uid_len);
}

bool mf_ultralight_set_uid(MfUltralightData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);

    bool uid_valid = iso14443_3a_set_uid(data->iso14443_3a_data, uid, uid_len);

    if(uid_valid) {
        // Copy UID across first 2 pages
        memcpy(data->page[0].data, data->iso14443_3a_data->uid, 3);
        memcpy(data->page[1].data, &data->iso14443_3a_data->uid[3], 4);

        // Calculate BCC bytes
        data->page[0].data[3] = 0x88 ^ uid[0] ^ uid[1] ^ uid[2];
        data->page[2].data[0] = uid[3] ^ uid[4] ^ uid[5] ^ uid[6];
    }

    return uid_valid;
}

Iso14443_3aData* mf_ultralight_get_base_data(const MfUltralightData* data) {
    furi_check(data);

    return data->iso14443_3a_data;
}

MfUltralightType mf_ultralight_get_type_by_version(MfUltralightVersion* version) {
    furi_check(version);

    MfUltralightType type = MfUltralightTypeOrigin;

    if(version->storage_size == 0x0B || version->storage_size == 0x00) {
        type = MfUltralightTypeUL11;
    } else if(version->storage_size == 0x0E) {
        type = MfUltralightTypeUL21;
    } else if(version->storage_size == 0x0F) {
        type = MfUltralightTypeNTAG213;
    } else if(version->storage_size == 0x11) {
        type = MfUltralightTypeNTAG215;
    } else if(version->prod_subtype == 5 && version->prod_ver_major == 2) {
        if(version->prod_ver_minor == 1) {
            if(version->storage_size == 0x13) {
                type = MfUltralightTypeNTAGI2C1K;
            } else if(version->storage_size == 0x15) {
                type = MfUltralightTypeNTAGI2C2K;
            }
        } else if(version->prod_ver_minor == 2) {
            if(version->storage_size == 0x13) {
                type = MfUltralightTypeNTAGI2CPlus1K;
            } else if(version->storage_size == 0x15) {
                type = MfUltralightTypeNTAGI2CPlus2K;
            }
        }
    } else if(version->storage_size == 0x13) {
        type = MfUltralightTypeNTAG216;
    }

    return type;
}

uint16_t mf_ultralight_get_pages_total(MfUltralightType type) {
    furi_check(type < MfUltralightTypeNum);

    return mf_ultralight_features[type].total_pages;
}

uint32_t mf_ultralight_get_feature_support_set(MfUltralightType type) {
    furi_check(type < MfUltralightTypeNum);

    return mf_ultralight_features[type].feature_set;
}

bool mf_ultralight_detect_protocol(const Iso14443_3aData* iso14443_3a_data) {
    furi_check(iso14443_3a_data);

    bool mfu_detected = (iso14443_3a_data->atqa[0] == 0x44) &&
                        (iso14443_3a_data->atqa[1] == 0x00) && (iso14443_3a_data->sak == 0x00);

    return mfu_detected;
}

uint16_t mf_ultralight_get_config_page_num(MfUltralightType type) {
    furi_check(type < MfUltralightTypeNum);

    return mf_ultralight_features[type].config_page;
}

uint8_t mf_ultralight_get_write_end_page(MfUltralightType type) {
    furi_check(type < MfUltralightTypeNum);
    furi_assert(
        type == MfUltralightTypeUL11 || type == MfUltralightTypeUL21 ||
        type == MfUltralightTypeNTAG213 || type == MfUltralightTypeNTAG215 ||
        type == MfUltralightTypeNTAG216 || type == MfUltralightTypeOrigin);

    uint8_t end_page = mf_ultralight_get_config_page_num(type);
    if(type == MfUltralightTypeNTAG213 || type == MfUltralightTypeNTAG215 ||
       type == MfUltralightTypeNTAG216) {
        end_page -= 1;
    } else if(type == MfUltralightTypeOrigin) {
        end_page = mf_ultralight_features[type].total_pages;
    }

    return end_page;
}

uint8_t mf_ultralight_get_pwd_page_num(MfUltralightType type) {
    furi_check(type < MfUltralightTypeNum);

    uint8_t config_page = mf_ultralight_features[type].config_page;
    return (config_page != 0) ? config_page + 2 : 0;
}

bool mf_ultralight_is_page_pwd_or_pack(MfUltralightType type, uint16_t page) {
    uint8_t pwd_page = mf_ultralight_get_pwd_page_num(type);
    uint8_t pack_page = pwd_page + 1;
    return (pwd_page != 0) && (page == pwd_page || page == pack_page);
}

bool mf_ultralight_support_feature(const uint32_t feature_set, const uint32_t features_to_check) {
    return (feature_set & features_to_check) != 0;
}

bool mf_ultralight_get_config_page(const MfUltralightData* data, MfUltralightConfigPages** config) {
    furi_check(data);
    furi_check(config);

    bool config_pages_found = false;

    uint16_t config_page = mf_ultralight_features[data->type].config_page;
    if(config_page != 0) {
        *config = (MfUltralightConfigPages*)&data->page[config_page]; //-V1027
        config_pages_found = true;
    }

    return config_pages_found;
}

bool mf_ultralight_is_all_data_read(const MfUltralightData* data) {
    furi_check(data);

    bool all_read = false;

    if(data->pages_read == data->pages_total) {
        uint32_t feature_set = mf_ultralight_get_feature_support_set(data->type);
        if((data->type == MfUltralightTypeMfulC) &&
           mf_ultralight_support_feature(feature_set, MfUltralightFeatureSupportAuthenticate)) {
            all_read = true;
        } else if(!mf_ultralight_support_feature(
                      feature_set, MfUltralightFeatureSupportPasswordAuth)) {
            all_read = true;
        } else {
            // Having read all the pages doesn't mean that we've got everything.
            // By default PWD is 0xFFFFFFFF, but if read back it is always 0x00000000,
            // so a default read on an auth-supported NTAG is never complete.
            MfUltralightConfigPages* config = NULL;
            if(mf_ultralight_get_config_page(data, &config)) {
                uint32_t pass = bit_lib_bytes_to_num_be(
                    config->password.data, sizeof(MfUltralightAuthPassword));
                uint16_t pack =
                    bit_lib_bytes_to_num_be(config->pack.data, sizeof(MfUltralightAuthPack));
                all_read = ((pass != 0) || (pack != 0));
            }
        }
    }

    return all_read;
}

bool mf_ultralight_is_counter_configured(const MfUltralightData* data) {
    furi_check(data);

    MfUltralightConfigPages* config = NULL;
    bool configured = false;

    switch(data->type) {
    case MfUltralightTypeNTAG213:
    case MfUltralightTypeNTAG215:
    case MfUltralightTypeNTAG216:
        if(mf_ultralight_get_config_page(data, &config)) {
            configured = config->access.nfc_cnt_en;
        }
        break;

    default:
        configured = true;
        break;
    }

    return configured;
}

void mf_ultralight_3des_shift_data(uint8_t* const data) {
    furi_check(data);

    uint8_t buf = data[0];
    for(uint8_t i = 1; i < MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE; i++) {
        data[i - 1] = data[i];
    }
    data[MF_ULTRALIGHT_C_AUTH_RND_BLOCK_SIZE - 1] = buf;
}

bool mf_ultralight_3des_key_valid(const MfUltralightData* data) {
    furi_check(data);
    furi_check(data->type == MfUltralightTypeMfulC);

    return !(data->pages_read == data->pages_total - 4);
}

const uint8_t* mf_ultralight_3des_get_key(const MfUltralightData* data) {
    furi_check(data);
    furi_check(data->type == MfUltralightTypeMfulC);

    return data->page[44].data;
}

void mf_ultralight_3des_encrypt(
    mbedtls_des3_context* ctx,
    const uint8_t* ck,
    const uint8_t* iv,
    const uint8_t* input,
    const uint8_t length,
    uint8_t* out) {
    furi_check(ctx);
    furi_check(ck);
    furi_check(iv);
    furi_check(input);
    furi_check(out);

    mbedtls_des3_set2key_enc(ctx, ck);
    mbedtls_des3_crypt_cbc(ctx, MBEDTLS_DES_ENCRYPT, length, (uint8_t*)iv, input, out);
}

void mf_ultralight_3des_decrypt(
    mbedtls_des3_context* ctx,
    const uint8_t* ck,
    const uint8_t* iv,
    const uint8_t* input,
    const uint8_t length,
    uint8_t* out) {
    furi_check(ctx);
    furi_check(ck);
    furi_check(iv);
    furi_check(input);
    furi_check(out);

    mbedtls_des3_set2key_dec(ctx, ck);
    mbedtls_des3_crypt_cbc(ctx, MBEDTLS_DES_DECRYPT, length, (uint8_t*)iv, input, out);
}
