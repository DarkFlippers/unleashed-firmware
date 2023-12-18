#pragma once

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>

#include <lib/toolbox/simple_array.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MF_DESFIRE_CMD_GET_VERSION (0x60)
#define MF_DESFIRE_CMD_GET_FREE_MEMORY (0x6E)
#define MF_DESFIRE_CMD_GET_KEY_SETTINGS (0x45)
#define MF_DESFIRE_CMD_GET_KEY_VERSION (0x64)
#define MF_DESFIRE_CMD_GET_APPLICATION_IDS (0x6A)
#define MF_DESFIRE_CMD_SELECT_APPLICATION (0x5A)
#define MF_DESFIRE_CMD_GET_FILE_IDS (0x6F)
#define MF_DESFIRE_CMD_GET_FILE_SETTINGS (0xF5)

#define MF_DESFIRE_CMD_READ_DATA (0xBD)
#define MF_DESFIRE_CMD_GET_VALUE (0x6C)
#define MF_DESFIRE_CMD_READ_RECORDS (0xBB)

#define MF_DESFIRE_FLAG_HAS_NEXT (0xAF)

#define MF_DESFIRE_MAX_KEYS (14)
#define MF_DESFIRE_MAX_FILES (32)

#define MF_DESFIRE_UID_SIZE (7)
#define MF_DESFIRE_BATCH_SIZE (5)
#define MF_DESFIRE_APP_ID_SIZE (3)
#define MF_DESFIRE_VALUE_SIZE (4)

typedef struct {
    uint8_t hw_vendor;
    uint8_t hw_type;
    uint8_t hw_subtype;
    uint8_t hw_major;
    uint8_t hw_minor;
    uint8_t hw_storage;
    uint8_t hw_proto;

    uint8_t sw_vendor;
    uint8_t sw_type;
    uint8_t sw_subtype;
    uint8_t sw_major;
    uint8_t sw_minor;
    uint8_t sw_storage;
    uint8_t sw_proto;

    uint8_t uid[MF_DESFIRE_UID_SIZE];
    uint8_t batch[MF_DESFIRE_BATCH_SIZE];
    uint8_t prod_week;
    uint8_t prod_year;
} MfDesfireVersion;

typedef struct {
    uint32_t bytes_free;
    bool is_present;
} MfDesfireFreeMemory; // EV1+ only

typedef struct {
    bool is_master_key_changeable;
    bool is_free_directory_list;
    bool is_free_create_delete;
    bool is_config_changeable;
    uint8_t change_key_id;
    uint8_t max_keys;
    uint8_t flags;
} MfDesfireKeySettings;

typedef uint8_t MfDesfireKeyVersion;

typedef struct {
    MfDesfireKeySettings key_settings;
    SimpleArray* key_versions;
} MfDesfireKeyConfiguration;

typedef enum {
    MfDesfireFileTypeStandard = 0,
    MfDesfireFileTypeBackup = 1,
    MfDesfireFileTypeValue = 2,
    MfDesfireFileTypeLinearRecord = 3,
    MfDesfireFileTypeCyclicRecord = 4,
} MfDesfireFileType;

typedef enum {
    MfDesfireFileCommunicationSettingsPlaintext = 0,
    MfDesfireFileCommunicationSettingsAuthenticated = 1,
    MfDesfireFileCommunicationSettingsEnciphered = 3,
} MfDesfireFileCommunicationSettings;

typedef uint8_t MfDesfireFileId;
typedef uint16_t MfDesfireFileAccessRights;

typedef struct {
    MfDesfireFileType type;
    MfDesfireFileCommunicationSettings comm;
    MfDesfireFileAccessRights access_rights;
    union {
        struct {
            uint32_t size;
        } data;
        struct {
            uint32_t lo_limit;
            uint32_t hi_limit;
            uint32_t limited_credit_value;
            bool limited_credit_enabled;
        } value;
        struct {
            uint32_t size;
            uint32_t max;
            uint32_t cur;
        } record;
    };
} MfDesfireFileSettings;

typedef struct {
    SimpleArray* data;
} MfDesfireFileData;

typedef struct {
    uint8_t data[MF_DESFIRE_APP_ID_SIZE];
} MfDesfireApplicationId;

typedef struct MfDesfireApplication {
    MfDesfireKeySettings key_settings;
    SimpleArray* key_versions;
    SimpleArray* file_ids;
    SimpleArray* file_settings;
    SimpleArray* file_data;
} MfDesfireApplication;

typedef enum {
    MfDesfireErrorNone,
    MfDesfireErrorNotPresent,
    MfDesfireErrorProtocol,
    MfDesfireErrorTimeout,
} MfDesfireError;

typedef struct {
    Iso14443_4aData* iso14443_4a_data;
    MfDesfireVersion version;
    MfDesfireFreeMemory free_memory;
    MfDesfireKeySettings master_key_settings;
    SimpleArray* master_key_versions;
    SimpleArray* application_ids;
    SimpleArray* applications;
} MfDesfireData;

extern const NfcDeviceBase nfc_device_mf_desfire;

// Virtual methods

MfDesfireData* mf_desfire_alloc();

void mf_desfire_free(MfDesfireData* data);

void mf_desfire_reset(MfDesfireData* data);

void mf_desfire_copy(MfDesfireData* data, const MfDesfireData* other);

bool mf_desfire_verify(MfDesfireData* data, const FuriString* device_type);

bool mf_desfire_load(MfDesfireData* data, FlipperFormat* ff, uint32_t version);

bool mf_desfire_save(const MfDesfireData* data, FlipperFormat* ff);

bool mf_desfire_is_equal(const MfDesfireData* data, const MfDesfireData* other);

const char* mf_desfire_get_device_name(const MfDesfireData* data, NfcDeviceNameType name_type);

const uint8_t* mf_desfire_get_uid(const MfDesfireData* data, size_t* uid_len);

bool mf_desfire_set_uid(MfDesfireData* data, const uint8_t* uid, size_t uid_len);

Iso14443_4aData* mf_desfire_get_base_data(const MfDesfireData* data);

// Getters and tests

const MfDesfireApplication*
    mf_desfire_get_application(const MfDesfireData* data, const MfDesfireApplicationId* app_id);

const MfDesfireFileSettings*
    mf_desfire_get_file_settings(const MfDesfireApplication* data, const MfDesfireFileId* file_id);

const MfDesfireFileData*
    mf_desfire_get_file_data(const MfDesfireApplication* data, const MfDesfireFileId* file_id);

#ifdef __cplusplus
}
#endif
