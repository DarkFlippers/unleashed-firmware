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
#include "core/log.h"
#include "nfc_supported_card_plugin.h"

#include "protocols/mf_classic/mf_classic.h"
#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <nfc/helpers/nfc_util.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <stdbool.h>
#include <stdint.h>
#include <furi_hal_rtc.h>
#include "md5.h"

#define TAG "Kazan"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static const MfClassicKeyPair kazan_1k_keys[] = {
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

enum SubscriptionType {
    SUBSCRIPTION_TYPE_UNKNOWN,
    SUBSCRIPTION_TYPE_PURSE,
    SUBSCRIPTION_TYPE_ABONNEMENT,
};

enum SubscriptionType get_subscription_type(uint8_t value) {
    switch(value) {
    case 0:
    case 0x60:
    case 0x67:
    case 0x0F:
        return SUBSCRIPTION_TYPE_ABONNEMENT;
    case 0x53:
        return SUBSCRIPTION_TYPE_PURSE;
    default:
        return SUBSCRIPTION_TYPE_UNKNOWN;
    }
}

static bool kazan_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t ticket_sector_number = 8;
        const uint8_t ticket_block_number =
            mf_classic_get_first_block_num_of_sector(ticket_sector_number) + 1;
        FURI_LOG_D(TAG, "Verifying sector %u", ticket_sector_number);

        MfClassicKey key = {0};
        nfc_util_num2bytes(kazan_1k_keys[ticket_sector_number].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error = mf_classic_poller_sync_auth(
            nfc, ticket_block_number, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", ticket_block_number, error);
            break;
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

        MfClassicDeviceKeys keys = {
            .key_a_mask = 0,
            .key_b_mask = 0,
        };
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            nfc_util_num2bytes(kazan_1k_keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            nfc_util_num2bytes(kazan_1k_keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error != MfClassicErrorNone) {
            FURI_LOG_W(TAG, "Failed to read data");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = true;
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

        keys.a = nfc_util_bytes2num(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        keys.b = nfc_util_bytes2num(sec_tr->key_b.data, COUNT_OF(sec_tr->key_b.data));

        if((keys.a != 0xE954024EE754) && (keys.b != 0x0CD464CDC100)) break;

        // Parse data
        uint8_t start_block_num = mf_classic_get_first_block_num_of_sector(ticket_sector_number);

        const uint8_t* block_start_ptr = &data->block[start_block_num].data[6];

        enum SubscriptionType subscription_type = get_subscription_type(block_start_ptr[0]);

        FuriHalRtcDateTime valid_from;
        valid_from.year = 2000 + block_start_ptr[1];
        valid_from.month = block_start_ptr[2];
        valid_from.day = block_start_ptr[3];

        FuriHalRtcDateTime valid_to;
        valid_to.year = 2000 + block_start_ptr[4];
        valid_to.month = block_start_ptr[5];
        valid_to.day = block_start_ptr[6];

        const uint8_t last_trip_block_number = 2;
        block_start_ptr = &data->block[start_block_num + last_trip_block_number].data[1];

        FuriHalRtcDateTime last_trip;
        last_trip.year = 2000 + block_start_ptr[0];
        last_trip.month = block_start_ptr[1];
        last_trip.day = block_start_ptr[2];
        last_trip.hour = block_start_ptr[3];
        last_trip.minute = block_start_ptr[4];
        bool is_last_trip_valid = (block_start_ptr[0] | block_start_ptr[1] | block_start_ptr[0]) &&
                                  (last_trip.day < 32 && last_trip.month < 12 &&
                                   last_trip.hour < 24 && last_trip.minute < 60);

        start_block_num = mf_classic_get_first_block_num_of_sector(balance_sector_number);
        block_start_ptr = &data->block[start_block_num].data[0];

        const uint32_t trip_counter = (block_start_ptr[3] << 24) | (block_start_ptr[2] << 16) |
                                      (block_start_ptr[1] << 8) | (block_start_ptr[0]);

        size_t uid_len = 0;
        const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
        const uint32_t card_number = (uid[3] << 24) | (uid[2] << 16) | (uid[1] << 8) | (uid[0]);

        furi_string_cat_printf(
            parsed_data, "\e#Kazan transport card\nCard number: %lu\n", card_number);

        if(subscription_type == SUBSCRIPTION_TYPE_PURSE) {
            furi_string_cat_printf(
                parsed_data,
                "Type: purse\nBalance: %lu RUR\nBalance valid:\nfrom: %02u.%02u.%u\nto: %02u.%02u.%u",
                trip_counter,
                valid_from.day,
                valid_from.month,
                valid_from.year,
                valid_to.day,
                valid_to.month,
                valid_to.year);
        }

        if(subscription_type == SUBSCRIPTION_TYPE_ABONNEMENT) {
            furi_string_cat_printf(
                parsed_data,
                "Type: abonnement\nTrips left: %lu\nCard valid:\nfrom: %02u.%02u.%u\nto: %02u.%02u.%u",
                trip_counter,
                valid_from.day,
                valid_from.month,
                valid_from.year,
                valid_to.day,
                valid_to.month,
                valid_to.year);
        }

        if(subscription_type == SUBSCRIPTION_TYPE_UNKNOWN) {
            furi_string_cat_printf(
                parsed_data,
                "Type: unknown\nBalance: %lu RUR\nValid from: %02u.%02u.%u\nValid to: %02u.%02u.%u",
                trip_counter,
                valid_from.day,
                valid_from.month,
                valid_from.year,
                valid_to.day,
                valid_to.month,
                valid_to.year);
        }

        if(is_last_trip_valid) {
            furi_string_cat_printf(
                parsed_data,
                "\nLast trip: %02u.%02u.%u at %02u:%02u",
                last_trip.day,
                last_trip.month,
                last_trip.year,
                last_trip.hour,
                last_trip.minute);
        }

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
const FlipperAppPluginDescriptor* kazan_plugin_ep() {
    return &kazan_plugin_descriptor;
}