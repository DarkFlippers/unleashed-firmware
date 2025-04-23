#include "mf_desfire_i.h"

#include <furi.h>

#define MF_DESFIRE_PROTOCOL_NAME "Mifare DESFire"

#define MF_DESFIRE_HW_MINOR_TYPE          (0x00)
#define MF_DESFIRE_HW_MINOR_TYPE_MF3ICD40 (0x02)

#define MF_DESFIRE_HW_MAJOR_TYPE_EV1      (0x01)
#define MF_DESFIRE_HW_MAJOR_TYPE_EV2      (0x12)
#define MF_DESFIRE_HW_MAJOR_TYPE_EV2_XL   (0x22)
#define MF_DESFIRE_HW_MAJOR_TYPE_EV3      (0x33)
#define MF_DESFIRE_HW_MAJOR_TYPE_MF3ICD40 (0x00)

#define MF_DESFIRE_STORAGE_SIZE_2K       (0x16)
#define MF_DESFIRE_STORAGE_SIZE_4K       (0x18)
#define MF_DESFIRE_STORAGE_SIZE_8K       (0x1A)
#define MF_DESFIRE_STORAGE_SIZE_16K      (0x1C)
#define MF_DESFIRE_STORAGE_SIZE_32K      (0x1E)
#define MF_DESFIRE_STORAGE_SIZE_MF3ICD40 (0xFF)
#define MF_DESFIRE_STORAGE_SIZE_UNKNOWN  (0xFF)

#define MF_DESFIRE_TEST_TYPE_MF3ICD40(major, minor, storage) \
    (((major) == MF_DESFIRE_HW_MAJOR_TYPE_MF3ICD40) &&       \
     ((minor) == MF_DESFIRE_HW_MINOR_TYPE_MF3ICD40) &&       \
     ((storage) == MF_DESFIRE_STORAGE_SIZE_MF3ICD40))

static const char* mf_desfire_type_strings[] = {
    [MfDesfireTypeMF3ICD40] = "(MF3ICD40)",
    [MfDesfireTypeEV1] = "EV1",
    [MfDesfireTypeEV2] = "EV2",
    [MfDesfireTypeEV2XL] = "EV2 XL",
    [MfDesfireTypeEV3] = "EV3",
    [MfDesfireTypeUnknown] = "UNK",
};

static const char* mf_desfire_size_strings[] = {
    [MfDesfireSize2k] = "2K",
    [MfDesfireSize4k] = "4K",
    [MfDesfireSize8k] = "8K",
    [MfDesfireSize16k] = "16K",
    [MfDesfireSize32k] = "32K",
    [MfDesfireSizeUnknown] = "",
};

const NfcDeviceBase nfc_device_mf_desfire = {
    .protocol_name = MF_DESFIRE_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)mf_desfire_alloc,
    .free = (NfcDeviceFree)mf_desfire_free,
    .reset = (NfcDeviceReset)mf_desfire_reset,
    .copy = (NfcDeviceCopy)mf_desfire_copy,
    .verify = (NfcDeviceVerify)mf_desfire_verify,
    .load = (NfcDeviceLoad)mf_desfire_load,
    .save = (NfcDeviceSave)mf_desfire_save,
    .is_equal = (NfcDeviceEqual)mf_desfire_is_equal,
    .get_name = (NfcDeviceGetName)mf_desfire_get_device_name,
    .get_uid = (NfcDeviceGetUid)mf_desfire_get_uid,
    .set_uid = (NfcDeviceSetUid)mf_desfire_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)mf_desfire_get_base_data,
};

MfDesfireData* mf_desfire_alloc(void) {
    MfDesfireData* data = malloc(sizeof(MfDesfireData));
    data->iso14443_4a_data = iso14443_4a_alloc();
    data->master_key_versions = simple_array_alloc(&mf_desfire_key_version_array_config);
    data->application_ids = simple_array_alloc(&mf_desfire_app_id_array_config);
    data->applications = simple_array_alloc(&mf_desfire_application_array_config);
    data->device_name = furi_string_alloc();
    return data;
}

