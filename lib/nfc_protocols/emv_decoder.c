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

static uint16_t emv_parse_TLV(uint8_t* dest, uint8_t* src, uint16_t* idx) {
    uint8_t len = src[*idx + 1];
    memcpy(dest, &src[*idx + 2], len);
    *idx = *idx + len + 1;
    return len;
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
            memcpy(app->card_number, &buff[i + 2], 8);
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
    for(uint16_t i = 0; i < len; i++) {
        if(buff[i] == EMV_TAG_PAN) {
            memcpy(app->card_number, &buff[i + 2], 8);
            return true;
        }
    }
    return false;
}
