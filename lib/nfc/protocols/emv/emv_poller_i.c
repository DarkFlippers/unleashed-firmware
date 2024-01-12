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

static bool emv_decode_response(const uint8_t* buff, uint16_t len, EmvApplication* app) {
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
                success = true;
                FURI_LOG_T(TAG, "found EMV_TAG_TRACK_2_EQUIV %x : %s", tag, track_2_equiv);
                break;
            }
            case EMV_TAG_PAN:
                memcpy(app->pan, &buff[i], tlen);
                app->pan_len = tlen;
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

        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block(
            instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            FURI_LOG_E(TAG, "Failed select PPSE");
            error = emv_process_error(iso14443_4a_error);
            break;
        }

        emv_trace(instance, "Select PPSE answer:");

        const uint8_t* buff = bit_buffer_get_data(instance->rx_buffer);

        if(!emv_decode_response(
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

    //  DELETE IT???????????????????????????????????????????????????????????????????????????????????????
    instance->data->emv_application.app_started = false;

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

        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block(
            instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

        emv_trace(instance, "Start application answer:");

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            FURI_LOG_E(TAG, "Failed to read PAN or PDOL");
            error = emv_process_error(iso14443_4a_error);
            break;
        }

        const uint8_t* buff = bit_buffer_get_data(instance->rx_buffer);

        if(!emv_decode_response(
               buff,
               bit_buffer_get_size_bytes(instance->rx_buffer),
               &instance->data->emv_application)) {
            error = EmvErrorProtocol;
            FURI_LOG_E(TAG, "Failed to parse application");
            break;
        }

        instance->data->emv_application.app_started = true;
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

        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block(
            instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

        emv_trace(instance, "Get processing options answer:");

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            FURI_LOG_E(TAG, "Failed to get processing options, error %u", iso14443_4a_error);
            error = emv_process_error(iso14443_4a_error);
            break;
        }

        const uint8_t* buff = bit_buffer_get_data(instance->rx_buffer);

        if(!emv_decode_response(
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
        Iso14443_4aError iso14443_4a_error = iso14443_4a_poller_send_block(
            instance->iso14443_4a_poller, instance->tx_buffer, instance->rx_buffer);

        emv_trace(instance, "SFI record:");

        if(iso14443_4a_error != Iso14443_4aErrorNone) {
            error = emv_process_error(iso14443_4a_error);
            break;
        }

        const uint8_t* buff = bit_buffer_get_data(instance->rx_buffer);

        if(!emv_decode_response(
               buff,
               bit_buffer_get_size_bytes(instance->rx_buffer),
               &instance->data->emv_application)) {
            error = EmvErrorProtocol;
            FURI_LOG_E(TAG, "Failed to read SFI record %d", record_num);
        }
    } while(false);

    return error;
}

EmvError emv_poller_read_files(EmvPoller* instance) {
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
            error |= emv_poller_read_sfi_record(instance, sfi, record);
        }
    }

    return error;
}

EmvError emv_poller_read(EmvPoller* instance) {
    furi_assert(instance);
    EmvError error = EmvErrorNone;

    memset(&instance->data->emv_application, 0, sizeof(EmvApplication));
    do {
        error |= emv_poller_select_ppse(instance);
        if(error != EmvErrorNone) break;

        error |= emv_poller_select_application(instance);
        if(error != EmvErrorNone) break;

        if(emv_poller_get_processing_options(instance) != EmvErrorNone)
            error = emv_poller_read_files(instance);

    } while(false);

    return error;
}