/*
 * Parser for Zolotaya Korona Online card (Russia).
 * Tariffs research by DNZ1393
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

#include "protocols/mf_classic/mf_classic.h"

#include <bit_lib.h>

#define TAG "Zolotaya Korona Online"

#define TRIP_SECTOR_NUM (4)
#define INFO_SECTOR_NUM (15)

bool parse_online_card_tariff(uint16_t tariff_num, FuriString* tariff_name) {
    bool tariff_parsed = false;

    switch(tariff_num) {
    case 0x0100:
        furi_string_set_str(tariff_name, "Standart (online)");
        tariff_parsed = true;
        break;
    case 0x0101:
    case 0x0121:
        furi_string_set_str(tariff_name, "Standart (airtag)");
        tariff_parsed = true;
        break;
    case 0x0401:
        furi_string_set_str(tariff_name, "Student (50%% discount)");
        tariff_parsed = true;
        break;
    case 0x0402:
        furi_string_set_str(tariff_name, "Student (travel)");
        tariff_parsed = true;
        break;
    case 0x0002:
        furi_string_set_str(tariff_name, "School (50%% discount)");
        tariff_parsed = true;
        break;
    case 0x0505:
        furi_string_set_str(tariff_name, "Social (large families)");
        tariff_parsed = true;
        break;
    case 0x0528:
        furi_string_set_str(tariff_name, "Social (handicapped)");
        tariff_parsed = true;
        break;
    default:
        furi_string_set_str(tariff_name, "Unknown");
        tariff_parsed = false;
        break;
    }

    return tariff_parsed;
}

static bool zolotaya_korona_online_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify info sector data (card number prefix)
        const uint8_t start_trip_block_number =
            mf_classic_get_first_block_num_of_sector(TRIP_SECTOR_NUM);
        const uint8_t start_info_block_number =
            mf_classic_get_first_block_num_of_sector(INFO_SECTOR_NUM);
        const uint8_t* block_start_ptr = &data->block[start_info_block_number].data[3];

        // Validate card number
        bool is_bcd;
        const uint16_t card_number_prefix = bit_lib_bytes_to_num_bcd(block_start_ptr, 2, &is_bcd);
        if(!is_bcd) break;
        if(card_number_prefix != 9643) break;
        const uint64_t card_number_postfix =
            bit_lib_bytes_to_num_bcd(block_start_ptr + 2, 8, &is_bcd) / 10;
        if(!is_bcd) break;

        // Parse data
        FuriString* tariff_name = furi_string_alloc();

        block_start_ptr = &data->block[start_info_block_number].data[1];
        const uint16_t tariff = bit_lib_bytes_to_num_be(block_start_ptr, 2);
        parse_online_card_tariff(tariff, tariff_name);

        block_start_ptr = &data->block[start_trip_block_number].data[0];
        const uint8_t region_number = bit_lib_bytes_to_num_be(block_start_ptr, 1);

        furi_string_cat_printf(
            parsed_data,
            "\e#Zolotaya korona\nCard number: %u%015llu\nTariff: %02X.%02X: %s\nRegion: %u\n",
            card_number_prefix,
            card_number_postfix,
            tariff / 256,
            tariff % 256,
            furi_string_get_cstr(tariff_name),
            region_number);

        furi_string_free(tariff_name);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin zolotaya_korona_online_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = NULL,
    .read = NULL,
    .parse = zolotaya_korona_online_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor zolotaya_korona_online_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &zolotaya_korona_online_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* zolotaya_korona_online_plugin_ep(void) {
    return &zolotaya_korona_online_plugin_descriptor;
}
