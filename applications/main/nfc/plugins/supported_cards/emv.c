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

#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>

#include "protocols/emv/emv.h"
#include "helpers/nfc_emv_parser.h"

#include <bit_lib.h>
#include <datetime.h>
#include <locale/locale.h>

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
        if(strlen(app.application_label)) {
            furi_string_cat_printf(parsed_data, "\e#%s\n", app.application_label);
        } else if(strlen(app.application_name)) {
            furi_string_cat_printf(parsed_data, "\e#%s\n", app.application_name);
        } else
            furi_string_cat_printf(parsed_data, "\e#%s\n", "EMV");

        if(app.pan_len) {
            FuriString* pan = furi_string_alloc();
            for(uint8_t i = 0; i < app.pan_len; i += 2) {
                furi_string_cat_printf(pan, "%02X%02X ", app.pan[i], app.pan[i + 1]);
            }

            // Cut padding 'F' from card number
            size_t end = furi_string_search_rchar(pan, 'F');
            if(end) furi_string_left(pan, end);
            furi_string_cat_printf(pan, "\n");
            furi_string_cat(parsed_data, pan);

            furi_string_free(pan);
            parsed = true;
        }

        if(strlen(app.cardholder_name)) {
            furi_string_cat_printf(parsed_data, "Cardholder name: %s\n", app.cardholder_name);
            parsed = true;
        }

        bool nevermind = false;
        DateTime effective_datetime = {
            0,
            0,
            0,
            bit_lib_bytes_to_num_bcd(&app.effective_day, 1, &nevermind),
            bit_lib_bytes_to_num_bcd(&app.effective_month, 1, &nevermind),
            2000 + bit_lib_bytes_to_num_bcd(&app.effective_year, 1, &nevermind),
            0};
        DateTime expiration_datetime = {
            0,
            0,
            0,
            bit_lib_bytes_to_num_bcd(&app.exp_day, 1, &nevermind),
            bit_lib_bytes_to_num_bcd(&app.exp_month, 1, &nevermind),
            2000 + bit_lib_bytes_to_num_bcd(&app.exp_year, 1, &nevermind),
            0};

        LocaleDateFormat date_format = locale_get_date_format();
        const char* separator = (date_format == LocaleDateFormatDMY) ? "." : "/";

        FuriString* effective_date_str = furi_string_alloc();
        locale_format_date(effective_date_str, &effective_datetime, date_format, separator);

        FuriString* expiration_date_str = furi_string_alloc();
        locale_format_date(expiration_date_str, &expiration_datetime, date_format, separator);

        if(app.effective_month) {
            furi_string_cat_printf(
                parsed_data,
                "Effective: %s\n",
                app.effective_day ? furi_string_get_cstr(effective_date_str) :
                                    furi_string_get_cstr(effective_date_str) + 3);

            parsed = true;
        }

        if(app.exp_month) {
            furi_string_cat_printf(
                parsed_data,
                "Expires: %s\n",
                app.exp_day ? furi_string_get_cstr(expiration_date_str) :
                              furi_string_get_cstr(expiration_date_str) + 3);

            parsed = true;
        }

        FuriString* str = furi_string_alloc();
        bool storage_readed = emv_get_country_name(app.country_code, str);

        if(storage_readed) {
            furi_string_cat_printf(parsed_data, "Country: %s\n", furi_string_get_cstr(str));
            parsed = true;
        }

        storage_readed = emv_get_currency_name(app.currency_code, str);
        if(storage_readed) {
            furi_string_cat_printf(parsed_data, "Currency: %s\n", furi_string_get_cstr(str));
            parsed = true;
        }

        if(app.pin_try_counter != 0xFF) {
            furi_string_cat_printf(parsed_data, "PIN attempts left: %d\n", app.pin_try_counter);
            parsed = true;
        }

        if((app.application_interchange_profile[1] >> 6) & 0b1) {
            furi_string_cat_printf(parsed_data, "Mobile: yes\n");
            parsed = true;
        }

        if(!parsed) furi_string_cat_printf(parsed_data, "No data was parsed\n");

        furi_string_free(str);
        furi_string_free(effective_date_str);
        furi_string_free(expiration_date_str);

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
const FlipperAppPluginDescriptor* emv_plugin_ep(void) {
    return &emv_plugin_descriptor;
}
