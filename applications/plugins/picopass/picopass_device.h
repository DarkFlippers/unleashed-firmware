#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <mbedtls/des.h>

#include "rfal_picopass.h"
#include "loclass/optimized_ikeys.h"
#include "loclass/optimized_cipher.h"

#define PICOPASS_DEV_NAME_MAX_LEN 22
#define PICOPASS_READER_DATA_MAX_SIZE 64
#define PICOPASS_BLOCK_LEN 8
#define PICOPASS_MAX_APP_LIMIT 32

#define PICOPASS_CSN_BLOCK_INDEX 0
#define PICOPASS_CONFIG_BLOCK_INDEX 1
#define PICOPASS_AIA_BLOCK_INDEX 5

#define PICOPASS_APP_FOLDER ANY_PATH("picopass")
#define PICOPASS_APP_EXTENSION ".picopass"
#define PICOPASS_APP_SHADOW_EXTENSION ".pas"

typedef void (*PicopassLoadingCallback)(void* context, bool state);

typedef enum {
    PicopassDeviceEncryptionUnknown = 0,
    PicopassDeviceEncryptionNone = 0x14,
    PicopassDeviceEncryptionDES = 0x15,
    PicopassDeviceEncryption3DES = 0x17,
} PicopassEncryption;

typedef enum {
    PicopassDeviceSaveFormatHF,
    PicopassDeviceSaveFormatLF,
} PicopassDeviceSaveFormat;

typedef struct {
    bool valid;
    uint8_t bitLength;
    uint8_t FacilityCode;
    uint16_t CardNumber;
} PicopassWiegandRecord;

typedef struct {
    bool legacy;
    bool se_enabled;
    bool sio;
    bool biometrics;
    uint8_t pin_length;
    PicopassEncryption encryption;
    uint8_t credential[8];
    uint8_t pin0[8];
    uint8_t pin1[8];
    PicopassWiegandRecord record;
} PicopassPacs;

typedef struct {
    uint8_t data[PICOPASS_BLOCK_LEN];
} PicopassBlock;

typedef struct {
    PicopassBlock AA1[PICOPASS_MAX_APP_LIMIT];
    PicopassPacs pacs;
} PicopassDeviceData;

typedef struct {
    Storage* storage;
    DialogsApp* dialogs;
    PicopassDeviceData dev_data;
    char dev_name[PICOPASS_DEV_NAME_MAX_LEN + 1];
    string_t load_path;
    PicopassDeviceSaveFormat format;
    PicopassLoadingCallback loading_cb;
    void* loading_cb_ctx;

} PicopassDevice;

PicopassDevice* picopass_device_alloc();

void picopass_device_free(PicopassDevice* picopass_dev);

void picopass_device_set_name(PicopassDevice* dev, const char* name);

bool picopass_device_save(PicopassDevice* dev, const char* dev_name);

bool picopass_file_select(PicopassDevice* dev);

void picopass_device_data_clear(PicopassDeviceData* dev_data);

void picopass_device_clear(PicopassDevice* dev);

bool picopass_device_delete(PicopassDevice* dev, bool use_load_path);

void picopass_device_set_loading_callback(
    PicopassDevice* dev,
    PicopassLoadingCallback callback,
    void* context);

ReturnCode picopass_device_parse_credential(PicopassBlock* AA1, PicopassPacs* pacs);
ReturnCode picopass_device_parse_wiegand(uint8_t* data, PicopassWiegandRecord* record);
