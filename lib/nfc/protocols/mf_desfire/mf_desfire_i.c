#include "mf_desfire_i.h"

#define TAG "MfDesfire"

#define BITS_IN_BYTE (8U)

#define MF_DESFIRE_FFF_VERSION_KEY \
    MF_DESFIRE_FFF_PICC_PREFIX " " \
                               "Version"
#define MF_DESFIRE_FFF_FREE_MEM_KEY \
    MF_DESFIRE_FFF_PICC_PREFIX " "  \
                               "Free Memory"

#define MF_DESFIRE_FFF_CHANGE_KEY_ID_KEY      "Change Key ID"
#define MF_DESFIRE_FFF_CONFIG_CHANGEABLE_KEY  "Config Changeable"
#define MF_DESFIRE_FFF_FREE_CREATE_DELETE_KEY "Free Create Delete"
#define MF_DESFIRE_FFF_FREE_DIR_LIST_KEY      "Free Directory List"
#define MF_DESFIRE_FFF_KEY_CHANGEABLE_KEY     "Key Changeable"
#define MF_DESFIRE_FFF_FLAGS_KEY              "Flags"
#define MF_DESFIRE_FFF_MAX_KEYS_KEY           "Max Keys"

#define MF_DESFIRE_FFF_KEY_SUB_PREFIX  "Key"
#define MF_DESFIRE_FFF_KEY_VERSION_KEY "Version"

#define MF_DESFIRE_FFF_APPLICATION_COUNT_KEY \
    MF_DESFIRE_FFF_APP_PREFIX " "            \
                              "Count"
#define MF_DESFIRE_FFF_APPLICATION_IDS_KEY \
    MF_DESFIRE_FFF_APP_PREFIX " "          \
                              "IDs"

#define MF_DESFIRE_FFF_FILE_SUB_PREFIX "File"
#define MF_DESFIRE_FFF_FILE_IDS_KEY    \
    MF_DESFIRE_FFF_FILE_SUB_PREFIX " " \
                                   "IDs"
#define MF_DESFIRE_FFF_FILE_TYPE_KEY          "Type"
#define MF_DESFIRE_FFF_FILE_COMM_SETTINGS_KEY "Communication Settings"
#define MF_DESFIRE_FFF_FILE_ACCESS_RIGHTS_KEY "Access Rights"

#define MF_DESFIRE_FFF_FILE_SIZE_KEY "Size"

#define MF_DESFIRE_FFF_FILE_HI_LIMIT_KEY             "Hi Limit"
#define MF_DESFIRE_FFF_FILE_LO_LIMIT_KEY             "Lo Limit"
#define MF_DESFIRE_FFF_FILE_LIMIT_CREDIT_VALUE_KEY   "Limited Credit Value"
#define MF_DESFIRE_FFF_FILE_LIMIT_CREDIT_ENABLED_KEY "Limited Credit Enabled"

#define MF_DESFIRE_FFF_FILE_MAX_KEY "Max"
#define MF_DESFIRE_FFF_FILE_CUR_KEY "Cur"

bool mf_desfire_version_parse(MfDesfireVersion* data, const BitBuffer* buf) {
    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfDesfireVersion);

    if(can_parse) {
        bit_buffer_write_bytes(buf, data, sizeof(MfDesfireVersion));
    }

    return can_parse;
}

bool mf_desfire_free_memory_parse(MfDesfireFreeMemory* data, const BitBuffer* buf) {
    typedef struct FURI_PACKED {
        uint32_t bytes_free : 3 * BITS_IN_BYTE;
    } MfDesfireFreeMemoryLayout;

    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfDesfireFreeMemoryLayout);

    if(can_parse) {
        MfDesfireFreeMemoryLayout layout;
        bit_buffer_write_bytes(buf, &layout, sizeof(MfDesfireFreeMemoryLayout));
        data->bytes_free = layout.bytes_free;
    }

    data->is_present = can_parse;

    return can_parse;
}

