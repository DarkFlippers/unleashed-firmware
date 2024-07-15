#include "slix_i.h"
#include "slix_device_defs.h"

#include <furi.h>
#include <nfc/nfc_common.h>

#define SLIX_PROTOCOL_NAME "SLIX"
#define SLIX_DEVICE_NAME   "SLIX"

#define SLIX_TYPE_SLIX_SLIX2 (0x01U)
#define SLIX_TYPE_SLIX_S     (0x02U)
#define SLIX_TYPE_SLIX_L     (0x03U)

#define SLIX_TYPE_INDICATOR_SLIX  (0x02U)
#define SLIX_TYPE_INDICATOR_SLIX2 (0x01U)

#define SLIX_CAPABILITIES_KEY         "Capabilities"
#define SLIX_PASSWORD_READ_KEY        "Password Read"
#define SLIX_PASSWORD_WRITE_KEY       "Password Write"
#define SLIX_PASSWORD_PRIVACY_KEY     "Password Privacy"
#define SLIX_PASSWORD_DESTROY_KEY     "Password Destroy"
#define SLIX_PASSWORD_EAS_KEY         "Password EAS"
#define SLIX_SIGNATURE_KEY            "Signature"
#define SLIX_PRIVACY_MODE_KEY         "Privacy Mode"
#define SLIX_PROTECTION_POINTER_KEY   "Protection Pointer"
#define SLIX_PROTECTION_CONDITION_KEY "Protection Condition"
#define SLIX_LOCK_EAS_KEY             "Lock EAS"
#define SLIX_LOCK_PPL_KEY             "Lock PPL"

typedef struct {
    uint8_t iso15693_3[2];
    uint8_t icode_type;
    union {
        struct {
            uint8_t unused_1       : 3;
            uint8_t type_indicator : 2;
            uint8_t unused_2       : 3;
        };
        uint8_t serial_num[5];
    };
} SlixUidLayout;

const NfcDeviceBase nfc_device_slix = {
    .protocol_name = SLIX_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)slix_alloc,
    .free = (NfcDeviceFree)slix_free,
    .reset = (NfcDeviceReset)slix_reset,
    .copy = (NfcDeviceCopy)slix_copy,
    .verify = (NfcDeviceVerify)slix_verify,
    .load = (NfcDeviceLoad)slix_load,
    .save = (NfcDeviceSave)slix_save,
    .is_equal = (NfcDeviceEqual)slix_is_equal,
    .get_name = (NfcDeviceGetName)slix_get_device_name,
    .get_uid = (NfcDeviceGetUid)slix_get_uid,
    .set_uid = (NfcDeviceSetUid)slix_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)slix_get_base_data,
};

static const char* slix_nfc_device_name[] = {
    [SlixTypeSlix] = SLIX_DEVICE_NAME,
    [SlixTypeSlixS] = SLIX_DEVICE_NAME "-S",
    [SlixTypeSlixL] = SLIX_DEVICE_NAME "-L",
    [SlixTypeSlix2] = SLIX_DEVICE_NAME "2",
};

static const SlixTypeFeatures slix_type_features[] = {
    [SlixTypeSlix] = SLIX_TYPE_FEATURES_SLIX,
    [SlixTypeSlixS] = SLIX_TYPE_FEATURES_SLIX_S,
    [SlixTypeSlixL] = SLIX_TYPE_FEATURES_SLIX_L,
    [SlixTypeSlix2] = SLIX_TYPE_FEATURES_SLIX2,
};

static const char* slix_capabilities_names[SlixCapabilitiesCount] = {
    [SlixCapabilitiesDefault] = "Default",
    [SlixCapabilitiesAcceptAllPasswords] = "AcceptAllPasswords",
};

typedef struct {
    const char* key;
    SlixTypeFeatures feature_flag;
    SlixPassword default_value;
} SlixPasswordConfig;

static const SlixPasswordConfig slix_password_configs[] = {
    [SlixPasswordTypeRead] = {SLIX_PASSWORD_READ_KEY, SLIX_TYPE_FEATURE_READ, 0x00000000U},
    [SlixPasswordTypeWrite] = {SLIX_PASSWORD_WRITE_KEY, SLIX_TYPE_FEATURE_WRITE, 0x00000000U},
    [SlixPasswordTypePrivacy] = {SLIX_PASSWORD_PRIVACY_KEY, SLIX_TYPE_FEATURE_PRIVACY, 0x0F0F0F0FU},
    [SlixPasswordTypeDestroy] = {SLIX_PASSWORD_DESTROY_KEY, SLIX_TYPE_FEATURE_DESTROY, 0x0F0F0F0FU},
    [SlixPasswordTypeEasAfi] = {SLIX_PASSWORD_EAS_KEY, SLIX_TYPE_FEATURE_EAS, 0x00000000U},
};

