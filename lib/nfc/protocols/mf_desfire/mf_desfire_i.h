#pragma once

#include "mf_desfire.h"

#define MF_DESFIRE_FFF_PICC_PREFIX "PICC"
#define MF_DESFIRE_FFF_APP_PREFIX "Application"

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
