/*
 * Parser for Kazan transport card (Kazan, Russia).
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

#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#include <bit_lib.h>
#include <datetime.h>
#include <locale/locale.h>

#define TAG "Kazan"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static const MfClassicKeyPair kazan_1k_keys_v1[] = {
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xE954024EE754, .b = 0x0CD464CDC100},
    {.a = 0xBC305FE2DA65, .b = 0xCF0EC6ACF2F9},
    {.a = 0xF7A545095C49, .b = 0x6862FD600F78},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
};

static const MfClassicKeyPair kazan_1k_keys_v2[] = {
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0x2058EAEE8446, .b = 0xCB9B23815F87},
    {.a = 0x492F3744A1DC, .b = 0x6B770AADA274},
    {.a = 0xF7A545095C49, .b = 0x6862FD600F78},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF},
};

static const MfClassicKeyPair kazan_1k_keys_v3[] = {
    {.a = 0x165D3B5280C0, .b = 0xFC7C2BB34E0F},
    {.a = 0xC178E3DA7A39, .b = 0xC70FB78B4934},
    {.a = 0x1BDF96089D2F, .b = 0x9500F058ABC5},
    {.a = 0xB65AA70AD524, .b = 0x733A63B8B7F3},
    {.a = 0x8BDB8FECDCAF, .b = 0xB0048EE71C0F},
    {.a = 0xBC10468ABF05, .b = 0x1700A7D5C034},
    {.a = 0xE7F3282E0C7D, .b = 0x65909B89BDA5},
    {.a = 0x986C63DD0355, .b = 0x901C125ED37D},
    {.a = 0x2058EAEE8446, .b = 0xCB9B23815F87},
    {.a = 0x492F3744A1DC, .b = 0x6B770AADA274},
    {.a = 0x87EB933B9BF7, .b = 0xFC98A9460EE5},
    {.a = 0x7EBC8337F8F0, .b = 0x887C97E53DBC},
    {.a = 0xA369BF6D4452, .b = 0x03BBA7CA2F24},
    {.a = 0x37569D7992EF, .b = 0x710BBD01B3B8},
    {.a = 0xD4AA94C4B5E8, .b = 0x7F5C4D210F0B},
    {.a = 0x521B8C4B2123, .b = 0x2D2392CC43A7},
};

enum SubscriptionType {
    SUBSCRIPTION_TYPE_UNKNOWN,
    SUBSCRIPTION_TYPE_PURSE,
    SUBSCRIPTION_TYPE_ABONNEMENT_BY_TIME,
    SUBSCRIPTION_TYPE_ABONNEMENT_BY_TRIPS,
};

enum SubscriptionType get_subscription_type(uint8_t value, FuriString* tariff_name) {
    switch(value) {
    case 0x51:
        furi_string_printf(tariff_name, "Social. Adult");
        return SUBSCRIPTION_TYPE_ABONNEMENT_BY_TIME;
    case 0x67:
        furi_string_printf(tariff_name, "Ground electric transport. 1 month");
        return SUBSCRIPTION_TYPE_ABONNEMENT_BY_TIME;
    case 0x0F:
        furi_string_printf(tariff_name, "Underground only");
        return SUBSCRIPTION_TYPE_ABONNEMENT_BY_TRIPS;
    case 0x6D:
        furi_string_printf(tariff_name, "Tram. 60 minutes. Transfer. 10 trips");
        return SUBSCRIPTION_TYPE_ABONNEMENT_BY_TRIPS;
    case 0x53:
        furi_string_printf(tariff_name, "Standart purse");
        return SUBSCRIPTION_TYPE_PURSE;
    case 0x01:
        furi_string_printf(tariff_name, "Token");
        return SUBSCRIPTION_TYPE_ABONNEMENT_BY_TRIPS;
    default:
        furi_string_printf(tariff_name, "Unknown");
        return SUBSCRIPTION_TYPE_UNKNOWN;
    }
}

static bool kazan_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verification_sector_number = 8;
        const uint8_t verification_block_number =
            mf_classic_get_first_block_num_of_sector(verification_sector_number) + 1;
        FURI_LOG_D(TAG, "Verifying sector %u", verification_sector_number);

        MfClassicKey key_1 = {0};
        bit_lib_num_to_bytes_be(
            kazan_1k_keys_v1[verification_sector_number].a, COUNT_OF(key_1.data), key_1.data);

        MfClassicAuthContext auth_context;
        MfClassicError error = mf_classic_poller_sync_auth(
            nfc, verification_block_number, &key_1, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(
                TAG, "Failed to read block %u: %d. Keys: v1", verification_block_number, error);

            MfClassicKey key_2 = {0};
            bit_lib_num_to_bytes_be(
                kazan_1k_keys_v2[verification_sector_number].a, COUNT_OF(key_2.data), key_2.data);

            MfClassicAuthContext auth_context;
            MfClassicError error = mf_classic_poller_sync_auth(
                nfc, verification_block_number, &key_2, MfClassicKeyTypeA, &auth_context);
            if(error != MfClassicErrorNone) {
                FURI_LOG_D(
                    TAG, "Failed to read block %u: %d. Keys: v2", verification_block_number, error);
                break;
            }
        }

        verified = true;
    } while(false);
    return verified;
}

static bool kazan_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicTypeMini;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        if(type != MfClassicType1k) break;

        MfClassicDeviceKeys keys_v1 = {
            .key_a_mask = 0,
            .key_b_mask = 0,
        };

        MfClassicDeviceKeys keys_v2 = {
            .key_a_mask = 0,
            .key_b_mask = 0,
        };

        MfClassicDeviceKeys keys_v3 = {
            .key_a_mask = 0,
            .key_b_mask = 0,
        };

        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(
                kazan_1k_keys_v1[i].a, sizeof(MfClassicKey), keys_v1.key_a[i].data);
            bit_lib_num_to_bytes_be(
                kazan_1k_keys_v2[i].a, sizeof(MfClassicKey), keys_v2.key_a[i].data);
            bit_lib_num_to_bytes_be(
                kazan_1k_keys_v3[i].a, sizeof(MfClassicKey), keys_v3.key_a[i].data);

            FURI_BIT_SET(keys_v1.key_a_mask, i);
            FURI_BIT_SET(keys_v2.key_a_mask, i);
            FURI_BIT_SET(keys_v3.key_a_mask, i);

            bit_lib_num_to_bytes_be(
                kazan_1k_keys_v1[i].b, sizeof(MfClassicKey), keys_v1.key_b[i].data);
            bit_lib_num_to_bytes_be(
                kazan_1k_keys_v2[i].b, sizeof(MfClassicKey), keys_v2.key_b[i].data);
            bit_lib_num_to_bytes_be(
                kazan_1k_keys_v3[i].b, sizeof(MfClassicKey), keys_v3.key_b[i].data);

            FURI_BIT_SET(keys_v1.key_b_mask, i);
            FURI_BIT_SET(keys_v2.key_b_mask, i);
            FURI_BIT_SET(keys_v3.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys_v1, data);
        if(error == MfClassicErrorNotPresent) {
            FURI_LOG_W(TAG, "Failed to read data: keys_v1");
            break;
        }

        if(error == MfClassicErrorPartialRead) {
            error = mf_classic_poller_sync_read(nfc, &keys_v2, data);
            if(error == MfClassicErrorNotPresent) {
                FURI_LOG_W(TAG, "Failed to read data: keys_v1");
                break;
            }
        }

        if(error == MfClassicErrorPartialRead) {
            error = mf_classic_poller_sync_read(nfc, &keys_v3, data);
            if(error == MfClassicErrorNotPresent) {
                FURI_LOG_W(TAG, "Failed to read data: keys_v3");
                break;
            }
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (error == MfClassicErrorNone);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static bool kazan_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        const uint8_t ticket_sector_number = 8;
        const uint8_t balance_sector_number = 9;

        // Verify keys
        MfClassicKeyPair keys = {};
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, ticket_sector_number);

        keys.a = bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        keys.b = bit_lib_bytes_to_num_be(sec_tr->key_b.data, COUNT_OF(sec_tr->key_b.data));

        if(((keys.a != kazan_1k_keys_v1[8].a) && (keys.a != kazan_1k_keys_v2[8].a)) ||
           ((keys.b != kazan_1k_keys_v1[8].b) && (keys.b != kazan_1k_keys_v2[8].b))) {
            FURI_LOG_D(TAG, "Parser: Failed to verify key a: %llu", keys.a);
            break;
        }

        // Parse data
        uint8_t start_block_num = mf_classic_get_first_block_num_of_sector(ticket_sector_number);

        const uint8_t* block_start_ptr = &data->block[start_block_num].data[6];

        FuriString* tariff_name = furi_string_alloc();
        enum SubscriptionType subscription_type =
            get_subscription_type(block_start_ptr[0], tariff_name);

        DateTime valid_from;
        valid_from.year = 2000 + block_start_ptr[1];
        valid_from.month = block_start_ptr[2];
        valid_from.day = block_start_ptr[3];

        DateTime valid_to;
        valid_to.year = 2000 + block_start_ptr[4];
        valid_to.month = block_start_ptr[5];
        valid_to.day = block_start_ptr[6];

        const uint8_t last_trip_block_number = 2;
        block_start_ptr = &data->block[start_block_num + last_trip_block_number].data[1];

        DateTime last_trip;
        last_trip.year = 2000 + block_start_ptr[0];
        last_trip.month = block_start_ptr[1];
        last_trip.day = block_start_ptr[2];
        last_trip.hour = block_start_ptr[3];
        last_trip.minute = block_start_ptr[4];
        bool is_last_trip_valid = (block_start_ptr[0] | block_start_ptr[1] | block_start_ptr[2]) &&
                                  (last_trip.day < 32 && last_trip.month < 12 &&
                                   last_trip.hour < 24 && last_trip.minute < 60);

        start_block_num = mf_classic_get_first_block_num_of_sector(balance_sector_number);
        block_start_ptr = &data->block[start_block_num].data[0];

        const uint32_t trip_counter = bit_lib_bytes_to_num_le(block_start_ptr, 4);

        size_t uid_len = 0;
        const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
        const uint32_t card_number = bit_lib_bytes_to_num_le(uid, 4);

        furi_string_cat_printf(
            parsed_data, "\e#Kazan transport card\nCard number: %lu\n", card_number);

        LocaleDateFormat date_format = locale_get_date_format();
        const char* separator = (date_format == LocaleDateFormatDMY) ? "." : "/";

        FuriString* valid_from_str = furi_string_alloc();
        locale_format_date(valid_from_str, &valid_from, date_format, separator);

        FuriString* valid_to_str = furi_string_alloc();
        locale_format_date(valid_to_str, &valid_to, date_format, separator);

        FuriString* last_trip_date_str = furi_string_alloc();
        locale_format_date(last_trip_date_str, &last_trip, date_format, separator);

        FuriString* last_trip_time_str = furi_string_alloc();
        locale_format_time(last_trip_time_str, &last_trip, locale_get_time_format(), false);

        if(subscription_type == SUBSCRIPTION_TYPE_PURSE) {
            furi_string_cat_printf(
                parsed_data,
                "Type: purse\nBalance: %lu RUR\nBalance valid:\nfrom: %s\nto: %s",
                trip_counter,
                furi_string_get_cstr(valid_from_str),
                furi_string_get_cstr(valid_to_str));
        }

        if(subscription_type == SUBSCRIPTION_TYPE_ABONNEMENT_BY_TRIPS) {
            furi_string_cat_printf(
                parsed_data,
                "Type: abonnement\nTariff: %s\nTrips left: %lu\nCard valid:\nfrom: %s\nto: %s",
                furi_string_get_cstr(tariff_name),
                trip_counter,
                furi_string_get_cstr(valid_from_str),
                furi_string_get_cstr(valid_to_str));
        }

        if(subscription_type == SUBSCRIPTION_TYPE_ABONNEMENT_BY_TIME) {
            furi_string_cat_printf(
                parsed_data,
                "Type: abonnement\nTariff: %s\nTotal valid time: %lu days\nCard valid:\nfrom: %s\nto: %s",
                furi_string_get_cstr(tariff_name),
                trip_counter,
                furi_string_get_cstr(valid_from_str),
                furi_string_get_cstr(valid_to_str));
        }

        if(subscription_type == SUBSCRIPTION_TYPE_UNKNOWN) {
            furi_string_cat_printf(
                parsed_data,
                "Type: unknown\nTariff: %s\nCounter: %lu\nValid from: %s\nValid to: %s",
                furi_string_get_cstr(tariff_name),
                trip_counter,
                furi_string_get_cstr(valid_from_str),
                furi_string_get_cstr(valid_to_str));
        }

        if(is_last_trip_valid) {
            furi_string_cat_printf(
                parsed_data,
                "\nLast trip: %s at %s",
                furi_string_get_cstr(last_trip_date_str),
                furi_string_get_cstr(last_trip_time_str));
        }

        furi_string_free(tariff_name);

        furi_string_free(valid_from_str);
        furi_string_free(valid_to_str);
        furi_string_free(last_trip_date_str);
        furi_string_free(last_trip_time_str);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin kazan_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = kazan_verify,
    .read = kazan_read,
    .parse = kazan_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor kazan_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &kazan_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* kazan_plugin_ep(void) {
    return &kazan_plugin_descriptor;
}