static void slix_password_set_defaults(SlixPassword* passwords) {
    for(uint32_t i = 0; i < COUNT_OF(slix_password_configs); ++i) {
        passwords[i] = slix_password_configs[i].default_value;
    }
}

SlixData* slix_alloc(void) {
    SlixData* data = malloc(sizeof(SlixData));

    data->iso15693_3_data = iso15693_3_alloc();
    slix_password_set_defaults(data->passwords);

    return data;
}

void slix_free(SlixData* data) {
    furi_check(data);

    iso15693_3_free(data->iso15693_3_data);

    free(data);
}

void slix_reset(SlixData* data) {
    furi_check(data);

    iso15693_3_reset(data->iso15693_3_data);
    data->capabilities = SlixCapabilitiesDefault;
    slix_password_set_defaults(data->passwords);

    memset(&data->system_info, 0, sizeof(SlixSystemInfo));
    memset(data->signature, 0, sizeof(SlixSignature));

    data->privacy = false;
}

void slix_copy(SlixData* data, const SlixData* other) {
    furi_check(data);
    furi_check(other);

    iso15693_3_copy(data->iso15693_3_data, other->iso15693_3_data);
    data->capabilities = other->capabilities;

    memcpy(data->passwords, other->passwords, sizeof(SlixPassword) * SlixPasswordTypeCount);
    memcpy(data->signature, other->signature, sizeof(SlixSignature));

    data->system_info = other->system_info;
    data->privacy = other->privacy;
}

bool slix_verify(SlixData* data, const FuriString* device_type) {
    UNUSED(data);
    UNUSED(device_type);
    // No backward compatibility, unified format only
    return false;
}

static bool slix_load_capabilities(SlixData* data, FlipperFormat* ff) {
    bool capabilities_loaded = false;
    FuriString* capabilities_str = furi_string_alloc();

    if(!flipper_format_read_string(ff, SLIX_CAPABILITIES_KEY, capabilities_str)) {
        if(flipper_format_rewind(ff)) {
            data->capabilities = SlixCapabilitiesDefault;
            capabilities_loaded = true;
        }
    } else {
        for(size_t i = 0; i < COUNT_OF(slix_capabilities_names); i++) {
            if(furi_string_cmp_str(capabilities_str, slix_capabilities_names[i]) == 0) {
                data->capabilities = i;
                capabilities_loaded = true;
                break;
            }
        }
    }

    furi_string_free(capabilities_str);

    return capabilities_loaded;
}

static bool slix_load_passwords(SlixPassword* passwords, SlixType slix_type, FlipperFormat* ff) {
    bool ret = true;

    for(uint32_t i = 0; i < COUNT_OF(slix_password_configs); ++i) {
        const SlixPasswordConfig* password_config = &slix_password_configs[i];

        if(!slix_type_has_features(slix_type, password_config->feature_flag)) continue;
        if(!flipper_format_key_exist(ff, password_config->key)) {
            passwords[i] = password_config->default_value;
            continue;
        }
        if(!flipper_format_read_hex(
               ff, password_config->key, (uint8_t*)&passwords[i], sizeof(SlixPassword))) {
            ret = false;
            break;
        }
    }

    return ret;
}

bool slix_load(SlixData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    bool loaded = false;
    do {
        if(!iso15693_3_load(data->iso15693_3_data, ff, version)) break;

        const SlixType slix_type = slix_get_type(data);
        if(slix_type >= SlixTypeCount) break;

        if(!slix_load_capabilities(data, ff)) break;

        if(!slix_load_passwords(data->passwords, slix_type, ff)) break;

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_SIGNATURE)) {
            if(flipper_format_key_exist(ff, SLIX_SIGNATURE_KEY)) {
                if(!flipper_format_read_hex(
                       ff, SLIX_SIGNATURE_KEY, data->signature, SLIX_SIGNATURE_SIZE))
                    break;
            }
        }

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_PRIVACY)) {
            if(flipper_format_key_exist(ff, SLIX_PRIVACY_MODE_KEY)) {
                if(!flipper_format_read_bool(ff, SLIX_PRIVACY_MODE_KEY, &data->privacy, 1)) break;
            }
        }

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_PROTECTION)) {
            SlixProtection* protection = &data->system_info.protection;
            if(flipper_format_key_exist(ff, SLIX_PROTECTION_POINTER_KEY) &&
               flipper_format_key_exist(ff, SLIX_PROTECTION_CONDITION_KEY)) {
                if(!flipper_format_read_hex(
                       ff, SLIX_PROTECTION_POINTER_KEY, &protection->pointer, 1))
                    break;
                if(!flipper_format_read_hex(
                       ff, SLIX_PROTECTION_CONDITION_KEY, &protection->condition, 1))
                    break;
            }
        }

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_EAS)) {
            if(flipper_format_key_exist(ff, SLIX_LOCK_EAS_KEY)) {
                SlixLockBits* lock_bits = &data->system_info.lock_bits;
                if(!flipper_format_read_bool(ff, SLIX_LOCK_EAS_KEY, &lock_bits->eas, 1)) break;
            }
        }

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_PROTECTION)) {
            if(flipper_format_key_exist(ff, SLIX_LOCK_PPL_KEY)) {
                SlixLockBits* lock_bits = &data->system_info.lock_bits;
                if(!flipper_format_read_bool(ff, SLIX_LOCK_PPL_KEY, &lock_bits->ppl, 1)) break;
            }
        }

        loaded = true;
    } while(false);

    return loaded;
}

