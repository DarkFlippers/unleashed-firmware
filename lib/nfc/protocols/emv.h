#pragma once

#include <furi_hal_nfc.h>

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
    char name[32];
    uint8_t aid[16];
    uint16_t aid_len;
    uint8_t number[10];
    uint8_t number_len;
    uint8_t exp_mon;
    uint8_t exp_year;
    uint16_t country_code;
    uint16_t currency_code;
} EmvData;

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
    uint8_t card_number[10];
    uint8_t card_number_len;
    uint8_t exp_month;
    uint8_t exp_year;
    uint16_t country_code;
    uint16_t currency_code;
    APDU pdol;
    APDU afl;
} EmvApplication;

/** Read bank card data
 * @note Search EMV Application, start it, try to read AID, PAN, card name,
 * expiration date, currency and country codes
 *
 * @param tx_rx     FuriHalNfcTxRxContext instance
 * @param emv_app   EmvApplication instance
 * 
 * @return true on success
 */
bool emv_read_bank_card(FuriHalNfcTxRxContext* tx_rx, EmvApplication* emv_app);

/** Emulate bank card
 * @note Answer to application selection and PDOL
 *
 * @param tx_rx     FuriHalNfcTxRxContext instance
 *
 * @return true on success
 */
bool emv_card_emulation(FuriHalNfcTxRxContext* tx_rx);
