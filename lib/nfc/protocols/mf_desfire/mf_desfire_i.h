#pragma once

#include "mf_desfire.h"

#define MF_DESFIRE_FFF_PICC_PREFIX "PICC"
#define MF_DESFIRE_FFF_APP_PREFIX  "Application"

// Successful operation
#define MF_DESFIRE_STATUS_OPERATION_OK          (0x00)
// No changes done to backup files, CommitTransaction / AbortTransaction not necessary
#define MF_DESFIRE_STATUS_NO_CHANGES            (0x0C)
// Insufficient NV-Memory to complete command
#define MF_DESFIRE_STATUS_OUT_OF_EEPROM_ERROR   (0x0E)
// Command code not supported
#define MF_DESFIRE_STATUS_ILLEGAL_COMMAND_CODE  (0x1C)
// CRC or MAC does not match data Padding bytes not valid
#define MF_DESFIRE_STATUS_INTEGRITY_ERROR       (0x1E)
// Invalid key number specified
#define MF_DESFIRE_STATUS_NO_SUCH_KEY           (0x40)
// Length of command string invalid
#define MF_DESFIRE_STATUS_LENGTH_ERROR          (0x7E)
// Current configuration / status does not allow the requested command
#define MF_DESFIRE_STATUS_PERMISSION_DENIED     (0x9D)
// Value of the parameter(s) invalid
#define MF_DESFIRE_STATUS_PARAMETER_ERROR       (0x9E)
// Requested AID not present on PICC
#define MF_DESFIRE_STATUS_APPLICATION_NOT_FOUND (0xA0)
// Unrecoverable error within application, application will be disabled
#define MF_DESFIRE_STATUS_APPL_INTEGRITY_ERROR  (0xA1)
// Current authentication status does not allow the requested command
#define MF_DESFIRE_STATUS_AUTHENTICATION_ERROR  (0xAE)
// Additional data frame is expected to be sent
#define MF_DESFIRE_STATUS_ADDITIONAL_FRAME      (0xAF)
// Attempt to read/write data from/to beyond the file's/record's limits
// Attempt to exceed the limits of a value file.
#define MF_DESFIRE_STATUS_BOUNDARY_ERROR        (0xBE)
// Unrecoverable error within PICC, PICC will be disabled
#define MF_DESFIRE_STATUS_PICC_INTEGRITY_ERROR  (0xC1)
// Previous Command was not fully completed. Not all Frames were requested or provided by the PCD
#define MF_DESFIRE_STATUS_COMMAND_ABORTED       (0xCA)
// PICC was disabled by an unrecoverable error
#define MF_DESFIRE_STATUS_PICC_DISABLED_ERROR   (0xCD)
// Number of Applications limited to 28, no additional CreateApplication possible
#define MF_DESFIRE_STATUS_COUNT_ERROR           (0xCE)
// Creation of file/application failed because file/application with same number already exists
#define MF_DESFIRE_STATUS_DUBLICATE_ERROR       (0xDE)
// Could not complete NV-write operation due to loss of power, internal backup/rollback mechanism activated
#define MF_DESFIRE_STATUS_EEPROM_ERROR          (0xEE)
// Specified file number does not exist
#define MF_DESFIRE_STATUS_FILE_NOT_FOUND        (0xF0)
// Unrecoverable error within file, file will be disabled
#define MF_DESFIRE_STATUS_FILE_INTEGRITY_ERROR  (0xF1)

// SimpleArray configurations

extern const SimpleArrayConfig mf_desfire_key_version_array_config;
extern const SimpleArrayConfig mf_desfire_app_id_array_config;
extern const SimpleArrayConfig mf_desfire_file_id_array_config;
extern const SimpleArrayConfig mf_desfire_file_settings_array_config;
extern const SimpleArrayConfig mf_desfire_file_data_array_config;
extern const SimpleArrayConfig mf_desfire_application_array_config;

// Parse internal MfDesfire structures

bool mf_desfire_version_parse(MfDesfireVersion* data, const BitBuffer* buf);