static bool slix_save_capabilities(const SlixData* data, FlipperFormat* ff) {
    bool save_success = false;

    FuriString* tmp_str = furi_string_alloc();
    do {
        furi_string_set_str(
            tmp_str, "SLIX capabilities field affects emulation modes. Possible options: ");
        for(size_t i = 0; i < SlixCapabilitiesCount; i++) {
            furi_string_cat_str(tmp_str, slix_capabilities_names[i]);
            if(i < SlixCapabilitiesCount - 1) {
                furi_string_cat(tmp_str, ", ");
            }
        }
        if(!flipper_format_write_comment_cstr(ff, furi_string_get_cstr(tmp_str))) break;

        if(!flipper_format_write_string_cstr(
               ff, SLIX_CAPABILITIES_KEY, slix_capabilities_names[data->capabilities]))
            break;

        save_success = true;
    } while(false);

    furi_string_free(tmp_str);

    return save_success;
}

static bool
    slix_save_passwords(const SlixPassword* passwords, SlixType slix_type, FlipperFormat* ff) {
    bool ret = true;

    for(uint32_t i = 0; i < COUNT_OF(slix_password_configs); ++i) {
        const SlixPasswordConfig* password_config = &slix_password_configs[i];

        if(!slix_type_has_features(slix_type, password_config->feature_flag)) continue;
        if(!flipper_format_write_hex(
               ff, password_config->key, (uint8_t*)&passwords[i], sizeof(SlixPassword))) {
            ret = false;
            break;
        }
    }

    return ret;
}

bool slix_save(const SlixData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    bool saved = false;

    do {
        const SlixType slix_type = slix_get_type(data);
        if(slix_type >= SlixTypeCount) break;

        if(!iso15693_3_save(data->iso15693_3_data, ff)) break;
        if(!flipper_format_write_comment_cstr(ff, SLIX_PROTOCOL_NAME " specific data")) break;

        if(!slix_save_capabilities(data, ff)) break;

        if(!flipper_format_write_comment_cstr(
               ff,
               "Passwords are optional. If a password is omitted, a default value will be used"))
            break;

        if(!slix_save_passwords(data->passwords, slix_type, ff)) break;

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_SIGNATURE)) {
            if(!flipper_format_write_comment_cstr(
                   ff,
                   "This is the card's secp128r1 elliptic curve signature. It can not be calculated without knowing NXP's private key."))
                break;
            if(!flipper_format_write_hex(
                   ff, SLIX_SIGNATURE_KEY, data->signature, SLIX_SIGNATURE_SIZE))
                break;
        }

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_PRIVACY)) {
            if(!flipper_format_write_bool(ff, SLIX_PRIVACY_MODE_KEY, &data->privacy, 1)) break;
        }

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_PROTECTION)) {
            const SlixProtection* protection = &data->system_info.protection;
            if(!flipper_format_write_comment_cstr(ff, "Protection pointer configuration")) break;
            if(!flipper_format_write_hex(
                   ff, SLIX_PROTECTION_POINTER_KEY, &protection->pointer, sizeof(uint8_t)))
                break;
            if(!flipper_format_write_hex(
                   ff, SLIX_PROTECTION_CONDITION_KEY, &protection->condition, sizeof(uint8_t)))
                break;
        }

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_EAS) ||
           slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_PROTECTION)) {
            if(!flipper_format_write_comment_cstr(ff, "SLIX Lock Bits")) break;
        }

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_EAS)) {
            const SlixLockBits* lock_bits = &data->system_info.lock_bits;
            if(!flipper_format_write_bool(ff, SLIX_LOCK_EAS_KEY, &lock_bits->eas, 1)) break;
        }

        if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_PROTECTION)) {
            const SlixLockBits* lock_bits = &data->system_info.lock_bits;
            if(!flipper_format_write_bool(ff, SLIX_LOCK_PPL_KEY, &lock_bits->ppl, 1)) break;
        }

        saved = true;
    } while(false);

    return saved;
}

