#pragma once

#include <furi_hal_nfc.h>
#include <helpers/mrtd_helpers.h>

#define MRTD_APP_FOLDER "/mrtd"
#define MRTD_APP_EXTENSION ".mrtd"

typedef struct {
    FuriHalNfcTxRxContext* tx_rx;
    MrtdData* mrtd_data;
    uint16_t file_offset;
    uint8_t ksenc[16];
    uint8_t ksmac[16];
    uint64_t ssc_long; // TODO: rename without _long

    bool secure_messaging;
} MrtdApplication;

//TODO: description
MrtdApplication* mrtd_alloc_init(FuriHalNfcTxRxContext* tx_rx, MrtdData* mrtd_data);
bool mrtd_select_app(MrtdApplication* app, AIDValue aid);
bool mrtd_authenticate(MrtdApplication* app);
bool mrtd_read_parse_file(MrtdApplication* app, EFFile file);

bool mrtd_auth_params_save(
    Storage* storage,
    DialogsApp* dialogs,
    MrtdAuthData* auth_data,
    const char* file_name);
bool mrtd_auth_params_save_file(
    Storage* storage,
    DialogsApp* dialogs,
    MrtdAuthData* auth_data,
    const char* file_name,
    const char* folder,
    const char* extension);

bool mrtd_auth_params_load(
    Storage* storage,
    DialogsApp* dialogs,
    MrtdAuthData* auth_data,
    const char* file_path,
    bool show_dialog);
