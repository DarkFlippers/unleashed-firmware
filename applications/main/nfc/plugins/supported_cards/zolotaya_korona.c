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
#include "core/core_defines.h"
#include "core/string.h"
#include "furi_hal_rtc.h"
#include "nfc_supported_card_plugin.h"

#include "protocols/mf_classic/mf_classic.h"
#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <nfc/helpers/nfc_util.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "Zolotaya Korona"

#define TRIP_SECTOR_NUM (4)
#define PURSE_SECTOR_NUM (6)
#define INFO_SECTOR_NUM (15)

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

// Sector 15 data. Byte [11] contains the mistake. If byte [11] was 0xEF, bytes [1-18] means "ЗАО Золотая Корона"
static const uint8_t info_sector_signature[] = {0xE2, 0x87, 0x80, 0x8E, 0x20, 0x87, 0xAE,
                                                0xAB, 0xAE, 0xF2, 0xA0, 0xEF, 0x20, 0x8A,
                                                0xAE, 0xE0, 0xAE, 0xAD, 0xA0, 0x00, 0x00,
                                                0x00, 0x00, 0x00, 0x00};

#define FURI_HAL_RTC_SECONDS_PER_MINUTE 60
#define FURI_HAL_RTC_SECONDS_PER_HOUR (FURI_HAL_RTC_SECONDS_PER_MINUTE * 60)
#define FURI_HAL_RTC_SECONDS_PER_DAY (FURI_HAL_RTC_SECONDS_PER_HOUR * 24)
#define FURI_HAL_RTC_EPOCH_START_YEAR 1970

void timestamp_to_datetime(uint32_t timestamp, FuriHalRtcDateTime* datetime) {
    uint32_t days = timestamp / FURI_HAL_RTC_SECONDS_PER_DAY;
    uint32_t seconds_in_day = timestamp % FURI_HAL_RTC_SECONDS_PER_DAY;

    datetime->year = FURI_HAL_RTC_EPOCH_START_YEAR;

    while(days >= furi_hal_rtc_get_days_per_year(datetime->year)) {
        days -= furi_hal_rtc_get_days_per_year(datetime->year);
        (datetime->year)++;
    }

    datetime->month = 1;
    while(days >= furi_hal_rtc_get_days_per_month(
                      furi_hal_rtc_is_leap_year(datetime->year), datetime->month)) {
        days -= furi_hal_rtc_get_days_per_month(
            furi_hal_rtc_is_leap_year(datetime->year), datetime->month);
        (datetime->month)++;
    }

    datetime->day = days + 1;
    datetime->hour = seconds_in_day / FURI_HAL_RTC_SECONDS_PER_HOUR;
    datetime->minute =
        (seconds_in_day % FURI_HAL_RTC_SECONDS_PER_HOUR) / FURI_HAL_RTC_SECONDS_PER_MINUTE;
    datetime->second = seconds_in_day % FURI_HAL_RTC_SECONDS_PER_MINUTE;
}

