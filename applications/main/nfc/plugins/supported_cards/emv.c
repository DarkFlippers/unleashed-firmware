/*
 * Parser for EMV cards.
 *
 * Copyright 2023 Leptoptilos <leptoptilos@icloud.com>
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "core/string.h"
#include "furi_hal_rtc.h"
#include "helpers/nfc_emv_parser.h"
#include "nfc_supported_card_plugin.h"

#include "protocols/emv/emv.h"
#include "protocols/nfc_protocol.h"
#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <nfc/helpers/nfc_util.h>

#define TAG "EMV"

bool emv_get_currency_name(uint16_t cur_code, FuriString* currency_name) {
    if(!cur_code) return false;

    Storage* storage = furi_record_open(RECORD_STORAGE);

    bool succsess = nfc_emv_parser_get_currency_name(storage, cur_code, currency_name);

    furi_record_close(RECORD_STORAGE);
    return succsess;
}

bool emv_get_country_name(uint16_t country_code, FuriString* country_name) {
    if(!country_code) return false;

    Storage* storage = furi_record_open(RECORD_STORAGE);

    bool succsess = nfc_emv_parser_get_country_name(storage, country_code, country_name);

    furi_record_close(RECORD_STORAGE);
    return succsess;
}

bool emv_get_aid_name(const EmvApplication* apl, FuriString* aid_name) {
    const uint8_t len = apl->aid_len;

    if(!len) return false;

    Storage* storage = furi_record_open(RECORD_STORAGE);

    bool succsess = nfc_emv_parser_get_aid_name(storage, apl->aid, len, aid_name);

    furi_record_close(RECORD_STORAGE);
    return succsess;
}

static bool emv_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    bool parsed = false;

    const EmvData* data = nfc_device_get_data(device, NfcProtocolEmv);
    const EmvApplication app = data->emv_application;

    do {
        if(strlen(app.label))
            furi_string_cat_printf(parsed_data, "\e#%s\n", app.label);
        else
            furi_string_cat_printf(parsed_data, "\e#%s\n", "EMV");

        if(app.pan_len) {
            FuriString* pan = furi_string_alloc();
            for(uint8_t i = 0; i < app.pan_len; i += 2) {
                furi_string_cat_printf(pan, "%02X%02X ", app.pan[i], app.pan[i + 1]);
            }

            // Cut padding 'F' from card number
            size_t end = furi_string_search_rchar(pan, 'F');
            if(end) furi_string_left(pan, end);
            furi_string_cat(parsed_data, pan);
            furi_string_free(pan);
        }

        if(app.exp_month | app.exp_year)
            furi_string_cat_printf(parsed_data, "\nExp: %02X/%02X\n", app.exp_month, app.exp_year);

        FuriString* str = furi_string_alloc();
        bool storage_readed = emv_get_country_name(app.country_code, str);

        if(storage_readed)
            furi_string_cat_printf(parsed_data, "Country: %s\n", furi_string_get_cstr(str));

        storage_readed = emv_get_currency_name(app.currency_code, str);
        if(storage_readed)
            furi_string_cat_printf(parsed_data, "Currency: %s\n", furi_string_get_cstr(str));

        if(app.pin_try_counter != 0xFF)
            furi_string_cat_printf(parsed_data, "PIN attempts left: %d\n", app.pin_try_counter);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin emv_plugin = {
    .protocol = NfcProtocolEmv,
    .verify = NULL,
    .read = NULL,
    .parse = emv_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor emv_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &emv_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* emv_plugin_ep() {
    return &emv_plugin_descriptor;
}