bool mf_desfire_key_settings_parse(MfDesfireKeySettings* data, const BitBuffer* buf) {
    typedef struct FURI_PACKED {
        bool is_master_key_changeable : 1;
        bool is_free_directory_list   : 1;
        bool is_free_create_delete    : 1;
        bool is_config_changeable     : 1;
        uint8_t change_key_id         : 4;
        uint8_t max_keys              : 4;
        uint8_t flags                 : 4;
    } MfDesfireKeySettingsLayout;

    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfDesfireKeySettingsLayout);

    if(can_parse) {
        MfDesfireKeySettingsLayout layout;
        bit_buffer_write_bytes(buf, &layout, sizeof(MfDesfireKeySettingsLayout));

        data->is_master_key_changeable = layout.is_master_key_changeable;
        data->is_free_directory_list = layout.is_free_directory_list;
        data->is_free_create_delete = layout.is_free_create_delete;
        data->is_config_changeable = layout.is_config_changeable;

        data->change_key_id = layout.change_key_id;
        data->max_keys = layout.max_keys;
        data->flags = layout.flags;
    }

    return can_parse;
}

bool mf_desfire_key_version_parse(MfDesfireKeyVersion* data, const BitBuffer* buf) {
    const bool can_parse = bit_buffer_get_size_bytes(buf) == sizeof(MfDesfireKeyVersion);

    if(can_parse) {
        bit_buffer_write_bytes(buf, data, sizeof(MfDesfireKeyVersion));
    }

    return can_parse;
}

bool mf_desfire_application_id_parse(
    MfDesfireApplicationId* data,
    uint32_t index,
    const BitBuffer* buf) {
    const bool can_parse =
        bit_buffer_get_size_bytes(buf) >=
        (index * sizeof(MfDesfireApplicationId) + sizeof(MfDesfireApplicationId));

    if(can_parse) {
        bit_buffer_write_bytes_mid(
            buf, data, index * sizeof(MfDesfireApplicationId), sizeof(MfDesfireApplicationId));
    }

    return can_parse;
}

bool mf_desfire_file_id_parse(MfDesfireFileId* data, uint32_t index, const BitBuffer* buf) {
    const bool can_parse = bit_buffer_get_size_bytes(buf) >=
                           (index * sizeof(MfDesfireFileId) + sizeof(MfDesfireFileId));
    if(can_parse) {
        bit_buffer_write_bytes_mid(
            buf, data, index * sizeof(MfDesfireFileId), sizeof(MfDesfireFileId));
    }

    return can_parse;
}

