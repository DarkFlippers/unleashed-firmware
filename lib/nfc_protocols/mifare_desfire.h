#pragma once

#include <m-string.h>
#include <stdint.h>
#include <stdbool.h>

#define MF_DF_GET_VERSION (0x60)
#define MF_DF_GET_FREE_MEMORY (0x6E)
#define MF_DF_GET_KEY_SETTINGS (0x45)
#define MF_DF_GET_KEY_VERSION (0x64)
#define MF_DF_GET_APPLICATION_IDS (0x6A)
#define MF_DF_SELECT_APPLICATION (0x5A)
#define MF_DF_GET_FILE_IDS (0x6F)
#define MF_DF_GET_FILE_SETTINGS (0xF5)

#define MF_DF_READ_DATA (0xBD)
#define MF_DF_GET_VALUE (0x6C)
#define MF_DF_READ_RECORDS (0xBB)

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

    uint8_t uid[7];
    uint8_t batch[5];
    uint8_t prod_week;
    uint8_t prod_year;
} MifareDesfireVersion;

typedef struct {
    uint32_t bytes;
} MifareDesfireFreeMemory; // EV1+ only

typedef struct MifareDesfireKeyVersion {
    uint8_t id;
    uint8_t version;
    struct MifareDesfireKeyVersion* next;
} MifareDesfireKeyVersion;

typedef struct {
    uint8_t change_key_id;
    bool config_changeable;
    bool free_create_delete;
    bool free_directory_list;
    bool master_key_changeable;
    uint8_t flags;
    uint8_t max_keys;
    MifareDesfireKeyVersion* key_version_head;
} MifareDesfireKeySettings;

typedef enum {
    MifareDesfireFileTypeStandard = 0,
    MifareDesfireFileTypeBackup = 1,
    MifareDesfireFileTypeValue = 2,
    MifareDesfireFileTypeLinearRecord = 3,
    MifareDesfireFileTypeCyclicRecord = 4,
} MifareDesfireFileType;

typedef enum {
    MifareDesfireFileCommunicationSettingsPlaintext = 0,
    MifareDesfireFileCommunicationSettingsAuthenticated = 1,
    MifareDesfireFileCommunicationSettingsEnciphered = 3,
} MifareDesfireFileCommunicationSettings;

typedef struct MifareDesfireFile {
    uint8_t id;
    MifareDesfireFileType type;
    MifareDesfireFileCommunicationSettings comm;
    uint16_t access_rights;
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
    } settings;
    uint8_t* contents;

    struct MifareDesfireFile* next;
} MifareDesfireFile;

typedef struct MifareDesfireApplication {
    uint8_t id[3];
    MifareDesfireKeySettings* key_settings;
    MifareDesfireFile* file_head;

    struct MifareDesfireApplication* next;
} MifareDesfireApplication;

typedef struct {
    MifareDesfireVersion version;
    MifareDesfireFreeMemory* free_memory;
    MifareDesfireKeySettings* master_key_settings;
    MifareDesfireApplication* app_head;
} MifareDesfireData;

void mf_df_clear(MifareDesfireData* data);

void mf_df_cat_data(MifareDesfireData* data, string_t out);
void mf_df_cat_card_info(MifareDesfireData* data, string_t out);
void mf_df_cat_version(MifareDesfireVersion* version, string_t out);
void mf_df_cat_free_mem(MifareDesfireFreeMemory* free_mem, string_t out);
void mf_df_cat_key_settings(MifareDesfireKeySettings* ks, string_t out);
void mf_df_cat_application_info(MifareDesfireApplication* app, string_t out);
void mf_df_cat_application(MifareDesfireApplication* app, string_t out);
void mf_df_cat_file(MifareDesfireFile* file, string_t out);

bool mf_df_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK);

uint16_t mf_df_prepare_get_version(uint8_t* dest);
bool mf_df_parse_get_version_response(uint8_t* buf, uint16_t len, MifareDesfireVersion* out);

uint16_t mf_df_prepare_get_free_memory(uint8_t* dest);
bool mf_df_parse_get_free_memory_response(uint8_t* buf, uint16_t len, MifareDesfireFreeMemory* out);

uint16_t mf_df_prepare_get_key_settings(uint8_t* dest);
bool mf_df_parse_get_key_settings_response(
    uint8_t* buf,
    uint16_t len,
    MifareDesfireKeySettings* out);

uint16_t mf_df_prepare_get_key_version(uint8_t* dest, uint8_t key_id);
bool mf_df_parse_get_key_version_response(uint8_t* buf, uint16_t len, MifareDesfireKeyVersion* out);

uint16_t mf_df_prepare_get_application_ids(uint8_t* dest);
bool mf_df_parse_get_application_ids_response(
    uint8_t* buf,
    uint16_t len,
    MifareDesfireApplication** app_head);

uint16_t mf_df_prepare_select_application(uint8_t* dest, uint8_t id[3]);
bool mf_df_parse_select_application_response(uint8_t* buf, uint16_t len);

uint16_t mf_df_prepare_get_file_ids(uint8_t* dest);
bool mf_df_parse_get_file_ids_response(uint8_t* buf, uint16_t len, MifareDesfireFile** file_head);

uint16_t mf_df_prepare_get_file_settings(uint8_t* dest, uint8_t file_id);
bool mf_df_parse_get_file_settings_response(uint8_t* buf, uint16_t len, MifareDesfireFile* out);

uint16_t mf_df_prepare_read_data(uint8_t* dest, uint8_t file_id, uint32_t offset, uint32_t len);
uint16_t mf_df_prepare_get_value(uint8_t* dest, uint8_t file_id);
uint16_t mf_df_prepare_read_records(uint8_t* dest, uint8_t file_id, uint32_t offset, uint32_t len);
bool mf_df_parse_read_data_response(uint8_t* buf, uint16_t len, MifareDesfireFile* out);
