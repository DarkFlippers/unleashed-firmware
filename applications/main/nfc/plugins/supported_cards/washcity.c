/*
 * Parser for WashCity MarkItaly Card (Europe).
 *
 * Copyright 2023 Filipe Polido (YaBaPT) <polido@gmail.com>
 * 
 * Based on MetroMoney by Leptoptilos <leptoptilos@icloud.com>
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

#include "protocols/mf_classic/mf_classic.h"
#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "WashCity"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static const MfClassicKeyPair washcity_1k_keys[] = {
    {.a = 0xA0A1A2A3A4A5, .b = 0x010155010100}, // Sector 00
    {.a = 0xC78A3D0E1BCD, .b = 0xFFFFFFFFFFFF}, // Sector 01
    {.a = 0xC78A3D0E0000, .b = 0xFFFFFFFFFFFF}, // Sector 02
    {.a = 0xC78A3D0E0000, .b = 0xFFFFFFFFFFFF}, // Sector 03
    {.a = 0xC78A3D0E0000, .b = 0xFFFFFFFFFFFF}, // Sector 04
    {.a = 0xC78A3D0E0000, .b = 0xFFFFFFFFFFFF}, // Sector 05
    {.a = 0xC78A3D0E0000, .b = 0xFFFFFFFFFFFF}, // Sector 06
    {.a = 0xC78A3D0E0000, .b = 0xFFFFFFFFFFFF}, // Sector 07
    {.a = 0xC78A3D0E0000, .b = 0xFFFFFFFFFFFF}, // Sector 08
    {.a = 0x010155010100, .b = 0xFFFFFFFFFFFF}, // Sector 09
    {.a = 0x010155010100, .b = 0xFFFFFFFFFFFF}, // Sector 10
    {.a = 0x010155010100, .b = 0xFFFFFFFFFFFF}, // Sector 11
    {.a = 0x010155010100, .b = 0xFFFFFFFFFFFF}, // Sector 12
    {.a = 0x010155010100, .b = 0xFFFFFFFFFFFF}, // Sector 13
    {.a = 0x010155010100, .b = 0xFFFFFFFFFFFF}, // Sector 14
    {.a = 0x010155010100, .b = 0xFFFFFFFFFFFF}, // Sector 15
};

static bool washcity_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector_number = 1;
        const uint8_t verify_block_number =
            mf_classic_get_first_block_num_of_sector(verify_sector_number);
        FURI_LOG_D(TAG, "Verifying sector %u", verify_sector_number);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(
            washcity_1k_keys[verify_sector_number].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error = mf_classic_poller_sync_auth(
            nfc, verify_block_number, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", verify_block_number, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool washcity_read(Nfc* nfc, NfcDevice* device) {
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
            bit_lib_num_to_bytes_be(
                washcity_1k_keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(
                washcity_1k_keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
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

static bool washcity_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify key
        const uint8_t ticket_sector_number = 1;
        const uint8_t ticket_block_number = 0;

        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, ticket_sector_number);

        const uint64_t key =
            bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        if(key != washcity_1k_keys[ticket_sector_number].a) break;

        // Parse data
        const uint8_t start_block_num =
            mf_classic_get_first_block_num_of_sector(ticket_sector_number);

        const uint8_t* block_start_ptr =
            &data->block[start_block_num + ticket_block_number].data[0];

        uint32_t balance = bit_lib_bytes_to_num_be(block_start_ptr + 2, 2);

        uint32_t balance_eur = balance / 100;
        uint8_t balance_cents = balance % 100;

        size_t uid_len = 0;
        const uint8_t* uid = mf_classic_get_uid(data, &uid_len);

        // Card Number is printed in HEX (equal to UID)
        uint64_t card_number = bit_lib_bytes_to_num_be(uid, uid_len);

        furi_string_printf(
            parsed_data,
            "\e#WashCity\nCard number: %0*llX\nBalance: %lu.%02u EUR",
            uid_len * 2,
            card_number,
            balance_eur,
            balance_cents);
        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin washcity_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = washcity_verify,
    .read = washcity_read,
    .parse = washcity_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor washcity_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &washcity_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* washcity_plugin_ep(void) {
    return &washcity_plugin_descriptor;
}