void mf_desfire_free(MfDesfireData* data) {
    furi_check(data);

    mf_desfire_reset(data);
    simple_array_free(data->applications);
    simple_array_free(data->application_ids);
    simple_array_free(data->master_key_versions);
    iso14443_4a_free(data->iso14443_4a_data);
    furi_string_free(data->device_name);
    free(data);
}

void mf_desfire_reset(MfDesfireData* data) {
    furi_check(data);

    iso14443_4a_reset(data->iso14443_4a_data);

    memset(&data->version, 0, sizeof(MfDesfireVersion));
    memset(&data->free_memory, 0, sizeof(MfDesfireFreeMemory));

    simple_array_reset(data->master_key_versions);
    simple_array_reset(data->application_ids);
    simple_array_reset(data->applications);
}

void mf_desfire_copy(MfDesfireData* data, const MfDesfireData* other) {
    furi_check(data);
    furi_check(other);

    mf_desfire_reset(data);

    iso14443_4a_copy(data->iso14443_4a_data, other->iso14443_4a_data);

    data->version = other->version;
    data->free_memory = other->free_memory;
    data->master_key_settings = other->master_key_settings;

    simple_array_copy(data->master_key_versions, other->master_key_versions);
    simple_array_copy(data->application_ids, other->application_ids);
    simple_array_copy(data->applications, other->applications);
}

bool mf_desfire_verify(MfDesfireData* data, const FuriString* device_type) {
    UNUSED(data);
    return furi_string_equal_str(device_type, MF_DESFIRE_PROTOCOL_NAME);
}

bool mf_desfire_load(MfDesfireData* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    FuriString* prefix = furi_string_alloc();

    bool success = false;

    do {
        if(!iso14443_4a_load(data->iso14443_4a_data, ff, version)) break;

        if(!mf_desfire_version_load(&data->version, ff)) break;
        if(!mf_desfire_free_memory_load(&data->free_memory, ff)) break;
        if(!mf_desfire_key_settings_load(
               &data->master_key_settings, MF_DESFIRE_FFF_PICC_PREFIX, ff))
            break;

        const uint32_t master_key_version_count = data->master_key_settings.max_keys;
        simple_array_init(data->master_key_versions, master_key_version_count);

        uint32_t i;
        for(i = 0; i < master_key_version_count; ++i) {
            if(!mf_desfire_key_version_load(
                   simple_array_get(data->master_key_versions, i),
                   MF_DESFIRE_FFF_PICC_PREFIX,
                   i,
                   ff))
                break;
        }

        if(i != master_key_version_count) break;

        uint32_t application_count;
        if(!mf_desfire_application_count_load(&application_count, ff)) break;

        if(application_count > 0) {
            simple_array_init(data->application_ids, application_count);
            if(!mf_desfire_application_ids_load(
                   simple_array_get_data(data->application_ids), application_count, ff))
                break;

            simple_array_init(data->applications, application_count);
            for(i = 0; i < application_count; ++i) {
                const MfDesfireApplicationId* app_id = simple_array_cget(data->application_ids, i);
                furi_string_printf(
                    prefix,
                    "%s %02x%02x%02x",
                    MF_DESFIRE_FFF_APP_PREFIX,
                    app_id->data[0],
                    app_id->data[1],
                    app_id->data[2]);

                if(!mf_desfire_application_load(
                       simple_array_get(data->applications, i), furi_string_get_cstr(prefix), ff))
                    break;
            }

            if(i != application_count) break;
        }

        success = true;
    } while(false);

    furi_string_free(prefix);
    return success;
}

