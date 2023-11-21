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

#include "core/core_defines.h"
#include "nfc_supported_card_plugin.h"

#include "protocols/mf_classic/mf_classic.h"
#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <nfc/helpers/nfc_util.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <stdbool.h>
#include <stdint.h>
#include <furi_hal_rtc.h>

#define TAG "Umarsh"

static bool umarsh_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify card type
        if(data->type != MfClassicType1k) break;

        const uint32_t ticket_sector = 8;

        const uint8_t ticket_sector_start_block_number =
            mf_classic_get_first_block_num_of_sector(ticket_sector);

        //Validate specific for Umarsh ticket sector header
        const uint8_t* block_start_ptr = &data->block[ticket_sector_start_block_number].data[0];

        uint32_t header = block_start_ptr[0] << 24 | block_start_ptr[1] << 16 |
                          block_start_ptr[2] << 8 | block_start_ptr[3];
        if(header != 0xFFFFFF7F && header != 0xFEFFFF7F && header != 0xE3FFFF7F) break;

        // Data parsing from block 1
        block_start_ptr = &data->block[ticket_sector_start_block_number + 1].data[0];
        uint8_t region_number = (((block_start_ptr[8] >> 5) & 0x07) << 4) |
                                (block_start_ptr[12] & 0x0F);
        uint32_t card_number = (block_start_ptr[8] << 24 | block_start_ptr[9] << 16 |
                                block_start_ptr[10] << 8 | block_start_ptr[11]) &
                               0x3FFFFFFF;
        uint8_t refill_counter = (block_start_ptr[7]);

        if(card_number == 0) break;

        // Data parsing from block 2
        block_start_ptr = &data->block[ticket_sector_start_block_number + 2].data[0];
        uint16_t expiry_date = (block_start_ptr[0] << 8 | block_start_ptr[1]);
        uint32_t terminal_number =
            (block_start_ptr[3] << 16 | block_start_ptr[4] << 8 | block_start_ptr[5]);
        uint16_t last_refill_date = (block_start_ptr[6] << 8 | block_start_ptr[7]);
        uint16_t balance_rub = (block_start_ptr[8] << 8 | block_start_ptr[9]) & 0x7FFF;
        uint8_t balance_kop = block_start_ptr[10] & 0x7F;

        FuriHalRtcDateTime expiry_datetime;
        expiry_datetime.year = 2000 + (expiry_date >> 9);
        expiry_datetime.month = expiry_date >> 5 & 0x0F;
        expiry_datetime.day = expiry_date & 0x1F;

        FuriHalRtcDateTime last_refill_datetime;
        last_refill_datetime.year = 2000 + (last_refill_date >> 9);
        last_refill_datetime.month = last_refill_date >> 5 & 0x0F;
        last_refill_datetime.day = last_refill_date & 0x1F;

        furi_string_printf(
            parsed_data,
            "\e#Umarsh\nCard number: %lu\nRegion: %02u\nBalance: %u.%u RUR\nTerminal number: %lu\nRefill counter: %u\nLast refill: %02u.%02u.%u\nExpires: %02u.%02u.%u",
            card_number,
            region_number,
            balance_rub,
            balance_kop,
            terminal_number,
            refill_counter,
            last_refill_datetime.day,
            last_refill_datetime.month,
            last_refill_datetime.year,
            expiry_datetime.day,
            expiry_datetime.month,
            expiry_datetime.year);
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
const FlipperAppPluginDescriptor* umarsh_plugin_ep() {
    return &umarsh_plugin_descriptor;
}
