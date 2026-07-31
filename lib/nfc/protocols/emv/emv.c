//#include "emv_i.h"

#include "flipper_format.h"
#include <core/common_defines.h>
#include "protocols/emv/emv.h"
#include <furi.h>
#include <stdlib.h>
#include <string.h>

#define TAG "EMV"

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

EmvData* emv_alloc(void) {
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

// An empty value reads back as a failure, and emv_save() writes these keys even when the card
// had no such tag, so a nameless card must still load. An absent key is a different matter: the
// stream is left at EOF and the load fails at the next required key.
static void emv_load_string(FlipperFormat* ff, const char* key, char* dest, size_t dest_size) {
    FuriString* value = furi_string_alloc();

    if(flipper_format_read_string(ff, key, value)) {
        const size_t len = furi_string_size(value);
        if(len >= dest_size) {
            FURI_LOG_W(TAG, "\"%s\" is %zu bytes, truncating to %zu", key, len, dest_size - 1);
        }
        strlcpy(dest, furi_string_get_cstr(value), dest_size);
    }

    furi_string_free(value);
}

// Reject rather than clamp: the AID is transmitted verbatim in the SELECT APDU and the lengths
// bound the render loops, so a truncated value is worse than a failed load.
static bool emv_load_length(FlipperFormat* ff, const char* key, size_t max_len, uint8_t* len) {
    uint32_t value;
    if(!flipper_format_read_uint32(ff, key, &value, 1)) return false;

    if(value > max_len) {
        FURI_LOG_E(TAG, "\"%s\" is %lu, max %zu", key, (unsigned long)value, max_len);
        return false;
    }

    *len = value;
    return true;
}

bool emv_load(EmvData* data, FlipperFormat* ff, uint32_t version) {
    furi_assert(data);

    bool parsed = false;

    do {
        // Read ISO14443_4A data
        if(!iso14443_4a_load(data->iso14443_4a_data, ff, version)) break;

        EmvApplication* app = &data->emv_application;

        emv_load_string(ff, "Cardholder name", app->cardholder_name, sizeof(app->cardholder_name));
        emv_load_string(
            ff, "Application name", app->application_name, sizeof(app->application_name));
        emv_load_string(
            ff, "Application label", app->application_label, sizeof(app->application_label));

        // Record the length only once its bytes are in, so it always describes what was read
        uint8_t pan_len;
        if(!emv_load_length(ff, "PAN length", sizeof(app->pan), &pan_len)) break;
        if(!flipper_format_read_hex(ff, "PAN", app->pan, pan_len)) break;
        app->pan_len = pan_len;

        uint8_t aid_len;
        if(!emv_load_length(ff, "AID length", sizeof(app->aid), &aid_len)) break;
        if(!flipper_format_read_hex(ff, "AID", app->aid, aid_len)) break;
        app->aid_len = aid_len;

        if(!flipper_format_read_hex(
               ff, "Application interchange profile", app->application_interchange_profile, 2))
            break;

        if(!flipper_format_read_hex(ff, "Country code", (uint8_t*)&app->country_code, 2)) break;

        if(!flipper_format_read_hex(ff, "Currency code", (uint8_t*)&app->currency_code, 2)) break;

        if(!flipper_format_read_hex(ff, "Expiration year", &app->exp_year, 1)) break;
        if(!flipper_format_read_hex(ff, "Expiration month", &app->exp_month, 1)) break;
        if(!flipper_format_read_hex(ff, "Expiration day", &app->exp_day, 1)) break;

        if(!flipper_format_read_hex(ff, "Effective year", &app->effective_year, 1)) break;
        if(!flipper_format_read_hex(ff, "Effective month", &app->effective_month, 1)) break;
        if(!flipper_format_read_hex(ff, "Effective day", &app->effective_day, 1)) break;

        uint32_t pin_try_counter;
        if(!flipper_format_read_uint32(ff, "PIN try counter", &pin_try_counter, 1)) break;
        app->pin_try_counter = pin_try_counter;

        parsed = true;
    } while(false);

    return parsed;
}

bool emv_save(const EmvData* data, FlipperFormat* ff) {
    furi_assert(data);

    bool saved = false;

    do {
        EmvApplication app = data->emv_application;
        if(!iso14443_4a_save(data->iso14443_4a_data, ff)) break;

        if(!flipper_format_write_comment_cstr(ff, "EMV specific data:\n")) break;

        if(!flipper_format_write_string_cstr(ff, "Cardholder name", app.cardholder_name)) break;

        if(!flipper_format_write_string_cstr(ff, "Application name", app.application_name)) break;

        if(!flipper_format_write_string_cstr(ff, "Application label", app.application_label))
            break;

        uint32_t pan_len = app.pan_len;
        if(!flipper_format_write_uint32(ff, "PAN length", &pan_len, 1)) break;

        if(!flipper_format_write_hex(ff, "PAN", app.pan, pan_len)) break;

        uint32_t aid_len = app.aid_len;
        if(!flipper_format_write_uint32(ff, "AID length", &aid_len, 1)) break;

        if(!flipper_format_write_hex(ff, "AID", app.aid, aid_len)) break;

        if(!flipper_format_write_hex(
               ff, "Application interchange profile", app.application_interchange_profile, 2))
            break;

        if(!flipper_format_write_hex(ff, "Country code", (uint8_t*)&app.country_code, 2)) break;

        if(!flipper_format_write_hex(ff, "Currency code", (uint8_t*)&app.currency_code, 2)) break;

        if(!flipper_format_write_hex(ff, "Expiration year", (uint8_t*)&app.exp_year, 1)) break;
        if(!flipper_format_write_hex(ff, "Expiration month", (uint8_t*)&app.exp_month, 1)) break;
        if(!flipper_format_write_hex(ff, "Expiration day", (uint8_t*)&app.exp_day, 1)) break;

        if(!flipper_format_write_hex(ff, "Effective year", (uint8_t*)&app.effective_year, 1))
            break;
        if(!flipper_format_write_hex(ff, "Effective month", (uint8_t*)&app.effective_month, 1))
            break;
        if(!flipper_format_write_hex(ff, "Effective day", (uint8_t*)&app.effective_day, 1)) break;

        // pin_try_counter is a uint8_t; casting its address to uint32_t* read three neighbouring
        // bytes into the file, same as pan_len and aid_len are widened above
        uint32_t pin_try_counter = app.pin_try_counter;
        if(!flipper_format_write_uint32(ff, "PIN try counter", &pin_try_counter, 1)) break;

        saved = true;
    } while(false);

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
