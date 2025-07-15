/*
 * Parser for Zolotaya Korona card (Russia).
 *
 * Copyright 2023 Leptoptilos <leptoptilos@icloud.com>
 *
 * More info about Zolotaya Korona cards: https://github.com/metrodroid/metrodroid/wiki/Zolotaya-Korona
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
#include <locale/locale.h>
#include <furi_hal_rtc.h>

#define TAG "Zolotaya Korona"

#define TRIP_SECTOR_NUM  (4)
#define PURSE_SECTOR_NUM (6)
#define INFO_SECTOR_NUM  (15)

// Sector 15 data. Byte [11] contains the mistake. If byte [11] was 0xEF, bytes [1-18] means "ЗАО Золотая Корона"
static const uint8_t info_sector_signature[] = {0xE2, 0x87, 0x80, 0x8E, 0x20, 0x87, 0xAE,
                                                0xAB, 0xAE, 0xF2, 0xA0, 0xEF, 0x20, 0x8A,
                                                0xAE, 0xE0, 0xAE, 0xAD, 0xA0, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00};

static bool zolotaya_korona_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // Verify info sector data
        const uint8_t start_info_block_number =
            mf_classic_get_first_block_num_of_sector(INFO_SECTOR_NUM);
        const uint8_t* block_start_ptr = &data->block[start_info_block_number].data[0];

        bool verified = true;
        for(uint8_t i = 0; i < sizeof(info_sector_signature); i++) {
            if(i == 16) {
                block_start_ptr = &data->block[start_info_block_number + 1].data[0];
            }
            if(block_start_ptr[i % 16] != info_sector_signature[i]) {
                verified = false;
                break;
            }
        }

        if(!verified) break;

        // Parse data

        // INFO SECTOR
        // block 1
        const uint8_t region_number = bit_lib_bytes_to_num_bcd(block_start_ptr + 10, 1, &verified);

        // block 2
        block_start_ptr = &data->block[start_info_block_number + 2].data[4];
        const uint16_t card_number_prefix =
            bit_lib_bytes_to_num_bcd(block_start_ptr, 2, &verified);
        const uint64_t card_number_postfix =
            bit_lib_bytes_to_num_bcd(block_start_ptr + 2, 8, &verified) / 10;

        // TRIP SECTOR
        const uint8_t start_trip_block_number =
            mf_classic_get_first_block_num_of_sector(TRIP_SECTOR_NUM);
        // block 0
        block_start_ptr = &data->block[start_trip_block_number].data[7];

        const uint8_t status = block_start_ptr[0] % 16;
        const uint16_t sequence_number = bit_lib_bytes_to_num_be(block_start_ptr + 1, 2);
        const uint8_t discount_code = bit_lib_bytes_to_num_be(block_start_ptr + 3, 1);

        // block 1: refill block
        block_start_ptr = &data->block[start_trip_block_number + 1].data[1];

        const uint16_t refill_machine_id = bit_lib_bytes_to_num_le(block_start_ptr, 2);
        const uint32_t last_refill_timestamp = bit_lib_bytes_to_num_le(block_start_ptr + 2, 4);
        const uint32_t last_refill_amount = bit_lib_bytes_to_num_le(block_start_ptr + 6, 4);
        const uint32_t last_refill_amount_rub = last_refill_amount / 100;
        const uint8_t last_refill_amount_kop = last_refill_amount % 100;
        const uint16_t refill_counter = bit_lib_bytes_to_num_le(block_start_ptr + 10, 2);

        DateTime last_refill_datetime = {0};
        datetime_timestamp_to_datetime(last_refill_timestamp, &last_refill_datetime);

        // block 2: trip block
        block_start_ptr = &data->block[start_trip_block_number + 2].data[0];
        const char validator_first_letter = bit_lib_bytes_to_num_le(block_start_ptr + 1, 1);
        const uint32_t validator_id = bit_lib_bytes_to_num_bcd(block_start_ptr + 2, 3, &verified);
        const uint32_t last_trip_timestamp = bit_lib_bytes_to_num_le(block_start_ptr + 6, 4);
        const uint8_t track_number = bit_lib_bytes_to_num_le(block_start_ptr + 10, 1);
        const uint32_t prev_balance = bit_lib_bytes_to_num_le(block_start_ptr + 11, 4);
        const uint32_t prev_balance_rub = prev_balance / 100;
        const uint8_t prev_balance_kop = prev_balance % 100;

        DateTime last_trip_datetime = {0};
        datetime_timestamp_to_datetime(last_trip_timestamp, &last_trip_datetime);

        // PARSE DATA FROM PURSE SECTOR
        const uint8_t start_purse_block_number =
            mf_classic_get_first_block_num_of_sector(PURSE_SECTOR_NUM);
        block_start_ptr = &data->block[start_purse_block_number].data[0];

        // block 0
        const uint32_t balance = bit_lib_bytes_to_num_le(block_start_ptr, 4);

        uint32_t balance_rub = balance / 100;
        uint8_t balance_kop = balance % 100;

        LocaleDateFormat date_format = locale_get_date_format();
        const char* separator = (date_format == LocaleDateFormatDMY) ? "." : "/";

        FuriString* last_refill_date_str = furi_string_alloc();
        locale_format_date(last_refill_date_str, &last_refill_datetime, date_format, separator);

        FuriString* last_refill_time_str = furi_string_alloc();
        locale_format_time(
            last_refill_time_str, &last_refill_datetime, locale_get_time_format(), false);

        FuriString* last_trip_date_str = furi_string_alloc();
        locale_format_date(last_trip_date_str, &last_trip_datetime, date_format, separator);

        FuriString* last_trip_time_str = furi_string_alloc();
        locale_format_time(
            last_trip_time_str, &last_trip_datetime, locale_get_time_format(), false);

        furi_string_cat_printf(
            parsed_data,
            "\e#Zolotaya korona\nCard number: %u%015llu\nRegion: %u\nBalance: %lu.%02u RUR\nPrev. balance: %lu.%02u RUR",
            card_number_prefix,
            card_number_postfix,
            region_number,
            balance_rub,
            balance_kop,
            prev_balance_rub,
            prev_balance_kop);

        furi_string_cat_printf(
            parsed_data,
            "\nLast refill amount: %lu.%02u RUR\nRefill counter: %u\nLast refill: %s at %s\nRefill machine id: %u",
            last_refill_amount_rub,
            last_refill_amount_kop,
            refill_counter,
            furi_string_get_cstr(last_refill_date_str),
            furi_string_get_cstr(last_refill_time_str),
            refill_machine_id);

        furi_string_cat_printf(
            parsed_data,
            "\nLast trip: %s at %s\nTrack number: %u\nValidator: %c%06lu",
            furi_string_get_cstr(last_trip_date_str),
            furi_string_get_cstr(last_trip_time_str),
            track_number,
            validator_first_letter,
            validator_id);

        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            furi_string_cat_printf(
                parsed_data,
                "\nStatus: %u\nSequence num: %u\nDiscount code: %u",
                status,
                sequence_number,
                discount_code);
        }

        furi_string_free(last_refill_date_str);
        furi_string_free(last_refill_time_str);

        furi_string_free(last_trip_date_str);
        furi_string_free(last_trip_time_str);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin zolotaya_korona_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = NULL,
    .read = NULL,
    .parse = zolotaya_korona_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor zolotaya_korona_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &zolotaya_korona_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* zolotaya_korona_plugin_ep(void) {
    return &zolotaya_korona_plugin_descriptor;
}
