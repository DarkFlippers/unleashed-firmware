#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>

#include <furi_hal_nfc.h>
#include <lib/nfc/helpers/mf_classic_dict.h>
#include <lib/nfc/protocols/emv.h>
#include <lib/nfc/protocols/mifare_ultralight.h>
#include <lib/nfc/protocols/mifare_classic.h>
#include <lib/nfc/protocols/mifare_desfire.h>

#define NFC_DEV_NAME_MAX_LEN 22
#define NFC_READER_DATA_MAX_SIZE 64
#define NFC_DICT_KEY_BATCH_SIZE 50

#define NFC_APP_FOLDER ANY_PATH("nfc")
#define NFC_APP_EXTENSION ".nfc"
#define NFC_APP_SHADOW_EXTENSION ".shd"

typedef void (*NfcLoadingCallback)(void* context, bool state);

typedef enum {
    NfcDeviceProtocolUnknown,
    NfcDeviceProtocolEMV,
    NfcDeviceProtocolMifareUl,
    NfcDeviceProtocolMifareClassic,
    NfcDeviceProtocolMifareDesfire,
} NfcProtocol;

typedef enum {
    NfcDeviceSaveFormatUid,
    NfcDeviceSaveFormatBankCard,
    NfcDeviceSaveFormatMifareUl,
    NfcDeviceSaveFormatMifareClassic,
    NfcDeviceSaveFormatMifareDesfire,
} NfcDeviceSaveFormat;

typedef struct {
    uint8_t data[NFC_READER_DATA_MAX_SIZE];
    uint16_t size;
} NfcReaderRequestData;

typedef struct {
    MfClassicDict* dict;
} NfcMfClassicDictAttackData;

typedef struct {
    FuriHalNfcDevData nfc_data;
    NfcProtocol protocol;
    union {
        NfcReaderRequestData reader_data;
        NfcMfClassicDictAttackData mf_classic_dict_attack_data;
    };
    union {
        EmvData emv_data;
        MfUltralightData mf_ul_data;
        MfClassicData mf_classic_data;
        MifareDesfireData mf_df_data;
    };
    string_t parsed_data;
} NfcDeviceData;

typedef struct {
    Storage* storage;
    DialogsApp* dialogs;
    NfcDeviceData dev_data;
    char dev_name[NFC_DEV_NAME_MAX_LEN + 1];
    string_t load_path;
    NfcDeviceSaveFormat format;
    bool shadow_file_exist;

    NfcLoadingCallback loading_cb;
    void* loading_cb_ctx;
} NfcDevice;

NfcDevice* nfc_device_alloc();

void nfc_device_free(NfcDevice* nfc_dev);

void nfc_device_set_name(NfcDevice* dev, const char* name);

bool nfc_device_save(NfcDevice* dev, const char* dev_name);

bool nfc_device_save_shadow(NfcDevice* dev, const char* dev_name);

bool nfc_device_load(NfcDevice* dev, const char* file_path, bool show_dialog);

bool nfc_device_load_key_cache(NfcDevice* dev);

bool nfc_file_select(NfcDevice* dev);

void nfc_device_data_clear(NfcDeviceData* dev);

void nfc_device_clear(NfcDevice* dev);

bool nfc_device_delete(NfcDevice* dev, bool use_load_path);

bool nfc_device_restore(NfcDevice* dev, bool use_load_path);

void nfc_device_set_loading_callback(NfcDevice* dev, NfcLoadingCallback callback, void* context);
