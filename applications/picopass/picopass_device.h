#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>

#include <rfal_picopass.h>

typedef struct {
    bool valid;
    uint8_t bitLength;
    uint8_t FacilityCode;
    uint16_t CardNumber;
} PicopassWiegandRecord;

typedef struct {
    bool biometrics;
    uint8_t encryption;
    uint8_t credential[8];
    uint8_t pin0[8];
    uint8_t pin1[8];
    PicopassWiegandRecord record;
} PicopassPacs;

typedef struct {
    ApplicationArea AA1;
    PicopassPacs pacs;
} PicopassDeviceData;

typedef struct {
    Storage* storage;
    DialogsApp* dialogs;
    PicopassDeviceData dev_data;
} PicopassDevice;

PicopassDevice* picopass_device_alloc();

void picopass_device_free(PicopassDevice* picopass_dev);

void picopass_device_data_clear(PicopassDeviceData* dev_data);

void picopass_device_clear(PicopassDevice* dev);