bool mf_desfire_file_settings_parse(MfDesfireFileSettings* data, const BitBuffer* buf) {
    bool parsed = false;

    typedef struct FURI_PACKED {
        uint8_t type;
        uint8_t comm;
        uint16_t access_rights;
    } MfDesfireFileSettingsHeader;

    typedef struct FURI_PACKED {
        uint32_t size : 3 * BITS_IN_BYTE;
    } MfDesfireFileSettingsData;

    typedef struct FURI_PACKED {
        uint32_t lo_limit;
        uint32_t hi_limit;
        uint32_t limited_credit_value;
        uint8_t limited_credit_enabled;
    } MfDesfireFileSettingsValue;

    typedef struct FURI_PACKED {
        uint32_t size : 3 * BITS_IN_BYTE;
        uint32_t max  : 3 * BITS_IN_BYTE;
        uint32_t cur  : 3 * BITS_IN_BYTE;
    } MfDesfireFileSettingsRecord;

    typedef struct FURI_PACKED {
        MfDesfireFileSettingsHeader header;
        union {
            MfDesfireFileSettingsData data;
            MfDesfireFileSettingsValue value;
            MfDesfireFileSettingsRecord record;
        };
    } MfDesfireFileSettingsLayout;

    MfDesfireFileSettings file_settings_temp = {};
    do {
        const size_t data_size = bit_buffer_get_size_bytes(buf);
        const uint8_t* data_ptr = bit_buffer_get_data(buf);
        const size_t min_data_size =
            sizeof(MfDesfireFileSettingsHeader) + sizeof(MfDesfireFileSettingsData);

        if(data_size < min_data_size) {
            FURI_LOG_E(
                TAG, "File settings size %zu less than minimum %zu", data_size, min_data_size);
            break;
        }

        size_t bytes_processed = sizeof(MfDesfireFileSettingsHeader);
        MfDesfireFileSettingsLayout layout = {};
        memcpy(&layout.header, data_ptr, sizeof(MfDesfireFileSettingsHeader));
        bool has_additional_access_rights = (layout.header.comm & 0x80) != 0;

        file_settings_temp.type = layout.header.type;
        file_settings_temp.comm = layout.header.comm & 0x03;
        file_settings_temp.access_rights_len = 1;
        file_settings_temp.access_rights[0] = layout.header.access_rights;

        if(file_settings_temp.type == MfDesfireFileTypeStandard ||
           file_settings_temp.type == MfDesfireFileTypeBackup) {
            memcpy(
                &layout.data,
                &data_ptr[sizeof(MfDesfireFileSettingsHeader)],
                sizeof(MfDesfireFileSettingsData));
            file_settings_temp.data.size = layout.data.size;
            bytes_processed += sizeof(MfDesfireFileSettingsData);
        } else if(file_settings_temp.type == MfDesfireFileTypeValue) {
            memcpy(
                &layout.value,
                &data_ptr[sizeof(MfDesfireFileSettingsHeader)],
                sizeof(MfDesfireFileSettingsValue));
            file_settings_temp.value.lo_limit = layout.value.lo_limit;
            file_settings_temp.value.hi_limit = layout.value.hi_limit;
            file_settings_temp.value.limited_credit_value = layout.value.limited_credit_value;
            file_settings_temp.value.limited_credit_enabled = layout.value.limited_credit_enabled;
            bytes_processed += sizeof(MfDesfireFileSettingsValue);
        } else if(
            file_settings_temp.type == MfDesfireFileTypeLinearRecord ||
            file_settings_temp.type == MfDesfireFileTypeCyclicRecord) {
            memcpy(
                &layout.record,
                &data_ptr[sizeof(MfDesfireFileSettingsHeader)],
                sizeof(MfDesfireFileSettingsRecord));
            file_settings_temp.record.size = layout.record.size;
            file_settings_temp.record.max = layout.record.max;
            file_settings_temp.record.cur = layout.record.cur;
            bytes_processed += sizeof(MfDesfireFileSettingsRecord);
        } else {
            FURI_LOG_W(TAG, "Unknown file type: %02x", file_settings_temp.type);
            break;
        }

        if(has_additional_access_rights) {
            uint8_t additional_access_rights_len = bit_buffer_get_byte(buf, bytes_processed);
            FURI_LOG_D(TAG, "Has additional rights: %d", additional_access_rights_len);
            if(data_size != bytes_processed +
                                additional_access_rights_len * sizeof(MfDesfireFileAccessRights) +
                                1) {
                FURI_LOG_W(TAG, "Unexpected command length: %zu", data_size);
                for(size_t i = 0; i < bit_buffer_get_size_bytes(buf); i++) {
                    printf("%02X ", bit_buffer_get_byte(buf, i));
                }
                printf("\r\n");
                break;
            }
            if(additional_access_rights_len >
               MF_DESFIRE_MAX_KEYS * sizeof(MfDesfireFileAccessRights))
                break;

            memcpy(
                &file_settings_temp.access_rights[1],
                &data_ptr[bytes_processed],
                additional_access_rights_len * sizeof(MfDesfireFileAccessRights));
            file_settings_temp.access_rights_len += additional_access_rights_len;
        }

        *data = file_settings_temp;
        parsed = true;
    } while(false);

    return parsed;
}

bool mf_desfire_file_data_parse(MfDesfireFileData* data, const BitBuffer* buf) {
    const size_t data_size = bit_buffer_get_size_bytes(buf);

    if(data_size > 0) {
        simple_array_init(data->data, data_size);
        bit_buffer_write_bytes(buf, simple_array_get_data(data->data), data_size);
    }

    // Success no matter whether there is data or not
    return true;
}

