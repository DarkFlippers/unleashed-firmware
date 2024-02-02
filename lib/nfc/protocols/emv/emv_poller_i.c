#include "emv_poller_i.h"
#include "protocols/emv/emv.h"

#define TAG "EMVPoller"

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

EmvError emv_process_error(Iso14443_4aError error) {
    switch(error) {
    case Iso14443_4aErrorNone:
        return EmvErrorNone;
    case Iso14443_4aErrorNotPresent:
        return EmvErrorNotPresent;
    case Iso14443_4aErrorTimeout:
        return EmvErrorTimeout;
    default:
        return EmvErrorProtocol;
    }
}

static void emv_trace(EmvPoller* instance, const char* message) {
    if(furi_log_get_level() == FuriLogLevelTrace) {
        FURI_LOG_T(TAG, "%s", message);

        printf("TX: ");
        size_t size = bit_buffer_get_size_bytes(instance->tx_buffer);
        for(size_t i = 0; i < size; i++) {
            printf("%02X ", bit_buffer_get_byte(instance->tx_buffer, i));
        }

        printf("\r\nRX: ");
        size = bit_buffer_get_size_bytes(instance->rx_buffer);
        for(size_t i = 0; i < size; i++) {
            printf("%02X ", bit_buffer_get_byte(instance->rx_buffer, i));
        }
        printf("\r\n");
    }
}

