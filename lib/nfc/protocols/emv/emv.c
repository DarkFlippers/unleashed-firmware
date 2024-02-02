//#include "emv_i.h"

#include "flipper_format.h"
#include <core/common_defines.h>
#include "protocols/emv/emv.h"
#include <furi.h>
#include <stdlib.h>
#include <string.h>

#define EMV_PROTOCOL_NAME "EMV"

const NfcDeviceBase nfc_device_emv = {
    .protocol_name = EMV_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)emv_alloc,
    .free = (NfcDeviceFree)emv_free,
    .reset = (NfcDeviceReset)emv_reset,
    .copy = (NfcDeviceCopy)emv_copy,
    .verify = (NfcDeviceVerify)emv_verify,
    .load = (NfcDeviceLoad)emv_load,
    .save = (NfcDeviceSave)emv_save,
    .is_equal = (NfcDeviceEqual)emv_is_equal,
    .get_name = (NfcDeviceGetName)emv_get_device_name,
    .get_uid = (NfcDeviceGetUid)emv_get_uid,
    .set_uid = (NfcDeviceSetUid)emv_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)emv_get_base_data,
};

EmvData* emv_alloc() {
    EmvData* data = malloc(sizeof(EmvData));
    data->iso14443_4a_data = iso14443_4a_alloc();
    data->emv_application.pin_try_counter = 0xff;

    return data;
}

void emv_free(EmvData* data) {
    furi_assert(data);

    emv_reset(data);
    iso14443_4a_free(data->iso14443_4a_data);
    free(data);
}

void emv_reset(EmvData* data) {
    furi_assert(data);

    iso14443_4a_reset(data->iso14443_4a_data);

    memset(&data->emv_application, 0, sizeof(EmvApplication));
}

void emv_copy(EmvData* destination, const EmvData* source) {
    furi_assert(destination);
    furi_assert(source);

    emv_reset(destination);

    iso14443_4a_copy(destination->iso14443_4a_data, source->iso14443_4a_data);
    destination->emv_application = source->emv_application;
}

bool emv_verify(EmvData* data, const FuriString* device_type) {
    UNUSED(data);
    return furi_string_equal_str(device_type, EMV_PROTOCOL_NAME);
}

bool emv_load(EmvData* data, FlipperFormat* ff, uint32_t version) {
    furi_assert(data);

    FuriString* temp_str = furi_string_alloc();
    bool parsed = false;

    do {
        // Read ISO14443_4A data
        if(!iso14443_4a_load(data->iso14443_4a_data, ff, version)) break;

        EmvApplication* app = &data->emv_application;

        if(!flipper_format_read_string(ff, "Name", temp_str)) break;
        strcpy(app->name, furi_string_get_cstr(temp_str));

        //Read label
        if(!flipper_format_read_string(ff, "Payment system", temp_str)) break;
        strcpy(app->payment_sys, furi_string_get_cstr(temp_str));

        uint32_t pan_len;
        if(!flipper_format_read_uint32(ff, "PAN length", &pan_len, 1)) break;
        app->pan_len = pan_len;

        if(!flipper_format_read_hex(ff, "PAN", app->pan, pan_len)) break;

        uint32_t aid_len;
        if(!flipper_format_read_uint32(ff, "AID length", &aid_len, 1)) break;
        app->aid_len = aid_len;

        if(!flipper_format_read_hex(ff, "AID", app->aid, aid_len)) break;

        if(!flipper_format_read_hex(ff, "Country code", (uint8_t*)&app->country_code, 2)) break;

        if(!flipper_format_read_hex(ff, "Currency code", (uint8_t*)&app->currency_code, 2)) break;

        if(!flipper_format_read_hex(ff, "Expiration year", &app->exp_year, 1)) break;
        if(!flipper_format_read_hex(ff, "Expiration month", &app->exp_month, 1)) break;
        if(!flipper_format_read_hex(ff, "Expiration day", &app->exp_day, 1)) break;

        if(!flipper_format_read_hex(ff, "Issue year", &app->issue_year, 1)) break;
        if(!flipper_format_read_hex(ff, "Issue month", &app->issue_month, 1)) break;
        if(!flipper_format_read_hex(ff, "Issue day", &app->issue_day, 1)) break;

        uint32_t pin_try_counter;
        if(!flipper_format_read_uint32(ff, "PIN counter", &pin_try_counter, 1)) break;
        app->pin_try_counter = pin_try_counter;

        parsed = true;
    } while(false);

    furi_string_free(temp_str);

    return parsed;
}