bool mf_desfire_save(const MfDesfireData* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    FuriString* prefix = furi_string_alloc();

    bool success = false;

    do {
        if(!iso14443_4a_save(data->iso14443_4a_data, ff)) break;

        if(!flipper_format_write_comment_cstr(ff, MF_DESFIRE_PROTOCOL_NAME " specific data"))
            break;
        if(!mf_desfire_version_save(&data->version, ff)) break;
        if(!mf_desfire_free_memory_save(&data->free_memory, ff)) break;
        if(!mf_desfire_key_settings_save(
               &data->master_key_settings, MF_DESFIRE_FFF_PICC_PREFIX, ff))
            break;

        const uint32_t master_key_version_count =
            simple_array_get_count(data->master_key_versions);

        uint32_t i;
        for(i = 0; i < master_key_version_count; ++i) {
            if(!mf_desfire_key_version_save(
                   simple_array_cget(data->master_key_versions, i),
                   MF_DESFIRE_FFF_PICC_PREFIX,
                   i,
                   ff))
                break;
        }

        if(i != master_key_version_count) break;

        const uint32_t application_count = simple_array_get_count(data->application_ids);
        if(!mf_desfire_application_count_save(&application_count, ff)) break;

        if(application_count > 0) {
            if(!mf_desfire_application_ids_save(
                   simple_array_cget_data(data->application_ids), application_count, ff))
                break;

            for(i = 0; i < application_count; ++i) {
                const MfDesfireApplicationId* app_id = simple_array_cget(data->application_ids, i);
                furi_string_printf(
                    prefix,
                    "%s %02x%02x%02x",
                    MF_DESFIRE_FFF_APP_PREFIX,
                    app_id->data[0],
                    app_id->data[1],
                    app_id->data[2]);

                const MfDesfireApplication* app = simple_array_cget(data->applications, i);
                if(!mf_desfire_application_save(app, furi_string_get_cstr(prefix), ff)) break;
            }

            if(i != application_count) break;
        }

        success = true;
    } while(false);

    furi_string_free(prefix);
    return success;
}

bool mf_desfire_is_equal(const MfDesfireData* data, const MfDesfireData* other) {
    furi_check(data);
    furi_check(other);

    return iso14443_4a_is_equal(data->iso14443_4a_data, other->iso14443_4a_data) &&
           memcmp(&data->version, &other->version, sizeof(MfDesfireVersion)) == 0 &&
           memcmp( //-V1103
               &data->free_memory,
               &other->free_memory,
               sizeof(MfDesfireFreeMemory)) == 0 &&
           memcmp(
               &data->master_key_settings,
               &other->master_key_settings,
               sizeof(MfDesfireKeySettings)) == 0 &&
           simple_array_is_equal(data->master_key_versions, other->master_key_versions) &&
           simple_array_is_equal(data->application_ids, other->application_ids) &&
           simple_array_is_equal(data->applications, other->applications);
}

static MfDesfireType mf_desfire_get_type_from_version(const MfDesfireVersion* const version) {
    MfDesfireType type = MfDesfireTypeUnknown;

    switch(version->hw_major) {
    case MF_DESFIRE_HW_MAJOR_TYPE_EV1:
        type = MfDesfireTypeEV1;
        break;
    case MF_DESFIRE_HW_MAJOR_TYPE_EV2:
        type = MfDesfireTypeEV2;
        break;
    case MF_DESFIRE_HW_MAJOR_TYPE_EV2_XL:
        type = MfDesfireTypeEV2XL;
        break;
    case MF_DESFIRE_HW_MAJOR_TYPE_EV3:
        type = MfDesfireTypeEV3;
        break;
    default:
        if(MF_DESFIRE_TEST_TYPE_MF3ICD40(version->hw_major, version->hw_minor, version->hw_storage))
            type = MfDesfireTypeMF3ICD40;
        break;
    }

    return type;
}