static bool
    emv_decode_tlv_tag(const uint8_t* buff, uint16_t tag, uint8_t tlen, EmvApplication* app) {
    uint8_t i = 0;
    bool success = false;

    switch(tag) {
    case EMV_TAG_LOG_FMT:
        furi_check(tlen < sizeof(app->log_fmt));
        memcpy(app->log_fmt, &buff[i], tlen);
        app->log_fmt_len = tlen;
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_LOG_FMT %X: len %d", tag, tlen);
        break;
    case EMV_TAG_GPO_FMT1:
        // skip AIP
        i += 2;
        tlen -= 2;
        furi_check(tlen < sizeof(app->afl.data));
        memcpy(app->afl.data, &buff[i], tlen);
        app->afl.size = tlen;
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_GPO_FMT1 %X: ", tag);
        break;
    case EMV_TAG_AID:
        app->aid_len = tlen;
        memcpy(app->aid, &buff[i], tlen);
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_AID %X: ", tag);
        for(size_t x = 0; x < tlen; x++) {
            FURI_LOG_RAW_T("%02X ", app->aid[x]);
        }
        FURI_LOG_RAW_T("\r\n");
        break;
    case EMV_TAG_PRIORITY:
        memcpy(&app->priority, &buff[i], tlen);
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_APP_PRIORITY %X: %d", tag, app->priority);
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
    // Tracks data https://murdoch.is/papers/defcon20emvdecode.pdf
    case EMV_TAG_TRACK_1_EQUIV: {
        // Contain PAN and expire date
        char track_1_equiv[80];
        memcpy(track_1_equiv, &buff[i], tlen);
        track_1_equiv[tlen] = '\0';
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_TRACK_1_EQUIV %x : %s", tag, track_1_equiv);
        break;
    }
    case EMV_TAG_TRACK_2_DATA:
    case EMV_TAG_TRACK_2_EQUIV: {
        FURI_LOG_T(TAG, "found EMV_TAG_TRACK_2 %X", tag);
        // 0xD0 delimits PAN from expiry (YYMM)
        for(int x = 1; x < tlen; x++) {
            if(buff[i + x + 1] > 0xD0) {
                memcpy(app->pan, &buff[i], x + 1);
                app->pan_len = x + 1;
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
        FURI_LOG_T(TAG, "found EMV_TAG_TRACK_2 %X : %s", tag, track_2_equiv);
        success = true;
        break;
    }
    case EMV_TAG_CARDHOLDER_NAME: {
        char name[27];
        memcpy(name, &buff[i], tlen);
        name[tlen] = '\0';
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_CARDHOLDER_NAME %x: %s", tag, name);
        break;
    }
    case EMV_TAG_PAN:
        memcpy(app->pan, &buff[i], tlen);
        app->pan_len = tlen;
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_PAN %x", tag);
        break;
    case EMV_TAG_EXP_DATE:
        app->exp_year = buff[i];
        app->exp_month = buff[i + 1];
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_EXP_DATE %x", tag);
        break;
    case EMV_TAG_CURRENCY_CODE:
        app->currency_code = (buff[i] << 8 | buff[i + 1]);
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_CURRENCY_CODE %x", tag);
        break;
    case EMV_TAG_COUNTRY_CODE:
        app->country_code = (buff[i] << 8 | buff[i + 1]);
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_COUNTRY_CODE %x", tag);
        break;
    case EMV_TAG_LOG_ENTRY:
        app->log_sfi = buff[i];
        app->log_records = buff[i + 1];
        success = true;
        FURI_LOG_T(
            TAG,
            "found EMV_TAG_LOG_ENTRY %x: sfi 0x%x, records %d",
            tag,
            app->log_sfi,
            app->log_records);
        break;
    case EMV_TAG_LAST_ONLINE_ATC:
        app->last_online_atc = (buff[i] << 8 | buff[i + 1]);
        success = true;
        break;
    case EMV_TAG_ATC:
        if(app->saving_trans_list)
            app->trans[app->active_tr].atc = (buff[i] << 8 | buff[i + 1]);
        else
            app->transaction_counter = (buff[i] << 8 | buff[i + 1]);
        success = true;
        break;
    case EMV_TAG_LOG_AMOUNT:
        memcpy(&app->trans[app->active_tr].amount, &buff[i], tlen);
        success = true;
        break;
    case EMV_TAG_LOG_COUNTRY:
        app->trans[app->active_tr].country = (buff[i] << 8 | buff[i + 1]);
        success = true;
        break;
    case EMV_TAG_LOG_CURRENCY:
        app->trans[app->active_tr].currency = (buff[i] << 8 | buff[i + 1]);
        success = true;
        break;
    case EMV_TAG_LOG_DATE:
        memcpy(&app->trans[app->active_tr].date, &buff[i], tlen);
        success = true;
        break;
    case EMV_TAG_LOG_TIME:
        memcpy(&app->trans[app->active_tr].time, &buff[i], tlen);
        success = true;
        break;
    case EMV_TAG_PIN_TRY_COUNTER:
        app->pin_try_counter = buff[i];
        success = true;
        FURI_LOG_T(TAG, "found EMV_TAG_PIN_TRY_COUNTER %x: %d", tag, app->pin_try_counter);
        break;
    }
    return success;
}

static bool emv_response_error(const uint8_t* buff, uint16_t len) {
    uint8_t i = 0;
    uint8_t first_byte = 0;
    bool error = true;

    first_byte = buff[i];

    if((len == 2) && ((first_byte >> 4) == 6)) {
        switch(buff[i]) {
        case EMV_TAG_RESP_BUF_SIZE:
            FURI_LOG_T(TAG, " Wrong length. Read %02X bytes", buff[i + 1]);
            // Need to request SFI again with this length value
            return error;
        case EMV_TAG_RESP_BYTES_AVAILABLE:
            FURI_LOG_T(TAG, " Bytes available: %02X", buff[i + 1]);
            // Need to request one more time
            return error;

        default:
            FURI_LOG_T(TAG, " Error/warning code: %02X %02X", buff[i], buff[i + 1]);
            return error;
        }
    }
    return false;
}

static bool
    emv_parse_tag(const uint8_t* buff, uint16_t len, uint16_t* t, uint8_t* tl, uint8_t* off) {
    uint8_t i = *off;
    uint16_t tag = 0;
    uint8_t first_byte = 0;
    uint8_t tlen = 0;
    bool success = false;

    first_byte = buff[i];

    if(emv_response_error(buff, len)) return success;

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

    *off = i;
    *t = tag;
    *tl = tlen;
    success = true;
    return success;
}

static bool emv_decode_tl(
    const uint8_t* buff,
    uint16_t len,
    const uint8_t* fmt,
    uint8_t fmt_len,
    EmvApplication* app) {
    uint8_t i = 0;
    uint8_t f = 0;
    uint16_t tag = 0;
    uint8_t tlen = 0;
    bool success = false;

    if(emv_response_error(buff, len)) return success;

    while(f < fmt_len && i < len) {
        success = emv_parse_tag(fmt, fmt_len, &tag, &tlen, &f);
        if(!success) return success;
        emv_decode_tlv_tag(&buff[i], tag, tlen, app);
        i += tlen;
    }
    success = true;
    return success;
}

static bool emv_decode_response_tlv(const uint8_t* buff, uint8_t len, EmvApplication* app) {
    uint8_t i = 0;
    uint16_t tag = 0;
    uint8_t first_byte = 0;
    uint8_t tlen = 0;
    bool success = false;

    while(i < len) {
        first_byte = buff[i];

        success = emv_parse_tag(buff, len, &tag, &tlen, &i);
        if(!success) return success;

        if((first_byte & 32) == 32) { // "Constructed" -- contains more TLV data to parse
            FURI_LOG_T(TAG, "Constructed TLV %x", tag);
            if(!emv_decode_response_tlv(&buff[i], tlen, app)) {
                FURI_LOG_T(TAG, "Failed to decode response for %x", tag);
                // return false;
            } else {
                success = true;
            }
        } else {
            emv_decode_tlv_tag(&buff[i], tag, tlen, app);
        }
        i += tlen;
    }
    return success;
}

static void emv_prepare_pdol(APDU* dest, APDU* src) {
    uint16_t tag = 0;
    uint8_t tlen = 0;
    uint8_t i = 0;
    while(i < src->size) {
        bool tag_found = emv_parse_tag(src->data, src->size, &tag, &tlen, &i);
        if(tag_found) {
            for(uint8_t j = 0; j < COUNT_OF(pdol_values); j++) {
                if(tag == pdol_values[j]->tag) {
                    memcpy(dest->data + dest->size, pdol_values[j]->data, tlen);
                    dest->size += tlen;
                    break;
                }
            }
        } else {
            // Unknown tag, fill zeros
            furi_check(dest->size + tlen < sizeof(dest->data));
            memset(dest->data + dest->size, 0, tlen);
            dest->size += tlen;
        }
    }
}

EmvError emv_poller_select_ppse(EmvPoller* instance) {
    EmvError error = EmvErrorNone;

    const uint8_t emv_select_ppse_cmd[] = {
        0x00, 0xA4, // SELECT ppse
        0x04, 0x00, // P1:By name, P2: empty
        0x0e, // Lc: Data length
        0x32, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, // Data string:
        0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, // 2PAY.SYS.DDF01 (PPSE)
        0x00 // Le
    };

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    bit_buffer_copy_bytes(instance->tx_buffer, emv_select_ppse_cmd, sizeof(emv_select_ppse_cmd));
    do {
        FURI_LOG_D(TAG, "Send select PPSE");

        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block_pwt_ext(
            instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            FURI_LOG_E(TAG, "Failed select PPSE");
            error = emv_process_error(iso14443_4a_error);
            break;
        }

        emv_trace(instance, "Select PPSE answer:");

        const uint8_t* buff = bit_buffer_get_data(instance->rx_buffer);

        if(!emv_decode_response_tlv(
               buff,
               bit_buffer_get_size_bytes(instance->rx_buffer),
               &instance->data->emv_application)) {
            error = EmvErrorProtocol;
            FURI_LOG_E(TAG, "Failed to parse application");
        }
    } while(false);

    return error;
}

EmvError emv_poller_select_application(EmvPoller* instance) {
    EmvError error = EmvErrorNone;

    const uint8_t emv_select_header[] = {
        0x00,
        0xA4, // SELECT application
        0x04,
        0x00 // P1:By name, P2:First or only occurence
    };

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    // Copy header
    bit_buffer_copy_bytes(instance->tx_buffer, emv_select_header, sizeof(emv_select_header));

    // Copy AID
    bit_buffer_append_byte(instance->tx_buffer, instance->data->emv_application.aid_len);
    bit_buffer_append_bytes(
        instance->tx_buffer,
        instance->data->emv_application.aid,
        instance->data->emv_application.aid_len);
    bit_buffer_append_byte(instance->tx_buffer, 0x00);

    do {
        FURI_LOG_D(TAG, "Start application");

        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block_pwt_ext(
            instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

        emv_trace(instance, "Start application answer:");

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            FURI_LOG_E(TAG, "Failed to read PAN or PDOL");
            error = emv_process_error(iso14443_4a_error);
            break;
        }

        const uint8_t* buff = bit_buffer_get_data(instance->rx_buffer);

        if(!emv_decode_response_tlv(
               buff,
               bit_buffer_get_size_bytes(instance->rx_buffer),
               &instance->data->emv_application)) {
            error = EmvErrorProtocol;
            FURI_LOG_E(TAG, "Failed to parse application");
            break;
        }

    } while(false);

    return error;
}

EmvError emv_poller_get_processing_options(EmvPoller* instance) {
    EmvError error = EmvErrorNone;

    const uint8_t emv_gpo_header[] = {0x80, 0xA8, 0x00, 0x00};

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    // Copy header
    bit_buffer_copy_bytes(instance->tx_buffer, emv_gpo_header, sizeof(emv_gpo_header));

    // Prepare and copy pdol parameters
    APDU pdol_data = {0, {0}};
    emv_prepare_pdol(&pdol_data, &instance->data->emv_application.pdol);

    bit_buffer_append_byte(instance->tx_buffer, 0x02 + pdol_data.size);
    bit_buffer_append_byte(instance->tx_buffer, 0x83);
    bit_buffer_append_byte(instance->tx_buffer, pdol_data.size);

    bit_buffer_append_bytes(instance->tx_buffer, pdol_data.data, pdol_data.size);
    bit_buffer_append_byte(instance->tx_buffer, 0x00);

    do {
        FURI_LOG_D(TAG, "Get proccessing options");

        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block_pwt_ext(
            instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

        emv_trace(instance, "Get processing options answer:");

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            FURI_LOG_E(TAG, "Failed to get processing options, error %u", iso14443_4a_error);
            error = emv_process_error(iso14443_4a_error);
            break;
        }

        const uint8_t* buff = bit_buffer_get_data(instance->rx_buffer);

        if(!emv_decode_response_tlv(
               buff,
               bit_buffer_get_size_bytes(instance->rx_buffer),
               &instance->data->emv_application)) {
            error = EmvErrorProtocol;
            FURI_LOG_E(TAG, "Failed to parse processing options");
        }
    } while(false);

    return error;
}

EmvError emv_poller_read_sfi_record(EmvPoller* instance, uint8_t sfi, uint8_t record_num) {
    EmvError error = EmvErrorNone;
    FuriString* text = furi_string_alloc();

    uint8_t sfi_param = (sfi << 3) | (1 << 2);
    uint8_t emv_sfi_header[] = {
        0x00,
        0xB2, // READ RECORD
        record_num, // P1:record_number
        sfi_param, // P2:SFI
        0x00 // Le
    };

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    bit_buffer_copy_bytes(instance->tx_buffer, emv_sfi_header, sizeof(emv_sfi_header));

    do {
        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block_pwt_ext(
            instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

        furi_string_printf(text, "SFI 0x%X record %d:", sfi, record_num);
        emv_trace(instance, furi_string_get_cstr(text));

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            FURI_LOG_E(TAG, "Failed to read SFI 0x%X record %d", sfi, record_num);
            error = emv_process_error(iso14443_4a_error);
            break;
        }
    } while(false);

    furi_string_free(text);

    return error;
}

EmvError emv_poller_read_afl(EmvPoller* instance) {
    EmvError error = EmvErrorNone;

    APDU* afl = &instance->data->emv_application.afl;

    if(afl->size == 0) {
        return false;
    }

    FURI_LOG_D(TAG, "Search PAN in SFI");

    // Iterate through all files
    for(size_t i = 0; i < instance->data->emv_application.afl.size; i += 4) {
        uint8_t sfi = afl->data[i] >> 3;
        uint8_t record_start = afl->data[i + 1];
        uint8_t record_end = afl->data[i + 2];
        // Iterate through all records in file
        for(uint8_t record = record_start; record <= record_end; ++record) {
            error = emv_poller_read_sfi_record(instance, sfi, record);
            if(error != EmvErrorNone) break;

            if(!emv_decode_response_tlv(
                   bit_buffer_get_data(instance->rx_buffer),
                   bit_buffer_get_size_bytes(instance->rx_buffer),
                   &instance->data->emv_application)) {
                error = EmvErrorProtocol;
                FURI_LOG_T(TAG, "Failed to parse SFI 0x%X record %d", sfi, record);
            }

            // Some READ RECORD returns 1 byte response 0x12/0x13 (IDK WTF),
            // then poller return Timeout to all subsequent requests.
            // TODO: remove below lines when it was fixed
            if(instance->data->emv_application.pan_len != 0)
                return EmvErrorNone; // Card number fetched
        }
    }

    return error;
}

static EmvError emv_poller_req_get_data(EmvPoller* instance, uint16_t tag) {
    EmvError error = EmvErrorNone;

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    bit_buffer_append_byte(instance->tx_buffer, EMV_REQ_GET_DATA >> 8);
    bit_buffer_append_byte(instance->tx_buffer, EMV_REQ_GET_DATA & 0xFF);
    bit_buffer_append_byte(instance->tx_buffer, tag >> 8);
    bit_buffer_append_byte(instance->tx_buffer, tag & 0xFF);
    bit_buffer_append_byte(instance->tx_buffer, 0x00); //Length

    do {
        FURI_LOG_D(TAG, "Get data for tag 0x%x", tag);

        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block_pwt_ext(
            instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

        emv_trace(instance, "Get log data answer:");

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            FURI_LOG_E(TAG, "Failed to get data, error %u", iso14443_4a_error);
            error = emv_process_error(iso14443_4a_error);
            break;
        }

        const uint8_t* buff = bit_buffer_get_data(instance->rx_buffer);

        if(!emv_decode_response_tlv(
               buff,
               bit_buffer_get_size_bytes(instance->rx_buffer),
               &instance->data->emv_application)) {
            error = EmvErrorProtocol;
            FURI_LOG_E(TAG, "Failed to parse get data");
        }
    } while(false);

    return error;
}

EmvError emv_poller_get_pin_try_counter(EmvPoller* instance) {
    return emv_poller_req_get_data(instance, EMV_TAG_PIN_TRY_COUNTER);
}

EmvError emv_poller_get_last_online_atc(EmvPoller* instance) {
    return emv_poller_req_get_data(instance, EMV_TAG_LAST_ONLINE_ATC);
}

static EmvError emv_poller_get_log_format(EmvPoller* instance) {
    return emv_poller_req_get_data(instance, EMV_TAG_LOG_FMT);
}

EmvError emv_poller_read_log_entry(EmvPoller* instance) {
    EmvError error = EmvErrorProtocol;

    if(!instance->data->emv_application.log_sfi) return error;
    uint8_t records = instance->data->emv_application.log_records;
    if(records == 0) {
        return error;
    }

    instance->data->emv_application.saving_trans_list = true;
    error = emv_poller_get_log_format(instance);
    if(error != EmvErrorNone) return error;

    FURI_LOG_D(TAG, "Read Transaction logs");

    uint8_t sfi = instance->data->emv_application.log_sfi;
    uint8_t record_start = 1;
    uint8_t record_end = records;
    // Iterate through all records in file
    for(uint8_t record = record_start; record <= record_end; ++record) {
        error = emv_poller_read_sfi_record(instance, sfi, record);
        if(error != EmvErrorNone) break;
        if(!emv_decode_tl(
               bit_buffer_get_data(instance->rx_buffer),
               bit_buffer_get_size_bytes(instance->rx_buffer),
               instance->data->emv_application.log_fmt,
               instance->data->emv_application.log_fmt_len,
               &instance->data->emv_application)) {
            error = EmvErrorProtocol;
            FURI_LOG_T(TAG, "Failed to parse log SFI 0x%X record %d", sfi, record);
            break;
        }

        instance->data->emv_application.active_tr++;
        furi_check(
            instance->data->emv_application.active_tr <
            COUNT_OF(instance->data->emv_application.trans));
    }

    instance->data->emv_application.saving_trans_list = false;
    return error;
}