bool emv_save(const EmvData* data, FlipperFormat* ff) {
    furi_assert(data);

    FuriString* temp_str = furi_string_alloc();
    bool saved = false;

    do {
        EmvApplication app = data->emv_application;
        if(!iso14443_4a_save(data->iso14443_4a_data, ff)) break;

        if(!flipper_format_write_comment_cstr(ff, "EMV specific data:\n")) break;

        if(!flipper_format_write_string_cstr(ff, "Name", app.name)) break;

        if(!flipper_format_write_string_cstr(ff, "Payment system", app.payment_sys)) break;

        uint32_t pan_len = app.pan_len;
        if(!flipper_format_write_uint32(ff, "PAN length", &pan_len, 1)) break;

        if(!flipper_format_write_hex(ff, "PAN", app.pan, pan_len)) break;

        uint32_t aid_len = app.aid_len;
        if(!flipper_format_write_uint32(ff, "AID length", &aid_len, 1)) break;

        if(!flipper_format_write_hex(ff, "AID", app.aid, aid_len)) break;

        if(!flipper_format_write_hex(ff, "Country code", (uint8_t*)&app.country_code, 2)) break;

        if(!flipper_format_write_hex(ff, "Currency code", (uint8_t*)&app.currency_code, 2)) break;

        if(!flipper_format_write_hex(ff, "Expiration year", (uint8_t*)&app.exp_year, 1)) break;
        if(!flipper_format_write_hex(ff, "Expiration month", (uint8_t*)&app.exp_month, 1)) break;
        if(!flipper_format_write_hex(ff, "Expiration day", (uint8_t*)&app.exp_day, 1)) break;

        if(!flipper_format_write_hex(ff, "Issue year", (uint8_t*)&app.issue_year, 1)) break;
        if(!flipper_format_write_hex(ff, "Issue month", (uint8_t*)&app.issue_month, 1)) break;
        if(!flipper_format_write_hex(ff, "Issue day", (uint8_t*)&app.issue_day, 1)) break;

        if(!flipper_format_write_uint32(ff, "PIN counter", (uint32_t*)&app.pin_try_counter, 1))
            break;

        saved = true;
    } while(false);

    furi_string_free(temp_str);

    return saved;
}

bool emv_is_equal(const EmvData* data, const EmvData* other) {
    furi_assert(data);
    furi_assert(other);

    return iso14443_4a_is_equal(data->iso14443_4a_data, other->iso14443_4a_data) &&
           memcmp(&data->emv_application, &other->emv_application, sizeof(EmvApplication)) == 0;
}

const char* emv_get_device_name(const EmvData* data, NfcDeviceNameType name_type) {
    UNUSED(data);
    UNUSED(name_type);
    return EMV_PROTOCOL_NAME;
}

const uint8_t* emv_get_uid(const EmvData* data, size_t* uid_len) {
    furi_assert(data);

    return iso14443_4a_get_uid(data->iso14443_4a_data, uid_len);
}

bool emv_set_uid(EmvData* data, const uint8_t* uid, size_t uid_len) {
    furi_assert(data);

    return iso14443_4a_set_uid(data->iso14443_4a_data, uid, uid_len);
}

Iso14443_4aData* emv_get_base_data(const EmvData* data) {
    furi_assert(data);

    return data->iso14443_4a_data;
}