static MfDesfireSize mf_desfire_get_size_from_version(const MfDesfireVersion* const version) {
    MfDesfireSize size = MfDesfireSizeUnknown;

    switch(version->hw_storage) {
    case MF_DESFIRE_STORAGE_SIZE_2K:
        size = MfDesfireSize2k;
        break;
    case MF_DESFIRE_STORAGE_SIZE_4K:
        size = MfDesfireSize4k;
        break;
    case MF_DESFIRE_STORAGE_SIZE_8K:
        size = MfDesfireSize8k;
        break;
    case MF_DESFIRE_STORAGE_SIZE_16K:
        size = MfDesfireSize16k;
        break;
    case MF_DESFIRE_STORAGE_SIZE_32K:
        size = MfDesfireSize32k;
        break;
    default:
        if(MF_DESFIRE_TEST_TYPE_MF3ICD40(version->hw_major, version->hw_minor, version->hw_storage))
            size = MfDesfireSize4k;
        break;
    }

    return size;
}

const char* mf_desfire_get_device_name(const MfDesfireData* data, NfcDeviceNameType name_type) {
    furi_check(data);

    const MfDesfireType type = mf_desfire_get_type_from_version(&data->version);
    const MfDesfireSize size = mf_desfire_get_size_from_version(&data->version);

    if(type == MfDesfireTypeUnknown) {
        furi_string_printf(data->device_name, "Unknown %s", MF_DESFIRE_PROTOCOL_NAME);
    } else if(name_type == NfcDeviceNameTypeFull) {
        furi_string_printf(
            data->device_name,
            "%s %s %s",
            MF_DESFIRE_PROTOCOL_NAME,
            mf_desfire_type_strings[type],
            mf_desfire_size_strings[size]);
    } else {
        furi_string_printf(
            data->device_name,
            "%s %s",
            mf_desfire_type_strings[type],
            mf_desfire_size_strings[size]);
    }

    return furi_string_get_cstr(data->device_name);
}

const uint8_t* mf_desfire_get_uid(const MfDesfireData* data, size_t* uid_len) {
    furi_check(data);
    furi_check(uid_len);

    return iso14443_4a_get_uid(data->iso14443_4a_data, uid_len);
}

bool mf_desfire_set_uid(MfDesfireData* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);

    return iso14443_4a_set_uid(data->iso14443_4a_data, uid, uid_len);
}

Iso14443_4aData* mf_desfire_get_base_data(const MfDesfireData* data) {
    furi_check(data);

    return data->iso14443_4a_data;
}

const MfDesfireApplication*
    mf_desfire_get_application(const MfDesfireData* data, const MfDesfireApplicationId* app_id) {
    furi_check(data);
    furi_check(app_id);

    MfDesfireApplication* app = NULL;

    for(uint32_t i = 0; i < simple_array_get_count(data->application_ids); ++i) {
        const MfDesfireApplicationId* current_app_id = simple_array_cget(data->application_ids, i);
        if(memcmp(app_id, current_app_id, sizeof(MfDesfireApplicationId)) == 0) {
            app = simple_array_get(data->applications, i);
        }
    }

    return app;
}

const MfDesfireFileSettings*
    mf_desfire_get_file_settings(const MfDesfireApplication* data, const MfDesfireFileId* file_id) {
    furi_check(data);
    furi_check(file_id);

    MfDesfireFileSettings* file_settings = NULL;

    for(uint32_t i = 0; i < simple_array_get_count(data->file_ids); ++i) {
        const MfDesfireFileId* current_file_id = simple_array_cget(data->file_ids, i);
        if(*file_id == *current_file_id) {
            file_settings = simple_array_get(data->file_settings, i);
        }
    }

    return file_settings;
}

const MfDesfireFileData*
    mf_desfire_get_file_data(const MfDesfireApplication* data, const MfDesfireFileId* file_id) {
    furi_check(data);
    furi_check(file_id);

    MfDesfireFileData* file_data = NULL;

    for(uint32_t i = 0; i < simple_array_get_count(data->file_ids); ++i) {
        const MfDesfireFileId* current_file_id = simple_array_cget(data->file_ids, i);
        if(*file_id == *current_file_id) {
            file_data = simple_array_get(data->file_data, i);
        }
    }

    return file_data;
}