bool mf_desfire_free_memory_parse(MfDesfireFreeMemory* data, const BitBuffer* buf);

bool mf_desfire_key_settings_parse(MfDesfireKeySettings* data, const BitBuffer* buf);

bool mf_desfire_key_version_parse(MfDesfireKeyVersion* data, const BitBuffer* buf);

bool mf_desfire_application_id_parse(
    MfDesfireApplicationId* data,
    uint32_t index,
    const BitBuffer* buf);

bool mf_desfire_file_id_parse(MfDesfireFileId* data, uint32_t index, const BitBuffer* buf);

bool mf_desfire_file_settings_parse(MfDesfireFileSettings* data, const BitBuffer* buf);

bool mf_desfire_file_data_parse(MfDesfireFileData* data, const BitBuffer* buf);

// Init internal MfDesfire structures

void mf_desfire_file_data_init(MfDesfireFileData* data);

void mf_desfire_application_init(MfDesfireApplication* data);

// Reset internal MfDesfire structures

void mf_desfire_file_data_reset(MfDesfireFileData* data);

void mf_desfire_application_reset(MfDesfireApplication* data);

// Copy internal MfDesfire structures

void mf_desfire_file_data_copy(MfDesfireFileData* data, const MfDesfireFileData* other);

void mf_desfire_application_copy(MfDesfireApplication* data, const MfDesfireApplication* other);

// Load internal MfDesfire structures

bool mf_desfire_version_load(MfDesfireVersion* data, FlipperFormat* ff);

bool mf_desfire_free_memory_load(MfDesfireFreeMemory* data, FlipperFormat* ff);

bool mf_desfire_key_settings_load(
    MfDesfireKeySettings* data,
    const char* prefix,
    FlipperFormat* ff);

bool mf_desfire_key_version_load(
    MfDesfireKeyVersion* data,
    const char* prefix,
    uint32_t index,
    FlipperFormat* ff);

bool mf_desfire_file_count_load(uint32_t* data, const char* prefix, FlipperFormat* ff);

bool mf_desfire_file_ids_load(
    MfDesfireFileId* data,
    uint32_t count,
    const char* prefix,
    FlipperFormat* ff);

bool mf_desfire_file_settings_load(
    MfDesfireFileSettings* data,
    const char* prefix,
    FlipperFormat* ff);

bool mf_desfire_file_data_load(MfDesfireFileData* data, const char* prefix, FlipperFormat* ff);

bool mf_desfire_application_count_load(uint32_t* data, FlipperFormat* ff);

bool mf_desfire_application_ids_load(
    MfDesfireApplicationId* data,
    uint32_t count,
    FlipperFormat* ff);

bool mf_desfire_application_load(MfDesfireApplication* data, const char* prefix, FlipperFormat* ff);

// Save internal MFDesfire structures

bool mf_desfire_version_save(const MfDesfireVersion* data, FlipperFormat* ff);

bool mf_desfire_free_memory_save(const MfDesfireFreeMemory* data, FlipperFormat* ff);

bool mf_desfire_key_settings_save(
    const MfDesfireKeySettings* data,
    const char* prefix,
    FlipperFormat* ff);

bool mf_desfire_key_version_save(
    const MfDesfireKeyVersion* data,
    const char* prefix,
    uint32_t index,
    FlipperFormat* ff);

bool mf_desfire_file_ids_save(
    const MfDesfireFileId* data,
    uint32_t count,
    const char* prefix,
    FlipperFormat* ff);

bool mf_desfire_file_settings_save(
    const MfDesfireFileSettings* data,
    const char* prefix,
    FlipperFormat* ff);

bool mf_desfire_file_data_save(
    const MfDesfireFileData* data,
    const char* prefix,
    FlipperFormat* ff);

bool mf_desfire_application_count_save(const uint32_t* data, FlipperFormat* ff);

bool mf_desfire_application_ids_save(
    const MfDesfireApplicationId* data,
    uint32_t count,
    FlipperFormat* ff);

bool mf_desfire_application_save(
    const MfDesfireApplication* data,
    const char* prefix,
    FlipperFormat* ff);
