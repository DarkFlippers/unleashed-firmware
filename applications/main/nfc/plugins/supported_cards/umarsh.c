/*
 * Parser for Umarsh card (Russia).
 *
 * Copyright 2023 Leptoptilos <leptoptilos@icloud.com>
 * Thanks https://github.com/krolchonok for the provided dumps and their analysis
 *
 * Note: All meaningful data is stored in sectors 0, 8 and 12, reading data 
 * from which is possible only with the B key. The key B for these sectors 
 * is unique for each card. To get it, you should use a nested attack.
 * More info about Umarsh cards: https://github.com/metrodroid/metrodroid/wiki/Umarsh
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

#include <datetime/datetime.h>

#define TAG "Umarsh"

bool parse_datetime(uint16_t date, DateTime* result) {
    result->year = 2000 + (date >> 9);
    result->month = date >> 5 & 0x0F;
    result->day = date & 0x1F;
    return date != 0;
}

static bool umarsh_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify card type
        if(data->type != MfClassicType1k) break;

        const uint8_t ticket_sector = 8;

        const uint8_t ticket_sector_start_block_number =
            mf_classic_get_first_block_num_of_sector(ticket_sector);

        // Validate specific for Umarsh ticket sector header
        const uint8_t* block_start_ptr = &data->block[ticket_sector_start_block_number].data[0];

        const uint32_t header_part_0 = bit_lib_bytes_to_num_be(block_start_ptr, 4);
        const uint32_t header_part_1 = bit_lib_bytes_to_num_be(block_start_ptr + 4, 4);
        if((header_part_0 + header_part_1) != 0xFFFFFFFF) break;

        // Data parsing from block 1
        block_start_ptr = &data->block[ticket_sector_start_block_number + 1].data[0];
        const uint16_t expiry_date = bit_lib_bytes_to_num_be(block_start_ptr + 1, 2);
        const uint8_t region_number = (((block_start_ptr[8] >> 5) & 0x07) << 4) |
                                      (block_start_ptr[12] & 0x0F);
        const uint8_t refill_counter = bit_lib_bytes_to_num_be(block_start_ptr + 7, 1);
        const uint32_t card_number = bit_lib_bytes_to_num_be(block_start_ptr + 8, 4) & 0x3FFFFFFF;

        if(card_number == 0) break;

        // Data parsing from block 2
        block_start_ptr = &data->block[ticket_sector_start_block_number + 2].data[0];
        const uint16_t valid_to = bit_lib_bytes_to_num_be(block_start_ptr, 2);
        const uint32_t terminal_number = bit_lib_bytes_to_num_be(block_start_ptr + 3, 3);
        const uint16_t last_refill_date = bit_lib_bytes_to_num_be(block_start_ptr + 6, 2);
        const uint16_t balance_rub = (bit_lib_bytes_to_num_be(block_start_ptr + 8, 2)) & 0x7FFF;
        const uint8_t balance_kop = bit_lib_bytes_to_num_be(block_start_ptr + 10, 1) & 0x7F;

        DateTime expiry_datetime;
        bool is_expiry_datetime_valid = parse_datetime(expiry_date, &expiry_datetime);

        DateTime valid_to_datetime;
        bool is_valid_to_datetime_valid = parse_datetime(valid_to, &valid_to_datetime);

        DateTime last_refill_datetime;
        bool is_last_refill_datetime_valid =
            parse_datetime(last_refill_date, &last_refill_datetime);

        furi_string_cat_printf(
            parsed_data,
            "\e#Umarsh\nCard number: %lu\nRegion: %02u\nTerminal number: %lu\nRefill counter: %u\nBalance: %u.%02u RUR",
            card_number,
            region_number,
            terminal_number,
            refill_counter,
            balance_rub,
            balance_kop);

        if(is_expiry_datetime_valid)
            furi_string_cat_printf(
                parsed_data,
                "\nExpires: %02u.%02u.%u",
                expiry_datetime.day,
                expiry_datetime.month,
                expiry_datetime.year);
        if(is_valid_to_datetime_valid)
            furi_string_cat_printf(
                parsed_data,
                "\nValid to: %02u.%02u.%u",
                valid_to_datetime.day,
                valid_to_datetime.month,
                valid_to_datetime.year);
        if(is_last_refill_datetime_valid)
            furi_string_cat_printf(
                parsed_data,
                "\nLast refill: %02u.%02u.%u",
                last_refill_datetime.day,
                last_refill_datetime.month,
                last_refill_datetime.year);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin umarsh_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = NULL,
    .read = NULL,
    .parse = umarsh_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor umarsh_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &umarsh_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* umarsh_plugin_ep(void) {
    return &umarsh_plugin_descriptor;
}