void mf_desfire_file_data_init(MfDesfireFileData* data) {
    data->data = simple_array_alloc(&simple_array_config_uint8_t);
}

void mf_desfire_application_init(MfDesfireApplication* data) {
    data->key_versions = simple_array_alloc(&mf_desfire_key_version_array_config);
    data->file_ids = simple_array_alloc(&mf_desfire_file_id_array_config);
    data->file_settings = simple_array_alloc(&mf_desfire_file_settings_array_config);
    data->file_data = simple_array_alloc(&mf_desfire_file_data_array_config);
}

void mf_desfire_file_data_reset(MfDesfireFileData* data) {
    simple_array_free(data->data);
    memset(data, 0, sizeof(MfDesfireFileData));
}

void mf_desfire_application_reset(MfDesfireApplication* data) {
    simple_array_free(data->key_versions);
    simple_array_free(data->file_ids);
    simple_array_free(data->file_settings);
    simple_array_free(data->file_data);
    memset(data, 0, sizeof(MfDesfireApplication));
}

void mf_desfire_file_data_copy(MfDesfireFileData* data, const MfDesfireFileData* other) {
    simple_array_copy(data->data, other->data);
}

void mf_desfire_application_copy(MfDesfireApplication* data, const MfDesfireApplication* other) {
    data->key_settings = other->key_settings;
    simple_array_copy(data->key_versions, other->key_versions);
    simple_array_copy(data->file_ids, other->file_ids);
    simple_array_copy(data->file_settings, other->file_settings);
    simple_array_copy(data->file_data, other->file_data);
}

bool mf_desfire_version_load(MfDesfireVersion* data, FlipperFormat* ff) {
    return flipper_format_read_hex(
        ff, MF_DESFIRE_FFF_VERSION_KEY, (uint8_t*)data, sizeof(MfDesfireVersion));
}

bool mf_desfire_free_memory_load(MfDesfireFreeMemory* data, FlipperFormat* ff) {
    data->is_present = flipper_format_key_exist(ff, MF_DESFIRE_FFF_FREE_MEM_KEY);
    return data->is_present ?
               flipper_format_read_uint32(ff, MF_DESFIRE_FFF_FREE_MEM_KEY, &data->bytes_free, 1) :
               true;
}

