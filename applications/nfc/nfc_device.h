#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NFC_DEV_NAME_MAX_LEN 22
#define NFC_FILE_NAME_MAX_LEN 120

#define NFC_MIFARE_UL_MAX_SIZE 256

typedef enum {
    NfcDeviceNfca,
    NfcDeviceNfcb,
    NfcDeviceNfcf,
    NfcDeviceNfcv,
} NfcDeviceType;

typedef enum {
    NfcDeviceProtocolUnknown,
    NfcDeviceProtocolEMV,
    NfcDeviceProtocolMfUltralight,
} NfcProtocol;

typedef struct {
    uint8_t uid_len;
    uint8_t uid[10];
    uint8_t atqa[2];
    uint8_t sak;
    NfcDeviceType device;
    NfcProtocol protocol;
} NfcDeviceData;

typedef struct {
    NfcDeviceData nfc_data;
    char name[32];
    uint8_t number[8];
} NfcEmvData;

typedef struct {
    NfcDeviceData nfc_data;
    uint8_t full_dump[NFC_MIFARE_UL_MAX_SIZE];
    uint16_t dump_size;
    // TODO delete with debug view
    uint8_t man_block[12];
    uint8_t otp[4];
} NfcMifareUlData;

typedef struct {
    NfcDeviceData data;
    char dev_name[NFC_DEV_NAME_MAX_LEN];
    char file_name[NFC_FILE_NAME_MAX_LEN];
} NfcDevice;

void nfc_device_set_name(NfcDevice* dev, const char* name);

bool nfc_device_save(NfcDevice* dev, const char* dev_name);

bool nfc_device_load(NfcDevice* dev, const char* dev_name);

bool nfc_file_select(NfcDevice* dev);
