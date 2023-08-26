#include "emv.h"

#include <core/common_defines.h>

#define TAG "Emv"

const PDOLValue pdol_term_info = {0x9F59, {0xC8, 0x80, 0x00}}; // Terminal transaction information
const PDOLValue pdol_term_type = {0x9F5A, {0x00}}; // Terminal transaction type
const PDOLValue pdol_merchant_type = {0x9F58, {0x01}}; // Merchant type indicator
const PDOLValue pdol_term_trans_qualifies = {
    0x9F66,
    {0x79, 0x00, 0x40, 0x80}}; // Terminal transaction qualifiers
const PDOLValue pdol_addtnl_term_qualifies = {
    0x9F40,
    {0x79, 0x00, 0x40, 0x80}}; // Terminal transaction qualifiers
const PDOLValue pdol_amount_authorise = {
    0x9F02,
    {0x00, 0x00, 0x00, 0x10, 0x00, 0x00}}; // Amount, authorised
const PDOLValue pdol_amount = {0x9F03, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; // Amount
const PDOLValue pdol_country_code = {0x9F1A, {0x01, 0x24}}; // Terminal country code
const PDOLValue pdol_currency_code = {0x5F2A, {0x01, 0x24}}; // Transaction currency code
const PDOLValue pdol_term_verification = {
    0x95,
    {0x00, 0x00, 0x00, 0x00, 0x00}}; // Terminal verification results
const PDOLValue pdol_transaction_date = {0x9A, {0x19, 0x01, 0x01}}; // Transaction date
const PDOLValue pdol_transaction_type = {0x9C, {0x00}}; // Transaction type
const PDOLValue pdol_transaction_cert = {0x98, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}; // Transaction cert
const PDOLValue pdol_unpredict_number = {0x9F37, {0x82, 0x3D, 0xDE, 0x7A}}; // Unpredictable number

const PDOLValue* const pdol_values[] = {
    &pdol_term_info,
    &pdol_term_type,
    &pdol_merchant_type,
    &pdol_term_trans_qualifies,
    &pdol_addtnl_term_qualifies,
    &pdol_amount_authorise,
    &pdol_amount,
    &pdol_country_code,
    &pdol_currency_code,
    &pdol_term_verification,
    &pdol_transaction_date,
    &pdol_transaction_type,
    &pdol_transaction_cert,
    &pdol_unpredict_number,
};

static const uint8_t select_ppse_ans[] = {0x6F, 0x29, 0x84, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E,
                                          0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31,
                                          0xA5, 0x17, 0xBF, 0x0C, 0x14, 0x61, 0x12, 0x4F, 0x07,
                                          0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10, 0x50, 0x04,
                                          0x56, 0x49, 0x53, 0x41, 0x87, 0x01, 0x01, 0x90, 0x00};
static const uint8_t select_app_ans[] = {0x6F, 0x20, 0x84, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x03,
                                         0x10, 0x10, 0xA5, 0x15, 0x50, 0x04, 0x56, 0x49, 0x53,
                                         0x41, 0x9F, 0x38, 0x0C, 0x9F, 0x66, 0x04, 0x9F, 0x02,
                                         0x06, 0x9F, 0x37, 0x04, 0x5F, 0x2A, 0x02, 0x90, 0x00};
static const uint8_t pdol_ans[] = {0x77, 0x40, 0x82, 0x02, 0x20, 0x00, 0x57, 0x13, 0x55, 0x70,
                                   0x73, 0x83, 0x85, 0x87, 0x73, 0x31, 0xD1, 0x80, 0x22, 0x01,
                                   0x38, 0x84, 0x77, 0x94, 0x00, 0x00, 0x1F, 0x5F, 0x34, 0x01,
                                   0x00, 0x9F, 0x10, 0x07, 0x06, 0x01, 0x11, 0x03, 0x80, 0x00,
                                   0x00, 0x9F, 0x26, 0x08, 0x7A, 0x65, 0x7F, 0xD3, 0x52, 0x96,
                                   0xC9, 0x85, 0x9F, 0x27, 0x01, 0x00, 0x9F, 0x36, 0x02, 0x06,
                                   0x0C, 0x9F, 0x6C, 0x02, 0x10, 0x00, 0x90, 0x00};

static void emv_trace(FuriHalNfcTxRxContext* tx_rx, const char* message) {
    if(furi_log_get_level() == FuriLogLevelTrace) {
        FURI_LOG_T(TAG, "%s", message);
        printf("TX: ");
        for(size_t i = 0; i < tx_rx->tx_bits / 8; i++) {
            printf("%02X ", tx_rx->tx_data[i]);
        }
        printf("\r\nRX: ");
        for(size_t i = 0; i < tx_rx->rx_bits / 8; i++) {
            printf("%02X ", tx_rx->rx_data[i]);
        }
        printf("\r\n");
    }
}

static bool emv_decode_response(uint8_t* buff, uint16_t len, EmvApplication* app) {
    uint16_t i = 0;
    uint16_t tag = 0, first_byte = 0;
    uint16_t tlen = 0;
    bool success = false;

    while(i < len) {
        first_byte = buff[i];
        if((first_byte & 31) == 31) { // 2-byte tag
            tag = buff[i] << 8 | buff[i + 1];
            i++;
            FURI_LOG_T(TAG, " 2-byte TLV EMV tag: %x", tag);
        } else {
            tag = buff[i];
            FURI_LOG_T(TAG, " 1-byte TLV EMV tag: %x", tag);
        }
        i++;
        tlen = buff[i];
        if((tlen & 128) == 128) { // long length value
            i++;
            tlen = buff[i];
            FURI_LOG_T(TAG, " 2-byte TLV length: %d", tlen);
        } else {
            FURI_LOG_T(TAG, " 1-byte TLV length: %d", tlen);
        }
        i++;
        if((first_byte & 32) == 32) { // "Constructed" -- contains more TLV data to parse
            FURI_LOG_T(TAG, "Constructed TLV %x", tag);
            if(!emv_decode_response(&buff[i], tlen, app)) {
                FURI_LOG_T(TAG, "Failed to decode response for %x", tag);
                // return false;
            } else {
                success = true;
            }
        } else {
            switch(tag) {
            case EMV_TAG_AID:
                app->aid_len = tlen;
                memcpy(app->aid, &buff[i], tlen);
                success = true;
                FURI_LOG_T(TAG, "found EMV_TAG_AID %x", tag);
                break;
            case EMV_TAG_PRIORITY:
                memcpy(&app->priority, &buff[i], tlen);
                success = true;
                break;
            case EMV_TAG_CARD_NAME:
                memcpy(app->name, &buff[i], tlen);
                app->name[tlen] = '\0';
                app->name_found = true;
                success = true;
                FURI_LOG_T(TAG, "found EMV_TAG_CARD_NAME %x : %s", tag, app->name);
                break;
            case EMV_TAG_PDOL:
                memcpy(app->pdol.data, &buff[i], tlen);
                app->pdol.size = tlen;
                success = true;
                FURI_LOG_T(TAG, "found EMV_TAG_PDOL %x (len=%d)", tag, tlen);
                break;
            case EMV_TAG_AFL:
                memcpy(app->afl.data, &buff[i], tlen);
                app->afl.size = tlen;
                success = true;
                FURI_LOG_T(TAG, "found EMV_TAG_AFL %x (len=%d)", tag, tlen);
                break;
            case EMV_TAG_TRACK_1_EQUIV: {
                char track_1_equiv[80];
                memcpy(track_1_equiv, &buff[i], tlen);
                track_1_equiv[tlen] = '\0';
                success = true;
                FURI_LOG_T(TAG, "found EMV_TAG_TRACK_1_EQUIV %x : %s", tag, track_1_equiv);
                break;
            }
            case EMV_TAG_TRACK_2_EQUIV: {
                // 0xD0 delimits PAN from expiry (YYMM)
                for(int x = 1; x < tlen; x++) {
                    if(buff[i + x + 1] > 0xD0) {
                        memcpy(app->card_number, &buff[i], x + 1);
                        app->card_number_len = x + 1;
                        app->exp_year = (buff[i + x + 1] << 4) | (buff[i + x + 2] >> 4);
                        app->exp_month = (buff[i + x + 2] << 4) | (buff[i + x + 3] >> 4);
                        break;
                    }
                }

                // Convert 4-bit to ASCII representation
                char track_2_equiv[41];
                uint8_t track_2_equiv_len = 0;
                for(int x = 0; x < tlen; x++) {
                    char top = (buff[i + x] >> 4) + '0';
                    char bottom = (buff[i + x] & 0x0F) + '0';
                    track_2_equiv[x * 2] = top;
                    track_2_equiv_len++;
                    if(top == '?') break;
                    track_2_equiv[x * 2 + 1] = bottom;
                    track_2_equiv_len++;
                    if(bottom == '?') break;
                }
                track_2_equiv[track_2_equiv_len] = '\0';
                success = true;
                FURI_LOG_T(TAG, "found EMV_TAG_TRACK_2_EQUIV %x : %s", tag, track_2_equiv);
                break;
            }
            case EMV_TAG_PAN:
                memcpy(app->card_number, &buff[i], tlen);
                app->card_number_len = tlen;
                success = true;
                break;
            case EMV_TAG_EXP_DATE:
                app->exp_year = buff[i];
                app->exp_month = buff[i + 1];
                success = true;
                break;
            case EMV_TAG_CURRENCY_CODE:
                app->currency_code = (buff[i] << 8 | buff[i + 1]);
                success = true;
                break;
            case EMV_TAG_COUNTRY_CODE:
                app->country_code = (buff[i] << 8 | buff[i + 1]);
                success = true;
                break;
            }
        }
        i += tlen;
    }
    return success;
}

static bool emv_select_ppse(FuriHalNfcTxRxContext* tx_rx, EmvApplication* app) {
    bool app_aid_found = false;
    const uint8_t emv_select_ppse_cmd[] = {
        0x00, 0xA4, // SELECT ppse
        0x04, 0x00, // P1:By name, P2: empty
        0x0e, // Lc: Data length
        0x32, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, // Data string:
        0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, // 2PAY.SYS.DDF01 (PPSE)
        0x00 // Le
    };

    memcpy(tx_rx->tx_data, emv_select_ppse_cmd, sizeof(emv_select_ppse_cmd));
    tx_rx->tx_bits = sizeof(emv_select_ppse_cmd) * 8;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;

    FURI_LOG_D(TAG, "Send select PPSE");
    if(furi_hal_nfc_tx_rx(tx_rx, 300)) {
        emv_trace(tx_rx, "Select PPSE answer:");
        if(emv_decode_response(tx_rx->rx_data, tx_rx->rx_bits / 8, app)) {
            app_aid_found = true;
        } else {
            FURI_LOG_E(TAG, "Failed to parse application");
        }
    } else {
        FURI_LOG_E(TAG, "Failed select PPSE");
    }

    return app_aid_found;
}

static bool emv_select_app(FuriHalNfcTxRxContext* tx_rx, EmvApplication* app) {
    app->app_started = false;
    const uint8_t emv_select_header[] = {
        0x00,
        0xA4, // SELECT application
        0x04,
        0x00 // P1:By name, P2:First or only occurence
    };
    uint16_t size = sizeof(emv_select_header);

    // Copy header
    memcpy(tx_rx->tx_data, emv_select_header, size);
    // Copy AID
    tx_rx->tx_data[size++] = app->aid_len;
    memcpy(&tx_rx->tx_data[size], app->aid, app->aid_len);
    size += app->aid_len;
    tx_rx->tx_data[size++] = 0x00;
    tx_rx->tx_bits = size * 8;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;

    FURI_LOG_D(TAG, "Start application");
    if(furi_hal_nfc_tx_rx(tx_rx, 300)) {
        emv_trace(tx_rx, "Start application answer:");
        if(emv_decode_response(tx_rx->rx_data, tx_rx->rx_bits / 8, app)) {
            app->app_started = true;
        } else {
            FURI_LOG_E(TAG, "Failed to read PAN or PDOL");
        }
    } else {
        FURI_LOG_E(TAG, "Failed to start application");
    }

    return app->app_started;
}

static uint16_t emv_prepare_pdol(APDU* dest, APDU* src) {
    bool tag_found;
    for(uint16_t i = 0; i < src->size; i++) {
        tag_found = false;
        for(uint8_t j = 0; j < sizeof(pdol_values) / sizeof(PDOLValue*); j++) {
            if(src->data[i] == pdol_values[j]->tag) {
                // Found tag with 1 byte length
                uint8_t len = src->data[++i];
                memcpy(dest->data + dest->size, pdol_values[j]->data, len);
                dest->size += len;
                tag_found = true;
                break;
            } else if(((src->data[i] << 8) | src->data[i + 1]) == pdol_values[j]->tag) {
                // Found tag with 2 byte length
                i += 2;
                uint8_t len = src->data[i];
                memcpy(dest->data + dest->size, pdol_values[j]->data, len);
                dest->size += len;
                tag_found = true;
                break;
            }
        }
        if(!tag_found) {
            // Unknown tag, fill zeros
            i += 2;
            uint8_t len = src->data[i];
            memset(dest->data + dest->size, 0, len);
            dest->size += len;
        }
    }
    return dest->size;
}

static bool emv_get_processing_options(FuriHalNfcTxRxContext* tx_rx, EmvApplication* app) {
    bool card_num_read = false;
    const uint8_t emv_gpo_header[] = {0x80, 0xA8, 0x00, 0x00};
    uint16_t size = sizeof(emv_gpo_header);

    // Copy header
    memcpy(tx_rx->tx_data, emv_gpo_header, size);
    APDU pdol_data = {0, {0}};
    // Prepare and copy pdol parameters
    emv_prepare_pdol(&pdol_data, &app->pdol);
    tx_rx->tx_data[size++] = 0x02 + pdol_data.size;
    tx_rx->tx_data[size++] = 0x83;
    tx_rx->tx_data[size++] = pdol_data.size;
    memcpy(tx_rx->tx_data + size, pdol_data.data, pdol_data.size);
    size += pdol_data.size;
    tx_rx->tx_data[size++] = 0;
    tx_rx->tx_bits = size * 8;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;

    FURI_LOG_D(TAG, "Get proccessing options");
    if(furi_hal_nfc_tx_rx(tx_rx, 300)) {
        emv_trace(tx_rx, "Get processing options answer:");
        if(emv_decode_response(tx_rx->rx_data, tx_rx->rx_bits / 8, app)) {
            if(app->card_number_len > 0) {
                card_num_read = true;
            }
        }
    } else {
        FURI_LOG_E(TAG, "Failed to get processing options");
    }

    return card_num_read;
}

static bool emv_read_sfi_record(
    FuriHalNfcTxRxContext* tx_rx,
    EmvApplication* app,
    uint8_t sfi,
    uint8_t record_num) {
    bool card_num_read = false;
    uint8_t sfi_param = (sfi << 3) | (1 << 2);
    uint8_t emv_sfi_header[] = {
        0x00,
        0xB2, // READ RECORD
        record_num, // P1:record_number
        sfi_param, // P2:SFI
        0x00 // Le
    };

    memcpy(tx_rx->tx_data, emv_sfi_header, sizeof(emv_sfi_header));
    tx_rx->tx_bits = sizeof(emv_sfi_header) * 8;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;

    if(furi_hal_nfc_tx_rx(tx_rx, 300)) {
        emv_trace(tx_rx, "SFI record:");
        if(emv_decode_response(tx_rx->rx_data, tx_rx->rx_bits / 8, app)) {
            card_num_read = true;
        }
    } else {
        FURI_LOG_E(TAG, "Failed to read SFI record %d", record_num);
    }

    return card_num_read;
}

static bool emv_read_files(FuriHalNfcTxRxContext* tx_rx, EmvApplication* app) {
    bool card_num_read = false;

    if(app->afl.size == 0) {
        return false;
    }

    FURI_LOG_D(TAG, "Search PAN in SFI");
    // Iterate through all files
    for(size_t i = 0; i < app->afl.size; i += 4) {
        uint8_t sfi = app->afl.data[i] >> 3;
        uint8_t record_start = app->afl.data[i + 1];
        uint8_t record_end = app->afl.data[i + 2];
        // Iterate through all records in file
        for(uint8_t record = record_start; record <= record_end; ++record) {
            card_num_read |= emv_read_sfi_record(tx_rx, app, sfi, record);
        }
    }

    return card_num_read;
}

bool emv_read_bank_card(FuriHalNfcTxRxContext* tx_rx, EmvApplication* emv_app) {
    furi_assert(tx_rx);
    furi_assert(emv_app);
    bool card_num_read = false;
    memset(emv_app, 0, sizeof(EmvApplication));

    do {
        if(!emv_select_ppse(tx_rx, emv_app)) break;
        if(!emv_select_app(tx_rx, emv_app)) break;
        if(emv_get_processing_options(tx_rx, emv_app)) {
            card_num_read = true;
        } else {
            card_num_read = emv_read_files(tx_rx, emv_app);
        }
    } while(false);

    return card_num_read;
}

bool emv_card_emulation(FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(tx_rx);
    bool emulation_complete = false;
    tx_rx->tx_bits = 0;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;

    do {
        FURI_LOG_D(TAG, "Read select PPSE command");
        if(!furi_hal_nfc_tx_rx(tx_rx, 300)) break;

        memcpy(tx_rx->tx_data, select_ppse_ans, sizeof(select_ppse_ans));
        tx_rx->tx_bits = sizeof(select_ppse_ans) * 8;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        FURI_LOG_D(TAG, "Send select PPSE answer and read select App command");
        if(!furi_hal_nfc_tx_rx(tx_rx, 300)) break;

        memcpy(tx_rx->tx_data, select_app_ans, sizeof(select_app_ans));
        tx_rx->tx_bits = sizeof(select_app_ans) * 8;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        FURI_LOG_D(TAG, "Send select App answer and read get PDOL command");
        if(!furi_hal_nfc_tx_rx(tx_rx, 300)) break;

        memcpy(tx_rx->tx_data, pdol_ans, sizeof(pdol_ans));
        tx_rx->tx_bits = sizeof(pdol_ans) * 8;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        FURI_LOG_D(TAG, "Send get PDOL answer");
        if(!furi_hal_nfc_tx_rx(tx_rx, 300)) break;

        emulation_complete = true;
    } while(false);

    return emulation_complete;
}
