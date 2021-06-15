#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_APDU_LEN 255

#define EMV_TAG_APP_TEMPLATE 0x61
#define EMV_TAG_AID 0x4F
#define EMV_TAG_PRIORITY 0x87
#define EMV_TAG_PDOL 0x9F38
#define EMV_TAG_CARD_NAME 0x50
#define EMV_TAG_FCI 0xBF0C
#define EMV_TAG_LOG_CTRL 0x9F4D
#define EMV_TAG_CARD_NUM 0x57
#define EMV_TAG_PAN 0x5A
#define EMV_TAG_AFL 0x94

typedef struct {
    uint16_t tag;
    uint8_t data[];
} PDOLValue;

extern const PDOLValue* pdol_values[];

typedef struct {
    uint8_t size;
    uint8_t data[MAX_APDU_LEN];
} APDU;

typedef struct {
    uint8_t priority;
    uint8_t aid[16];
    uint8_t aid_len;
    char name[32];
    uint8_t card_number[8];
    APDU pdol;
    APDU afl;
} EmvApplication;

/* Terminal emulation */
uint16_t emv_prepare_select_ppse(uint8_t* dest);
bool emv_decode_ppse_response(uint8_t* buff, uint16_t len, EmvApplication* app);

uint16_t emv_prepare_select_app(uint8_t* dest, EmvApplication* app);
bool emv_decode_select_app_response(uint8_t* buff, uint16_t len, EmvApplication* app);

uint16_t emv_prepare_get_proc_opt(uint8_t* dest, EmvApplication* app);
bool emv_decode_get_proc_opt(uint8_t* buff, uint16_t len, EmvApplication* app);

uint16_t emv_prepare_read_sfi_record(uint8_t* dest, uint8_t sfi, uint8_t record_num);
bool emv_decode_read_sfi_record(uint8_t* buff, uint16_t len, EmvApplication* app);

/* Card emulation */
uint16_t emv_select_ppse_ans(uint8_t* buff);
uint16_t emv_select_app_ans(uint8_t* buff);
uint16_t emv_get_proc_opt_ans(uint8_t* buff);