uint64_t bytes2num_bcd(const uint8_t* src, uint8_t len_bytes) {
    furi_assert(src);

    uint64_t res = 0;

    for(uint8_t i = 0; i < len_bytes; i++) {
        res *= 10;
        res += src[i] / 16;
        res *= 10;
        res += src[i] % 16;
    }

    return res;
}

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
        const uint8_t region_number = bytes2num_bcd(block_start_ptr + 10, 1);

        // block 2
        block_start_ptr = &data->block[start_info_block_number + 2].data[4];
        const uint64_t card_number =
            bytes2num_bcd(block_start_ptr, 9) * 10 + bytes2num_bcd(block_start_ptr + 9, 1) / 10;

        // TRIP SECTOR
        const uint8_t start_trip_block_number =
            mf_classic_get_first_block_num_of_sector(TRIP_SECTOR_NUM);
        // block 0
        block_start_ptr = &data->block[start_trip_block_number].data[7];

        const uint8_t status = block_start_ptr[0] % 16;
        const uint16_t sequence_number = nfc_util_bytes2num(block_start_ptr + 1, 2);
        const uint8_t discount_code = nfc_util_bytes2num(block_start_ptr + 3, 1);

        // block 1: refill block
        block_start_ptr = &data->block[start_trip_block_number + 1].data[1];

        const uint16_t refill_machine_id = nfc_util_bytes2num_little_endian(block_start_ptr, 2);
        const uint32_t last_refill_timestamp =
            nfc_util_bytes2num_little_endian(block_start_ptr + 2, 4);
        const uint32_t last_refill_amount =
            nfc_util_bytes2num_little_endian(block_start_ptr + 6, 4);
        const uint32_t last_refill_amount_rub = last_refill_amount / 100;
        const uint8_t last_refill_amount_kop = last_refill_amount % 100;
        const uint16_t refill_counter = nfc_util_bytes2num_little_endian(block_start_ptr + 10, 2);

        FuriHalRtcDateTime last_refill_datetime = {0};
        timestamp_to_datetime(last_refill_timestamp, &last_refill_datetime);

        // block 2: trip block
        block_start_ptr = &data->block[start_trip_block_number + 2].data[0];
        const char validator_first_letter =
            nfc_util_bytes2num_little_endian(block_start_ptr + 1, 1);
        const uint32_t validator_id = bytes2num_bcd(block_start_ptr + 2, 3);
        const uint32_t last_trip_timestamp =
            nfc_util_bytes2num_little_endian(block_start_ptr + 6, 4);
        const uint8_t track_number = nfc_util_bytes2num_little_endian(block_start_ptr + 10, 1);
        const uint32_t prev_balance = nfc_util_bytes2num_little_endian(block_start_ptr + 11, 4);
        const uint32_t prev_balance_rub = prev_balance / 100;
        const uint8_t prev_balance_kop = prev_balance % 100;

        FuriHalRtcDateTime last_trip_datetime = {0};
        timestamp_to_datetime(last_trip_timestamp, &last_trip_datetime);

        // PARSE DATA FROM PURSE SECTOR
        const uint8_t start_purse_block_number =
            mf_classic_get_first_block_num_of_sector(PURSE_SECTOR_NUM);
        block_start_ptr = &data->block[start_purse_block_number].data[0];

        // block 0
        uint32_t balance = nfc_util_bytes2num_little_endian(block_start_ptr, 4);

        uint32_t balance_rub = balance / 100;
        uint8_t balance_kop = balance % 100;

        furi_string_cat_printf(
            parsed_data,
            "\e#Zolotaya korona\nCard number: %llu\nRegion: %u\nBalance: %lu.%02u RUR\nPrev. balance: %lu.%02u RUR",
            card_number,
            region_number,
            balance_rub,
            balance_kop,
            prev_balance_rub,
            prev_balance_kop);

        furi_string_cat_printf(
            parsed_data,
            "\nLast refill amount: %lu.%02u RUR\nRefill counter: %u\nLast refill: %u.%02u.%02u %02u:%02u\nRefill machine id: %u",
            last_refill_amount_rub,
            last_refill_amount_kop,
            refill_counter,
            last_refill_datetime.day,
            last_refill_datetime.month,
            last_refill_datetime.year,
            last_refill_datetime.hour,
            last_refill_datetime.minute,
            refill_machine_id);

        furi_string_cat_printf(
            parsed_data,
            "\nLast trip: %u.%02u.%02u %02u:%02u\nTrack number: %u\nValidator: %c%06lu",
            last_trip_datetime.day,
            last_trip_datetime.month,
            last_trip_datetime.year,
            last_trip_datetime.hour,
            last_trip_datetime.minute,
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
const FlipperAppPluginDescriptor* zolotaya_korona_plugin_ep() {
    return &zolotaya_korona_plugin_descriptor;
}