bool slix_is_equal(const SlixData* data, const SlixData* other) {
    furi_check(data);
    furi_check(other);

    return iso15693_3_is_equal(data->iso15693_3_data, other->iso15693_3_data) &&
           memcmp(&data->system_info, &other->system_info, sizeof(SlixSystemInfo)) == 0 &&
           memcmp(
               data->passwords, other->passwords, sizeof(SlixPassword) * SlixPasswordTypeCount) ==
               0 &&
           memcmp(&data->signature, &other->signature, sizeof(SlixSignature)) == 0 &&
           data->privacy == other->privacy;
}

const char* slix_get_device_name(const SlixData* data, NfcDeviceNameType name_type) {
    furi_check(data);
    UNUSED(name_type);

    const SlixType slix_type = slix_get_type(data);
    furi_assert(slix_type < SlixTypeCount);

    return slix_nfc_device_name[slix_type];
}

const uint8_t* slix_get_uid(const SlixData* data, size_t* uid_len) {
    furi_check(data);
    return iso15693_3_get_uid(data->iso15693_3_data, uid_len);
}

bool slix_set_uid(SlixData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);

    return iso15693_3_set_uid(data->iso15693_3_data, uid, uid_len);
}

const Iso15693_3Data* slix_get_base_data(const SlixData* data) {
    furi_check(data);

    return data->iso15693_3_data;
}

SlixType slix_get_type(const SlixData* data) {
    furi_check(data);

    SlixType type = SlixTypeUnknown;

    do {
        if(iso15693_3_get_manufacturer_id(data->iso15693_3_data) != SLIX_NXP_MANUFACTURER_CODE)
            break;

        const SlixUidLayout* uid = (const SlixUidLayout*)data->iso15693_3_data->uid;

        if(uid->icode_type == SLIX_TYPE_SLIX_SLIX2) {
            if(uid->type_indicator == SLIX_TYPE_INDICATOR_SLIX) {
                type = SlixTypeSlix;
            } else if(uid->type_indicator == SLIX_TYPE_INDICATOR_SLIX2) {
                type = SlixTypeSlix2;
            }
        } else if(uid->icode_type == SLIX_TYPE_SLIX_S) {
            type = SlixTypeSlixS;
        } else if(uid->icode_type == SLIX_TYPE_SLIX_L) {
            type = SlixTypeSlixL;
        }

    } while(false);

    return type;
}

SlixPassword slix_get_password(const SlixData* data, SlixPasswordType password_type) {
    furi_check(data);
    furi_check(password_type < SlixPasswordTypeCount);

    return data->passwords[password_type];
}

uint16_t slix_get_counter(const SlixData* data) {
    furi_check(data);
    const SlixCounter* counter = (const SlixCounter*)iso15693_3_get_block_data(
        data->iso15693_3_data, SLIX_COUNTER_BLOCK_NUM);

    return counter->value;
}

bool slix_is_privacy_mode(const SlixData* data) {
    furi_check(data);

    return data->privacy;
}

bool slix_is_block_protected(
    const SlixData* data,
    SlixPasswordType password_type,
    uint8_t block_num) {
    furi_check(data);
    furi_check(password_type < SlixPasswordTypeCount);

    bool ret = false;

    do {
        if(password_type != SlixPasswordTypeRead && password_type != SlixPasswordTypeWrite) break;
        if(block_num >= iso15693_3_get_block_count(data->iso15693_3_data)) break;
        if(block_num == SLIX_COUNTER_BLOCK_NUM) break;

        const bool high = block_num >= data->system_info.protection.pointer;
        const bool read = password_type == SlixPasswordTypeRead;

        const uint8_t condition = high ? (read ? SLIX_PP_CONDITION_RH : SLIX_PP_CONDITION_WH) :
                                         (read ? SLIX_PP_CONDITION_RL : SLIX_PP_CONDITION_WL);

        ret = data->system_info.protection.condition & condition;
    } while(false);

    return ret;
}

bool slix_is_counter_increment_protected(const SlixData* data) {
    furi_check(data);

    const SlixCounter* counter = (const SlixCounter*)iso15693_3_get_block_data(
        data->iso15693_3_data, SLIX_COUNTER_BLOCK_NUM);

    return counter->protection != 0;
}

bool slix_type_has_features(SlixType slix_type, SlixTypeFeatures features) {
    furi_check(slix_type < SlixTypeCount);

    return (slix_type_features[slix_type] & features) == features;
}

bool slix_type_supports_password(SlixType slix_type, SlixPasswordType password_type) {
    furi_check(slix_type < SlixTypeCount);
    furi_check(password_type < SlixPasswordTypeCount);

    return slix_type_features[slix_type] & slix_password_configs[password_type].feature_flag;
}
