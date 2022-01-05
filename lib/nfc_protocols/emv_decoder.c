#include "emv_decoder.h"

const PDOLValue pdol_term_info = {0x9F59, {0xC8, 0x80, 0x00}}; // Terminal transaction information
const PDOLValue pdol_term_type = {0x9F5A, {0x00}}; // Terminal transaction type
const PDOLValue pdol_merchant_type = {0x9F58, {0x01}}; // Merchant type indicator
const PDOLValue pdol_term_trans_qualifies = {
    0x9F66,
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

const PDOLValue* pdol_values[] = {
    &pdol_term_info,
    &pdol_term_type,
    &pdol_merchant_type,
    &pdol_term_trans_qualifies,
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

static uint16_t emv_parse_TLV(uint8_t* dest, uint8_t* src, uint16_t* idx) {
    uint8_t len = src[*idx + 1];
    memcpy(dest, &src[*idx + 2], len);
    *idx = *idx + len + 1;
    return len;
}

static bool emv_decode_search_tag_u16_r(uint16_t tag, uint8_t* buff, uint16_t* idx) {
    if((buff[*idx] << 8 | buff[*idx + 1]) == tag) {
        *idx = *idx + 3;
        return true;
    }
    return false;
}

uint16_t emv_prepare_select_ppse(uint8_t* dest) {
    const uint8_t emv_select_ppse[] = {
        0x00, 0xA4, // SELECT ppse
        0x04, 0x00, // P1:By name, P2: empty
        0x0e, // Lc: Data length
        0x32, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, // Data string:
        0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, // 2PAY.SYS.DDF01 (PPSE)
        0x00 // Le
    };
    memcpy(dest, emv_select_ppse, sizeof(emv_select_ppse));
    return sizeof(emv_select_ppse);
}

bool emv_decode_ppse_response(uint8_t* buff, uint16_t len, EmvApplication* app) {
    uint16_t i = 0;
    bool app_aid_found = false;

    while(i < len) {
        if(buff[i] == EMV_TAG_APP_TEMPLATE) {
            uint8_t app_len = buff[++i];
            for(uint16_t j = i; j < i + app_len; j++) {
                if(buff[j] == EMV_TAG_AID) {
                    app_aid_found = true;
                    app->aid_len = buff[j + 1];
                    emv_parse_TLV(app->aid, buff, &j);
                } else if(buff[j] == EMV_TAG_PRIORITY) {
                    emv_parse_TLV(&app->priority, buff, &j);
                }
            }
            i += app_len;
        }
        i++;
    }
    return app_aid_found;
}

uint16_t emv_prepare_select_app(uint8_t* dest, EmvApplication* app) {
    const uint8_t emv_select_header[] = {
        0x00,
        0xA4, // SELECT application
        0x04,
        0x00 // P1:By name, P2:First or only occurence
    };
    uint16_t size = sizeof(emv_select_header);
    // Copy header
    memcpy(dest, emv_select_header, size);
    // Copy AID
    dest[size++] = app->aid_len;
    memcpy(&dest[size], app->aid, app->aid_len);
    size += app->aid_len;
    dest[size++] = 0;
    return size;
}

bool emv_decode_select_app_response(uint8_t* buff, uint16_t len, EmvApplication* app) {
    uint16_t i = 0;
    bool found_name = false;

    while(i < len) {
        if(buff[i] == EMV_TAG_CARD_NAME) {
            uint8_t name_len = buff[i + 1];
            emv_parse_TLV((uint8_t*)app->name, buff, &i);
            app->name[name_len] = '\0';
            found_name = true;
        } else if(((buff[i] << 8) | buff[i + 1]) == EMV_TAG_PDOL) {
            i++;
            app->pdol.size = emv_parse_TLV(app->pdol.data, buff, &i);
        }
        i++;
    }
    return found_name;
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

uint16_t emv_prepare_get_proc_opt(uint8_t* dest, EmvApplication* app) {
    // Get processing option header
    const uint8_t emv_gpo_header[] = {0x80, 0xA8, 0x00, 0x00};
    uint16_t size = sizeof(emv_gpo_header);
    // Copy header
    memcpy(dest, emv_gpo_header, size);
    APDU pdol_data = {0, {0}};
    // Prepare and copy pdol parameters
    emv_prepare_pdol(&pdol_data, &app->pdol);
    dest[size++] = 0x02 + pdol_data.size;
    dest[size++] = 0x83;
    dest[size++] = pdol_data.size;
    memcpy(dest + size, pdol_data.data, pdol_data.size);
    size += pdol_data.size;
    dest[size++] = 0;
    return size;
}

bool emv_decode_get_proc_opt(uint8_t* buff, uint16_t len, EmvApplication* app) {
    for(uint16_t i = 0; i < len; i++) {
        if(buff[i] == EMV_TAG_CARD_NUM) {
            app->card_number_len = 8;
            memcpy(app->card_number, &buff[i + 2], app->card_number_len);
            return true;
        } else if(buff[i] == EMV_TAG_AFL) {
            app->afl.size = emv_parse_TLV(app->afl.data, buff, &i);
        }
    }
    return false;
}

uint16_t emv_prepare_read_sfi_record(uint8_t* dest, uint8_t sfi, uint8_t record_num) {
    const uint8_t sfi_param = (sfi << 3) | (1 << 2);
    const uint8_t emv_sfi_header[] = {
        0x00,
        0xB2, // READ RECORD
        record_num,
        sfi_param, // P1:record_number and P2:SFI
        0x00 // Le
    };
    uint16_t size = sizeof(emv_sfi_header);
    memcpy(dest, emv_sfi_header, size);
    return size;
}

bool emv_decode_read_sfi_record(uint8_t* buff, uint16_t len, EmvApplication* app) {
    bool pan_parsed = false;
    for(uint16_t i = 0; i < len; i++) {
        if(buff[i] == EMV_TAG_PAN) {
            if(buff[i + 1] == 8 || buff[i + 1] == 10) {
                app->card_number_len = buff[i + 1];
                memcpy(app->card_number, &buff[i + 2], app->card_number_len);
                pan_parsed = true;
            }
        } else if(emv_decode_search_tag_u16_r(EMV_TAG_EXP_DATE, buff, &i)) {
            app->exp_year = buff[i++];
            app->exp_month = buff[i++];
        } else if(emv_decode_search_tag_u16_r(EMV_TAG_CURRENCY_CODE, buff, &i)) {
            app->currency_code = (buff[i] << 8) | buff[i + 1];
            i += 2;
        } else if(emv_decode_search_tag_u16_r(EMV_TAG_COUNTRY_CODE, buff, &i)) {
            app->country_code = (buff[i] << 8) | buff[i + 1];
            i += 2;
        }
    }
    return pan_parsed;
}

uint16_t emv_select_ppse_ans(uint8_t* buff) {
    memcpy(buff, select_ppse_ans, sizeof(select_ppse_ans));
    return sizeof(select_ppse_ans);
}

uint16_t emv_select_app_ans(uint8_t* buff) {
    memcpy(buff, select_app_ans, sizeof(select_app_ans));
    return sizeof(select_app_ans);
}

uint16_t emv_get_proc_opt_ans(uint8_t* buff) {
    memcpy(buff, pdol_ans, sizeof(pdol_ans));
    return sizeof(pdol_ans);
}