bool mf_desfire_key_settings_load(
    MfDesfireKeySettings* data,
    const char* prefix,
    FlipperFormat* ff) {
    bool success = false;

    FuriString* key = furi_string_alloc();

    do {
        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_CHANGE_KEY_ID_KEY);
        if(!flipper_format_read_hex(ff, furi_string_get_cstr(key), &data->change_key_id, 1)) break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_CONFIG_CHANGEABLE_KEY);
        if(!flipper_format_read_bool(ff, furi_string_get_cstr(key), &data->is_config_changeable, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FREE_CREATE_DELETE_KEY);
        if(!flipper_format_read_bool(
               ff, furi_string_get_cstr(key), &data->is_free_create_delete, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FREE_DIR_LIST_KEY);
        if(!flipper_format_read_bool(
               ff, furi_string_get_cstr(key), &data->is_free_directory_list, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_KEY_CHANGEABLE_KEY);
        if(!flipper_format_read_bool(
               ff, furi_string_get_cstr(key), &data->is_master_key_changeable, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FLAGS_KEY);
        if(flipper_format_key_exist(ff, furi_string_get_cstr(key))) {
            if(!flipper_format_read_hex(ff, furi_string_get_cstr(key), &data->flags, 1)) break;
        }

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_MAX_KEYS_KEY);
        if(!flipper_format_read_hex(ff, furi_string_get_cstr(key), &data->max_keys, 1)) break;

        success = true;
    } while(false);

    furi_string_free(key);
    return success;
}

bool mf_desfire_key_version_load(
    MfDesfireKeyVersion* data,
    const char* prefix,
    uint32_t index,
    FlipperFormat* ff) {
    FuriString* key = furi_string_alloc_printf(
        "%s %s %lu %s",
        prefix,
        MF_DESFIRE_FFF_KEY_SUB_PREFIX,
        index,
        MF_DESFIRE_FFF_KEY_VERSION_KEY);
    const bool success = flipper_format_read_hex(ff, furi_string_get_cstr(key), data, 1);
    furi_string_free(key);
    return success;
}

bool mf_desfire_file_count_load(uint32_t* data, const char* prefix, FlipperFormat* ff) {
    FuriString* key = furi_string_alloc_printf("%s %s", prefix, MF_DESFIRE_FFF_FILE_IDS_KEY);
    const bool success = flipper_format_get_value_count(ff, furi_string_get_cstr(key), data);
    furi_string_free(key);
    return success;
}

bool mf_desfire_file_ids_load(
    MfDesfireFileId* data,
    uint32_t count,
    const char* prefix,
    FlipperFormat* ff) {
    FuriString* key = furi_string_alloc_printf("%s %s", prefix, MF_DESFIRE_FFF_FILE_IDS_KEY);
    const bool success = flipper_format_read_hex(ff, furi_string_get_cstr(key), data, count);
    furi_string_free(key);
    return success;
}

bool mf_desfire_file_settings_load(
    MfDesfireFileSettings* data,
    const char* prefix,
    FlipperFormat* ff) {
    bool success = false;

    FuriString* key = furi_string_alloc();

    do {
        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_TYPE_KEY);
        if(!flipper_format_read_hex(ff, furi_string_get_cstr(key), (uint8_t*)&data->type, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_COMM_SETTINGS_KEY);
        if(!flipper_format_read_hex(ff, furi_string_get_cstr(key), (uint8_t*)&data->comm, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_ACCESS_RIGHTS_KEY);
        uint32_t access_rights_len = 0;
        if(!flipper_format_get_value_count(ff, furi_string_get_cstr(key), &access_rights_len))
            break;
        if((access_rights_len == 0) || ((access_rights_len % 2) != 0)) break;
        if(!flipper_format_read_hex(
               ff, furi_string_get_cstr(key), (uint8_t*)&data->access_rights, access_rights_len))
            break;
        data->access_rights_len = access_rights_len / sizeof(MfDesfireFileAccessRights);

        if(data->type == MfDesfireFileTypeStandard || data->type == MfDesfireFileTypeBackup) {
            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_SIZE_KEY);
            if(!flipper_format_read_uint32(ff, furi_string_get_cstr(key), &data->data.size, 1))
                break;
        } else if(data->type == MfDesfireFileTypeValue) {
            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_HI_LIMIT_KEY);
            if(!flipper_format_read_uint32(ff, furi_string_get_cstr(key), &data->value.hi_limit, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_LO_LIMIT_KEY);
            if(!flipper_format_read_uint32(ff, furi_string_get_cstr(key), &data->value.lo_limit, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_LIMIT_CREDIT_VALUE_KEY);
            if(!flipper_format_read_uint32(
                   ff, furi_string_get_cstr(key), &data->value.limited_credit_value, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_LIMIT_CREDIT_ENABLED_KEY);
            if(!flipper_format_read_bool(
                   ff, furi_string_get_cstr(key), &data->value.limited_credit_enabled, 1))
                break;
        } else if(
            data->type == MfDesfireFileTypeLinearRecord ||
            data->type == MfDesfireFileTypeCyclicRecord) {
            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_SIZE_KEY);
            if(!flipper_format_read_uint32(ff, furi_string_get_cstr(key), &data->record.size, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_MAX_KEY);
            if(!flipper_format_read_uint32(ff, furi_string_get_cstr(key), &data->record.max, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_CUR_KEY);
            if(!flipper_format_read_uint32(ff, furi_string_get_cstr(key), &data->record.cur, 1))
                break;
        }

        success = true;
    } while(false);

    furi_string_free(key);

    return success;
}

bool mf_desfire_file_data_load(MfDesfireFileData* data, const char* prefix, FlipperFormat* ff) {
    bool success = false;
    do {
        if(!flipper_format_key_exist(ff, prefix)) {
            success = true;
            break;
        }

        uint32_t data_size;
        if(!flipper_format_get_value_count(ff, prefix, &data_size)) break;

        simple_array_init(data->data, data_size);

        if(!flipper_format_read_hex(ff, prefix, simple_array_get_data(data->data), data_size))
            break;

        success = true;
    } while(false);

    return success;
}

bool mf_desfire_application_count_load(uint32_t* data, FlipperFormat* ff) {
    return flipper_format_read_uint32(ff, MF_DESFIRE_FFF_APPLICATION_COUNT_KEY, data, 1);
}

bool mf_desfire_application_ids_load(
    MfDesfireApplicationId* data,
    uint32_t count,
    FlipperFormat* ff) {
    return flipper_format_read_hex(
        ff, MF_DESFIRE_FFF_APPLICATION_IDS_KEY, data->data, count * sizeof(MfDesfireApplicationId));
}

bool mf_desfire_application_load(MfDesfireApplication* data, const char* prefix, FlipperFormat* ff) {
    FuriString* sub_prefix = furi_string_alloc();
    bool success = false;

    do {
        if(!mf_desfire_key_settings_load(&data->key_settings, prefix, ff)) break;

        uint32_t i;
        const uint32_t key_version_count = data->key_settings.max_keys;
        if(key_version_count) {
            simple_array_init(data->key_versions, key_version_count);

            for(i = 0; i < key_version_count; ++i) {
                if(!mf_desfire_key_version_load(
                       simple_array_get(data->key_versions, i), prefix, i, ff))
                    break;
            }

            if(i != key_version_count) break;
        }

        uint32_t file_count;
        if(!mf_desfire_file_count_load(&file_count, prefix, ff)) {
            success = true;
            break;
        }

        simple_array_init(data->file_ids, file_count);
        if(!mf_desfire_file_ids_load(simple_array_get_data(data->file_ids), file_count, prefix, ff))
            break;

        simple_array_init(data->file_settings, file_count);
        simple_array_init(data->file_data, file_count);

        for(i = 0; i < file_count; ++i) {
            const MfDesfireFileId* file_id = simple_array_cget(data->file_ids, i);
            furi_string_printf(
                sub_prefix, "%s %s %u", prefix, MF_DESFIRE_FFF_FILE_SUB_PREFIX, *file_id);

            MfDesfireFileSettings* file_settings = simple_array_get(data->file_settings, i);
            if(!mf_desfire_file_settings_load(file_settings, furi_string_get_cstr(sub_prefix), ff))
                break;

            MfDesfireFileData* file_data = simple_array_get(data->file_data, i);
            if(!mf_desfire_file_data_load(file_data, furi_string_get_cstr(sub_prefix), ff)) break;
        }

        if(i != file_count) break;

        success = true;
    } while(false);

    furi_string_free(sub_prefix);
    return success;
}

bool mf_desfire_version_save(const MfDesfireVersion* data, FlipperFormat* ff) {
    return flipper_format_write_hex(
        ff, MF_DESFIRE_FFF_VERSION_KEY, (const uint8_t*)data, sizeof(MfDesfireVersion));
}

bool mf_desfire_free_memory_save(const MfDesfireFreeMemory* data, FlipperFormat* ff) {
    return data->is_present ?
               flipper_format_write_uint32(ff, MF_DESFIRE_FFF_FREE_MEM_KEY, &data->bytes_free, 1) :
               true;
}

bool mf_desfire_key_settings_save(
    const MfDesfireKeySettings* data,
    const char* prefix,
    FlipperFormat* ff) {
    bool success = false;

    FuriString* key = furi_string_alloc();

    do {
        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_CHANGE_KEY_ID_KEY);
        if(!flipper_format_write_hex(ff, furi_string_get_cstr(key), &data->change_key_id, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_CONFIG_CHANGEABLE_KEY);
        if(!flipper_format_write_bool(
               ff, furi_string_get_cstr(key), &data->is_config_changeable, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FREE_CREATE_DELETE_KEY);
        if(!flipper_format_write_bool(
               ff, furi_string_get_cstr(key), &data->is_free_create_delete, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FREE_DIR_LIST_KEY);
        if(!flipper_format_write_bool(
               ff, furi_string_get_cstr(key), &data->is_free_directory_list, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_KEY_CHANGEABLE_KEY);
        if(!flipper_format_write_bool(
               ff, furi_string_get_cstr(key), &data->is_master_key_changeable, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FLAGS_KEY);
        if(!flipper_format_write_hex(ff, furi_string_get_cstr(key), &data->flags, 1)) break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_MAX_KEYS_KEY);
        if(!flipper_format_write_hex(ff, furi_string_get_cstr(key), &data->max_keys, 1)) break;

        success = true;
    } while(false);

    furi_string_free(key);
    return success;
}

bool mf_desfire_key_version_save(
    const MfDesfireKeyVersion* data,
    const char* prefix,
    uint32_t index,
    FlipperFormat* ff) {
    FuriString* key = furi_string_alloc_printf(
        "%s %s %lu %s",
        prefix,
        MF_DESFIRE_FFF_KEY_SUB_PREFIX,
        index,
        MF_DESFIRE_FFF_KEY_VERSION_KEY);
    const bool success = flipper_format_write_hex(ff, furi_string_get_cstr(key), data, 1);
    furi_string_free(key);
    return success;
}

bool mf_desfire_file_ids_save(
    const MfDesfireFileId* data,
    uint32_t count,
    const char* prefix,
    FlipperFormat* ff) {
    FuriString* key = furi_string_alloc_printf("%s %s", prefix, MF_DESFIRE_FFF_FILE_IDS_KEY);
    const bool success = flipper_format_write_hex(ff, furi_string_get_cstr(key), data, count);
    furi_string_free(key);
    return success;
}

bool mf_desfire_file_settings_save(
    const MfDesfireFileSettings* data,
    const char* prefix,
    FlipperFormat* ff) {
    bool success = false;

    FuriString* key = furi_string_alloc();

    do {
        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_TYPE_KEY);
        if(!flipper_format_write_hex(ff, furi_string_get_cstr(key), (const uint8_t*)&data->type, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_COMM_SETTINGS_KEY);
        if(!flipper_format_write_hex(ff, furi_string_get_cstr(key), (const uint8_t*)&data->comm, 1))
            break;

        furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_ACCESS_RIGHTS_KEY);
        if(!flipper_format_write_hex(
               ff,
               furi_string_get_cstr(key),
               (const uint8_t*)data->access_rights,
               data->access_rights_len * sizeof(MfDesfireFileAccessRights)))
            break;

        if(data->type == MfDesfireFileTypeStandard || data->type == MfDesfireFileTypeBackup) {
            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_SIZE_KEY);
            if(!flipper_format_write_uint32(ff, furi_string_get_cstr(key), &data->data.size, 1))
                break;

        } else if(data->type == MfDesfireFileTypeValue) {
            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_HI_LIMIT_KEY);
            if(!flipper_format_write_uint32(
                   ff, furi_string_get_cstr(key), &data->value.hi_limit, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_LO_LIMIT_KEY);
            if(!flipper_format_write_uint32(
                   ff, furi_string_get_cstr(key), &data->value.lo_limit, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_LIMIT_CREDIT_VALUE_KEY);
            if(!flipper_format_write_uint32(
                   ff, furi_string_get_cstr(key), &data->value.limited_credit_value, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_LIMIT_CREDIT_ENABLED_KEY);
            if(!flipper_format_write_bool(
                   ff, furi_string_get_cstr(key), &data->value.limited_credit_enabled, 1))
                break;
        } else if(
            data->type == MfDesfireFileTypeLinearRecord ||
            data->type == MfDesfireFileTypeCyclicRecord) {
            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_SIZE_KEY);
            if(!flipper_format_write_uint32(ff, furi_string_get_cstr(key), &data->record.size, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_MAX_KEY);
            if(!flipper_format_write_uint32(ff, furi_string_get_cstr(key), &data->record.max, 1))
                break;

            furi_string_printf(key, "%s %s", prefix, MF_DESFIRE_FFF_FILE_CUR_KEY);
            if(!flipper_format_write_uint32(ff, furi_string_get_cstr(key), &data->record.cur, 1))
                break;
        }

        success = true;
    } while(false);

    furi_string_free(key);
    return success;
}

bool mf_desfire_file_data_save(
    const MfDesfireFileData* data,
    const char* prefix,
    FlipperFormat* ff) {
    const uint32_t data_size = simple_array_get_count(data->data);
    return data_size > 0 ? flipper_format_write_hex(
                               ff, prefix, simple_array_cget_data(data->data), data_size) :
                           true;
}

bool mf_desfire_application_count_save(const uint32_t* data, FlipperFormat* ff) {
    return flipper_format_write_uint32(ff, MF_DESFIRE_FFF_APPLICATION_COUNT_KEY, data, 1);
}

bool mf_desfire_application_ids_save(
    const MfDesfireApplicationId* data,
    uint32_t count,
    FlipperFormat* ff) {
    return flipper_format_write_hex(
        ff, MF_DESFIRE_FFF_APPLICATION_IDS_KEY, data->data, count * sizeof(MfDesfireApplicationId));
}

bool mf_desfire_application_save(
    const MfDesfireApplication* data,
    const char* prefix,
    FlipperFormat* ff) {
    FuriString* sub_prefix = furi_string_alloc();
    bool success = false;

    do {
        if(!mf_desfire_key_settings_save(&data->key_settings, prefix, ff)) break;

        const uint32_t key_version_count = data->key_settings.max_keys;

        uint32_t i;
        for(i = 0; i < key_version_count; ++i) {
            if(!mf_desfire_key_version_save(
                   simple_array_cget(data->key_versions, i), prefix, i, ff))
                break;
        }

        if(i != key_version_count) break;

        const uint32_t file_count = simple_array_get_count(data->file_ids);
        if(file_count > 0) {
            if(!mf_desfire_file_ids_save(
                   simple_array_get_data(data->file_ids), file_count, prefix, ff))
                break;
        }

        for(i = 0; i < file_count; ++i) {
            const MfDesfireFileId* file_id = simple_array_cget(data->file_ids, i);
            furi_string_printf(
                sub_prefix, "%s %s %u", prefix, MF_DESFIRE_FFF_FILE_SUB_PREFIX, *file_id);

            const MfDesfireFileSettings* file_settings = simple_array_cget(data->file_settings, i);
            if(!mf_desfire_file_settings_save(file_settings, furi_string_get_cstr(sub_prefix), ff))
                break;

            const MfDesfireFileData* file_data = simple_array_cget(data->file_data, i);
            if(!mf_desfire_file_data_save(file_data, furi_string_get_cstr(sub_prefix), ff)) break;
        }

        if(i != file_count) break;

        success = true;
    } while(false);

    furi_string_free(sub_prefix);
    return success;
}

const SimpleArrayConfig mf_desfire_key_version_array_config = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(MfDesfireKeyVersion),
};

const SimpleArrayConfig mf_desfire_app_id_array_config = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(MfDesfireApplicationId),
};

const SimpleArrayConfig mf_desfire_file_id_array_config = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(MfDesfireFileId),
};

const SimpleArrayConfig mf_desfire_file_settings_array_config = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(MfDesfireFileSettings),
};

const SimpleArrayConfig mf_desfire_file_data_array_config = {
    .init = (SimpleArrayInit)mf_desfire_file_data_init,
    .copy = (SimpleArrayCopy)mf_desfire_file_data_copy,
    .reset = (SimpleArrayReset)mf_desfire_file_data_reset,
    .type_size = sizeof(MfDesfireData),
};

const SimpleArrayConfig mf_desfire_application_array_config = {
    .init = (SimpleArrayInit)mf_desfire_application_init,
    .copy = (SimpleArrayCopy)mf_desfire_application_copy,
    .reset = (SimpleArrayReset)mf_desfire_application_reset,
    .type_size = sizeof(MfDesfireApplication),
};
