#pragma once

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_APDU_LEN 255

#define EMV_TAG_APP_TEMPLATE 0x61
#define EMV_TAG_AID 0x4F
#define EMV_TAG_PRIORITY 0x87
#define EMV_TAG_PDOL 0x9F38
#define EMV_TAG_CARD_NAME 0x50
#define EMV_TAG_FCI 0xBF0C
#define EMV_TAG_LOG_CTRL 0x9F4D
#define EMV_TAG_TRACK_1_EQUIV 0x56
#define EMV_TAG_TRACK_2_EQUIV 0x57
#define EMV_TAG_PAN 0x5A
#define EMV_TAG_AFL 0x94
#define EMV_TAG_EXP_DATE 0x5F24
#define EMV_TAG_COUNTRY_CODE 0x5F28
#define EMV_TAG_CURRENCY_CODE 0x9F42
#define EMV_TAG_CARDHOLDER_NAME 0x5F20

typedef struct {
    uint16_t tag;
    uint8_t data[];
} PDOLValue;

typedef struct {
    uint8_t size;
    uint8_t data[MAX_APDU_LEN];
} APDU;

typedef struct {
    uint8_t priority;
    uint8_t aid[16];
    uint8_t aid_len;
    bool app_started;
    char name[32];
    bool name_found;
    uint8_t pan[10];
    uint8_t pan_len;
    uint8_t exp_month;
    uint8_t exp_year;
    uint16_t country_code;
    uint16_t currency_code;
    APDU pdol;
    APDU afl;
} EmvApplication;

typedef enum {
    EmvErrorNone = 0,
    EmvErrorNotPresent,
    EmvErrorProtocol,
    EmvErrorTimeout,
} EmvError;

typedef struct {
    Iso14443_4aData* iso14443_4a_data;
    EmvApplication emv_application;
} EmvData;

extern const NfcDeviceBase nfc_device_emv;

// Virtual methods

EmvData* emv_alloc();

void emv_free(EmvData* data);

void emv_reset(EmvData* data);

void emv_copy(EmvData* data, const EmvData* other);

bool emv_verify(EmvData* data, const FuriString* device_type);

bool emv_load(EmvData* data, FlipperFormat* ff, uint32_t version);

bool emv_save(const EmvData* data, FlipperFormat* ff);

bool emv_is_equal(const EmvData* data, const EmvData* other);

const char* emv_get_device_name(const EmvData* data, NfcDeviceNameType name_type);

const uint8_t* emv_get_uid(const EmvData* data, size_t* uid_len);

bool emv_set_uid(EmvData* data, const uint8_t* uid, size_t uid_len);

Iso14443_4aData* emv_get_base_data(const EmvData* data);

// Getters and tests

//const EmvApplication* emv_get_application(const EmvData* data);

#ifdef __cplusplus
}
#endif
