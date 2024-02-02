#pragma once

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_APDU_LEN 255

#define EMV_REQ_GET_DATA 0x80CA

#define EMV_TAG_AID 0x4F
#define EMV_TAG_PRIORITY 0x87
#define EMV_TAG_PDOL 0x9F38
#define EMV_TAG_APPL_PAYMENT_SYS 0x50
#define EMV_TAG_APPL_NAME 0x9F12
#define EMV_TAG_APPL_ISSUE 0x5F25
#define EMV_TAG_PIN_TRY_COUNTER 0x9F17
#define EMV_TAG_LOG_ENTRY 0x9F4D
#define EMV_TAG_LOG_FMT 0x9F4F

#define EMV_TAG_LAST_ONLINE_ATC 0x9F13
#define EMV_TAG_ATC 0x9F36
#define EMV_TAG_LOG_AMOUNT 0x9F02
#define EMV_TAG_LOG_COUNTRY 0x9F1A
#define EMV_TAG_LOG_CURRENCY 0x5F2A
#define EMV_TAG_LOG_DATE 0x9A
#define EMV_TAG_LOG_TIME 0x9F21

#define EMV_TAG_TRACK_1_EQUIV 0x56
#define EMV_TAG_TRACK_2_EQUIV 0x57
#define EMV_TAG_PAN 0x5A
#define EMV_TAG_AFL 0x94
#define EMV_TAG_EXP_DATE 0x5F24
#define EMV_TAG_COUNTRY_CODE 0x5F28
#define EMV_TAG_CURRENCY_CODE 0x9F42
#define EMV_TAG_CARDHOLDER_NAME 0x5F20
#define EMV_TAG_TRACK_2_DATA 0x9F6B
#define EMV_TAG_GPO_FMT1 0x80

#define EMV_TAG_RESP_BUF_SIZE 0x6C
#define EMV_TAG_RESP_BYTES_AVAILABLE 0x61

// Not used tags
#define EMV_TAG_FORM_FACTOR 0x9F6E
#define EMV_TAG_APP_TEMPLATE 0x61
#define EMV_TAG_FCI 0xBF0C
#define EMV_TAG_DEPOSIT_LOG_ENTRY 0xDF4D

typedef struct {
    uint16_t tag;
    uint8_t data[];
} PDOLValue;

typedef struct {
    uint8_t size;
    uint8_t data[MAX_APDU_LEN];
} APDU;

typedef struct {
    uint16_t atc;
    uint64_t amount;
    uint16_t country;
    uint16_t currency;
    uint32_t date;
    uint32_t time;
} Transaction;

typedef struct {
    uint8_t log_sfi;
    uint8_t log_records;
    uint8_t log_fmt[50];
    uint8_t log_fmt_len;
    uint8_t active_tr;
    bool saving_trans_list;
    Transaction trans[16];
    uint8_t priority;
    uint8_t aid[16];
    uint8_t aid_len;
    char name[16 + 1];
    char payment_sys[16 + 1];
    uint8_t pan[10]; // card_number
    uint8_t pan_len;
    uint8_t exp_day;
    uint8_t exp_month;
    uint8_t exp_year;
    uint8_t issue_day;
    uint8_t issue_month;
    uint8_t issue_year;
    uint16_t country_code;
    uint16_t currency_code;
    uint8_t pin_try_counter;
    uint16_t transaction_counter;
    uint16_t last_online_atc;
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
