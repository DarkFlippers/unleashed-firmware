#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>

#include "mifare_ultralight.h"

#define NFC_DEV_NAME_MAX_LEN 22
#define NFC_FILE_NAME_MAX_LEN 120

typedef enum {
    NfcDeviceNfca,
    NfcDeviceNfcb,
    NfcDeviceNfcf,
    NfcDeviceNfcv,
} NfcDeviceType;

typedef enum {
    NfcDeviceProtocolUnknown,
    NfcDeviceProtocolEMV,
    NfcDeviceProtocolMifareUl,
} NfcProtocol;

typedef enum {
    NfcDeviceSaveFormatUid,
    NfcDeviceSaveFormatBankCard,
    NfcDeviceSaveFormatMifareUl,
} NfcDeviceSaveFormat;

typedef struct {
    uint8_t uid_len;
    uint8_t uid[10];
    uint8_t atqa[2];
    uint8_t sak;
    NfcDeviceType device;
    NfcProtocol protocol;
} NfcDeviceCommonData;

typedef struct {
    char name[32];
    uint8_t aid[16];
    uint16_t aid_len;
    uint8_t number[10];
    uint8_t number_len;
    uint8_t exp_mon;
    uint8_t exp_year;
    uint16_t country_code;
    uint16_t currency_code;
} NfcEmvData;

typedef struct {
    NfcDeviceCommonData nfc_data;
    union {
        NfcEmvData emv_data;
        MifareUlData mf_ul_data;
    };
} NfcDeviceData;

typedef struct {
    Storage* storage;
    DialogsApp* dialogs;
    NfcDeviceData dev_data;
    char dev_name[NFC_DEV_NAME_MAX_LEN + 1];
    char file_name[NFC_FILE_NAME_MAX_LEN];
    NfcDeviceSaveFormat format;
    bool shadow_file_exist;
} NfcDevice;

NfcDevice* nfc_device_alloc();

void nfc_device_free(NfcDevice* nfc_dev);

void nfc_device_set_name(NfcDevice* dev, const char* name);

bool nfc_device_save(NfcDevice* dev, const char* dev_name);

bool nfc_device_save_shadow(NfcDevice* dev, const char* dev_name);

bool nfc_device_load(NfcDevice* dev, const char* file_path);

bool nfc_file_select(NfcDevice* dev);

void nfc_device_clear(NfcDevice* dev);

bool nfc_device_delete(NfcDevice* dev);

bool nfc_device_restore(NfcDevice* dev);
