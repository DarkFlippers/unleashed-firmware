/*
 * clipper.c - Parser for Clipper cards (San Francisco, California).
 *
 * Based on research, some of which dates to 2007!
 *
 * Copyright 2024 Jeremy Cooper <jeremy.gthb@baymoo.org>
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

#include <flipper_application/flipper_application.h>
#include <lib/nfc/protocols/mf_desfire/mf_desfire.h>
#include <lib/nfc/helpers/nfc_util.h>
#include <applications/services/locale/locale.h>
#include <furi_hal_rtc.h>
#include <inttypes.h>

//
// Table of application ids observed in the wild, and their sources.
//
static const struct {
    const MfDesfireApplicationId app;
    const char* type;
} clipper_types[] = {
    // Application advertised on classic, plastic cards.
    {.app = {.data = {0x90, 0x11, 0xf2}}, .type = "Card"},
    // Application advertised on a mobile device.
    {.app = {.data = {0x91, 0x11, 0xf2}}, .type = "Mobile Device"},
};
static const size_t kNumCardTypes = sizeof(clipper_types) / sizeof(clipper_types[0]);

struct IdMapping_struct {
    uint16_t id;
    const char* name;
};
typedef struct IdMapping_struct IdMapping;

#define COUNT(_array) sizeof(_array) / sizeof(_array[0])

//
// Known transportation agencies and their identifiers.
//
static const IdMapping agency_names[] = {
    {.id = 0x0001, .name = "AC Transit"},
    {.id = 0x0004, .name = "BART"},
    {.id = 0x0006, .name = "Caltrain"},
    {.id = 0x0008, .name = "CCTA"},
    {.id = 0x000b, .name = "GGT"},
    {.id = 0x000f, .name = "SamTrans"},
    {.id = 0x0011, .name = "VTA"},
    {.id = 0x0012, .name = "Muni"},
    {.id = 0x0019, .name = "GG Ferry"},
    {.id = 0x001b, .name = "SF Bay Ferry"},
};
static const size_t kNumAgencies = COUNT(agency_names);

//
// Known station names for various agencies.
//
static const IdMapping bart_zones[] = {
    {.id = 0x0001, .name = "Colma"},
    {.id = 0x0002, .name = "Daly City"},
    {.id = 0x0003, .name = "Balboa Park"},
    {.id = 0x0004, .name = "Glen Park"},
    {.id = 0x0005, .name = "24th St Mission"},
    {.id = 0x0006, .name = "16th St Mission"},
    {.id = 0x0007, .name = "Civic Center/UN Plaza"},
    {.id = 0x0008, .name = "Powell St"},
    {.id = 0x0009, .name = "Montgomery St"},
    {.id = 0x000a, .name = "Embarcadero"},
    {.id = 0x000b, .name = "West Oakland"},
    {.id = 0x000c, .name = "12th St/Oakland City Center"},
    {.id = 0x000d, .name = "19th St/Oakland"},
    {.id = 0x000e, .name = "MacArthur"},
    {.id = 0x000f, .name = "Rockridge"},
    {.id = 0x0010, .name = "Orinda"},
    {.id = 0x0011, .name = "Lafayette"},
    {.id = 0x0012, .name = "Walnut Creek"},
    {.id = 0x0013, .name = "Pleasant Hill/Contra Costa Centre"},
    {.id = 0x0014, .name = "Concord"},
    {.id = 0x0015, .name = "North Concord/Martinez"},
    {.id = 0x0016, .name = "Pittsburg/Bay Point"},
    {.id = 0x0017, .name = "Ashby"},
    {.id = 0x0018, .name = "Downtown Berkeley"},
    {.id = 0x0019, .name = "North Berkeley"},
    {.id = 0x001a, .name = "El Cerrito Plaza"},
    {.id = 0x001b, .name = "El Cerrito Del Norte"},
    {.id = 0x001c, .name = "Richmond"},
    {.id = 0x001d, .name = "Lake Merrit"},
    {.id = 0x001e, .name = "Fruitvale"},
    {.id = 0x001f, .name = "Coliseum"},
    {.id = 0x0021, .name = "San Leandro"},
    {.id = 0x0022, .name = "Hayward"},
    {.id = 0x0023, .name = "South Hayward"},
    {.id = 0x0024, .name = "Union City"},
    {.id = 0x0025, .name = "Fremont"},
    {.id = 0x0026, .name = "Daly City(2)?"},
    {.id = 0x0027, .name = "Dublin/Pleasanton"},
    {.id = 0x0028, .name = "South San Francisco"},
    {.id = 0x0029, .name = "San Bruno"},
    {.id = 0x002a, .name = "SFO Airport"},
    {.id = 0x002b, .name = "Millbrae"},
    {.id = 0x002c, .name = "West Dublin/Pleasanton"},
    {.id = 0x002d, .name = "OAK Airport"},
    {.id = 0x002e, .name = "Warm Springs/South Fremont"},
};
static const size_t kNumBARTZones = COUNT(bart_zones);

static const IdMapping muni_zones[] = {
    {.id = 0x0000, .name = "City Street"},
    {.id = 0x0005, .name = "Embarcadero"},
    {.id = 0x0006, .name = "Montgomery"},
    {.id = 0x0007, .name = "Powell"},
    {.id = 0x0008, .name = "Civic Center"},
    {.id = 0x0009, .name = "Van Ness"}, // Guessed
    {.id = 0x000a, .name = "Church"},
    {.id = 0x000b, .name = "Castro"},
    {.id = 0x000c, .name = "Forest Hill"}, // Guessed
    {.id = 0x000d, .name = "West Portal"},
};
static const size_t kNumMUNIZones = COUNT(muni_zones);

static const IdMapping actransit_zones[] = {
    {.id = 0x0000, .name = "City Street"},
};
static const size_t kNumACTransitZones = COUNT(actransit_zones);

//
// Full agency+zone mapping.
//
static const struct {
    uint16_t agency_id;
    const IdMapping* zone_map;
    size_t zone_count;
} agency_zone_map[] = {
    {.agency_id = 0x0001, .zone_map = actransit_zones, .zone_count = kNumACTransitZones},
    {.agency_id = 0x0004, .zone_map = bart_zones, .zone_count = kNumBARTZones},
    {.agency_id = 0x0012, .zone_map = muni_zones, .zone_count = kNumMUNIZones}};
static const size_t kNumAgencyZoneMaps = COUNT(agency_zone_map);

// File ids of important files on the card.
static const MfDesfireFileId clipper_ecash_file_id = 2;
static const MfDesfireFileId clipper_histidx_file_id = 6;
static const MfDesfireFileId clipper_identity_file_id = 8;
static const MfDesfireFileId clipper_history_file_id = 14;

struct ClipperCardInfo_struct {
    uint32_t serial_number;
    uint16_t counter;
    uint16_t last_txn_id;
    uint32_t last_updated_tm_1900;
    uint16_t last_terminal_id;
    int16_t balance_cents;
};
typedef struct ClipperCardInfo_struct ClipperCardInfo;

// Forward declarations for helper functions.
static void furi_string_cat_timestamp(
    FuriString* str,
    const char* date_hdr,
    const char* time_hdr,
    uint32_t tmst_1900);
static void epoch_1900_datetime_to_furi(uint32_t seconds, FuriHalRtcDateTime* out);
static bool get_file_contents(
    const MfDesfireApplication* app,
    const MfDesfireFileId* id,
    MfDesfireFileType type,
    size_t min_size,
    const uint8_t** out);
static bool decode_id_file(const uint8_t* ef8_data, ClipperCardInfo* info);
static bool decode_cash_file(const uint8_t* ef2_data, ClipperCardInfo* info);
static bool get_map_item(uint16_t id, const IdMapping* map, size_t sz, const char** out);
static bool get_agency_zone_name(uint16_t agency_id, uint16_t zone_id, const char** out);
static void
    decode_usd(int16_t amount_cents, bool* out_is_negative, int16_t* out_usd, uint16_t* out_cents);
static bool dump_ride_history(
    const uint8_t* index_file,
    const uint8_t* history_file,
    size_t len,
    FuriString* parsed_data);
static bool dump_ride_event(const uint8_t* record, FuriString* parsed_data);

// Unmarshal a 32-bit integer, big endian, unsigned
static inline uint32_t get_u32be(const uint8_t* field) {
    return nfc_util_bytes2num(field, 4);
}

// Unmarshal a 16-bit integer, big endian, unsigned
static uint16_t get_u16be(const uint8_t* field) {
    return nfc_util_bytes2num(field, 2);
}

// Unmarshal a 16-bit integer, big endian, signed, two's-complement
static int16_t get_i16be(const uint8_t* field) {
    uint16_t raw = get_u16be(field);
    if(raw > 0x7fff)
        return -((uint32_t)0x10000 - raw);
    else
        return raw;
}

static bool clipper_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    bool parsed = false;

    do {
        const MfDesfireData* data = nfc_device_get_data(device, NfcProtocolMfDesfire);

        const MfDesfireApplication* app = NULL;
        const char* device_description = NULL;

        for(size_t i = 0; i < kNumCardTypes; i++) {
            app = mf_desfire_get_application(data, &clipper_types[i].app);
            device_description = clipper_types[i].type;
            if(app != NULL) break;
        }

        // If no matching application was found, abort this parser.
        if(app == NULL) break;

        ClipperCardInfo info;
        const uint8_t* id_data;
        if(!get_file_contents(
               app, &clipper_identity_file_id, MfDesfireFileTypeStandard, 5, &id_data))
            break;
        if(!decode_id_file(id_data, &info)) break;

        const uint8_t* cash_data;
        if(!get_file_contents(app, &clipper_ecash_file_id, MfDesfireFileTypeBackup, 32, &cash_data))
            break;
        if(!decode_cash_file(cash_data, &info)) break;

        int16_t balance_usd;
        uint16_t balance_cents;
        bool _balance_is_negative;
        decode_usd(info.balance_cents, &_balance_is_negative, &balance_usd, &balance_cents);

        furi_string_cat_printf(
            parsed_data,
            "\e#Clipper\n"
            "Serial: %" PRIu32 "\n"
            "Balance: $%d.%02u\n"
            "Type: %s\n"
            "\e#Last Update\n",
            info.serial_number,
            balance_usd,
            balance_cents,
            device_description);
        if(info.last_updated_tm_1900 != 0)
            furi_string_cat_timestamp(
                parsed_data, "Date: ", "\nTime: ", info.last_updated_tm_1900);
        else
            furi_string_cat_str(parsed_data, "Never");
        furi_string_cat_printf(
            parsed_data,
            "\nTerminal: 0x%04x\n"
            "Transaction Id: %u\n"
            "Counter: %u\n",
            info.last_terminal_id,
            info.last_txn_id,
            info.counter);

        const uint8_t *history_index, *history;

        if(!get_file_contents(
               app, &clipper_histidx_file_id, MfDesfireFileTypeBackup, 16, &history_index))
            break;
        if(!get_file_contents(
               app, &clipper_history_file_id, MfDesfireFileTypeStandard, 512, &history))
            break;

        if(!dump_ride_history(history_index, history, 512, parsed_data)) break;

        parsed = true;
    } while(false);

    return parsed;
}

static bool get_file_contents(
    const MfDesfireApplication* app,
    const MfDesfireFileId* id,
    MfDesfireFileType type,
    size_t min_size,
    const uint8_t** out) {
    const MfDesfireFileSettings* settings = mf_desfire_get_file_settings(app, id);
    if(settings == NULL) return false;
    if(settings->type != type) return false;

    const MfDesfireFileData* file_data = mf_desfire_get_file_data(app, id);
    if(file_data == NULL) return false;

    if(simple_array_get_count(file_data->data) < min_size) return false;

    *out = simple_array_cget_data(file_data->data);

    return true;
}

static bool decode_id_file(const uint8_t* ef8_data, ClipperCardInfo* info) {
    // Identity file (8)
    //
    // Byte view
    //
    //       0    1    2    3    4    5    6    7    8
    //       +----+----.----.----.----+----.----.----+
    // 0x00  | uk | card_id           | unknown      |
    //       +----+----.----.----.----+----.----.----+
    // 0x08  | unknown                               |
    //       +----.----.----.----.----.----.----.----+
    // 0x10    ...
    //
    //
    // Field          Datatype   Description
    // -----          --------   -----------
    // uk             ?8??       Unknown, 8-bit byte
    // card_id        U32BE      Card identifier
    //
    info->serial_number = nfc_util_bytes2num(&ef8_data[1], 4);
    return true;
}

static bool decode_cash_file(const uint8_t* ef2_data, ClipperCardInfo* info) {
    // ECash file (2)
    //
    // Byte view
    //
    //       0    1    2    3    4    5    6    7    8
    //       +----.----+----.----+----.----.----.----+
    // 0x00  |  unk00  | counter | timestamp_1900    |
    //       +----.----+----.----+----.----.----.----+
    // 0x08  | term_id |     unk01                   |
    //       +----.----+----.----+----.----.----.----+
    // 0x10  | txn_id  | balance |      unknown      |
    //       +----.----+----.----+----.----.----.----+
    // 0x18  |               unknown                 |
    //       +---------------------------------------+
    //
    // Field          Datatype Description
    // -----          -------- -----------
    // unk00          U8[2]     Unknown bytes
    // counter        U16BE     Unknown, appears to be a counter
    // timestamp_1900 U32BE     Timestamp of last transaction, in seconds
    //                          since 1900-01-01 GMT.
    // unk01          U8[6]     Unknown bytes
    // txn_id         U16BE     Id of last transaction.
    // balance        S16BE     Card cash balance, in cents.
    //                          Cards can obtain negative balances in this
    //                          system, so balances are signed integers.
    //                          Maximum card balance is therefore
    //                          $327.67.
    // unk02          U8[12]    Unknown bytes.
    //
    info->counter = get_u16be(&ef2_data[2]);
    info->last_updated_tm_1900 = get_u32be(&ef2_data[4]);
    info->last_terminal_id = get_u16be(&ef2_data[8]);
    info->last_txn_id = get_u16be(&ef2_data[0x10]);
    info->balance_cents = get_i16be(&ef2_data[0x12]);
    return true;
}

static bool dump_ride_history(
    const uint8_t* index_file,
    const uint8_t* history_file,
    size_t len,
    FuriString* parsed_data) {
    static const size_t kRideRecordSize = 0x20;

    for(size_t i = 0; i < 16; i++) {
        uint8_t record_num = index_file[i];
        if(record_num == 0xff) break;

        size_t record_offset = record_num * kRideRecordSize;

        if(record_offset + kRideRecordSize > len) break;

        const uint8_t* record = &history_file[record_offset];
        if(!dump_ride_event(record, parsed_data)) break;
    }

    return true;
}

static bool dump_ride_event(const uint8_t* record, FuriString* parsed_data) {
    // Ride record
    //
    //       0    1    2    3    4    5    6    7    8
    //       +----+----+----.----+----.----+----.----+
    // 0x00  |0x10| ?  | agency  | ?       | fare    |
    //       +----.----+----.----+----.----.----.----+
    // 0x08  | ?       | vehicle | time_on           |
    //       +----.----.----.----+----.----+----.----+
    // 0x10  | time_off          | zone_on | zone_off|
    //       +----+----.----.----.----+----+----+----+
    // 0x18  | ?  | ?                 | ?  | ?  | ?  |
    //       +----+----.----.----.----+----+----+----+
    //
    // Field          Datatype Description
    // -----          -------- -----------
    // agency         U16BE    Transportation agency identifier.
    //                         Known ids:
    //                         1  == AC Transit
    //                         4  == BART
    //                         18 == SF MUNI
    // fare           I16BE    Fare deducted, in cents.
    // vehicle        U16BE    Vehicle id (0 == not provided)
    // time_on        U32BE    Boarding time, in seconds since 1900-01-01 GMT.
    // time_off       U32BE    Off-boarding time, if present, in seconds
    //                         since 1900-01-01 GMT. Set to zero if no offboard
    //                         has been recorded.
    // zone_on        U16BE    Id of boarding zone or station. Agency-specific.
    // zone_off       U16BE    Id of offboarding zone or station. Agency-
    //                         specific.
    if(record[0] != 0x10) return false;

    uint16_t agency_id = get_u16be(&record[2]);
    if(agency_id == 0)
        // Likely empty record. Skip.
        return false;
    const char* agency_name;
    bool ok = get_map_item(agency_id, agency_names, kNumAgencies, &agency_name);
    if(!ok) agency_name = "Unknown";

    uint16_t vehicle_id = get_u16be(&record[0x0a]);

    int16_t fare_raw_cents = get_i16be(&record[6]);
    bool _fare_is_negative;
    int16_t fare_usd;
    uint16_t fare_cents;
    decode_usd(fare_raw_cents, &_fare_is_negative, &fare_usd, &fare_cents);

    uint32_t time_on_raw = get_u32be(&record[0x0c]);
    uint32_t time_off_raw = get_u32be(&record[0x10]);
    uint16_t zone_id_on = get_u16be(&record[0x14]);
    uint16_t zone_id_off = get_u16be(&record[0x16]);

    const char *zone_on, *zone_off;
    if(!get_agency_zone_name(agency_id, zone_id_on, &zone_on)) {
        zone_on = "Unknown";
    }
    if(!get_agency_zone_name(agency_id, zone_id_off, &zone_off)) {
        zone_off = "Unknown";
    }

    furi_string_cat_str(parsed_data, "\e#Ride Record\n");
    furi_string_cat_timestamp(parsed_data, "Date: ", "\nTime: ", time_on_raw);
    furi_string_cat_printf(
        parsed_data,
        "\n"
        "Fare: $%d.%02u\n"
        "Agency: %s (%04x)\n"
        "On: %s (%04x)\n",
        fare_usd,
        fare_cents,
        agency_name,
        agency_id,
        zone_on,
        zone_id_on);
    if(vehicle_id != 0) {
        furi_string_cat_printf(parsed_data, "Vehicle id: %d\n", vehicle_id);
    }
    if(time_off_raw != 0) {
        furi_string_cat_printf(parsed_data, "Off: %s (%04x)\n", zone_off, zone_id_off);
        furi_string_cat_timestamp(parsed_data, "Date Off: ", "\nTime Off: ", time_off_raw);
        furi_string_cat_str(parsed_data, "\n");
    }

    return true;
}

static bool get_map_item(uint16_t id, const IdMapping* map, size_t sz, const char** out) {
    for(size_t i = 0; i < sz; i++) {
        if(map[i].id == id) {
            *out = map[i].name;
            return true;
        }
    }

    return false;
}

static bool get_agency_zone_name(uint16_t agency_id, uint16_t zone_id, const char** out) {
    for(size_t i = 0; i < kNumAgencyZoneMaps; i++) {
        if(agency_zone_map[i].agency_id == agency_id) {
            return get_map_item(
                zone_id, agency_zone_map[i].zone_map, agency_zone_map[i].zone_count, out);
        }
    }

    return false;
}

// Split a balance/fare amount from raw cents to dollars and cents portion,
// automatically adjusting the cents portion so that it is always positive,
// for easier display.
static void
    decode_usd(int16_t amount_cents, bool* out_is_negative, int16_t* out_usd, uint16_t* out_cents) {
    *out_usd = amount_cents / 100;

    if(amount_cents >= 0) {
        *out_is_negative = false;
        *out_cents = amount_cents % 100;
    } else {
        *out_is_negative = true;
        *out_cents = (amount_cents * -1) % 100;
    }
}

// Decode a raw 1900-based timestamp and append a human-readable form to a
// FuriString.
static void furi_string_cat_timestamp(
    FuriString* str,
    const char* date_hdr,
    const char* time_hdr,
    uint32_t tmst_1900) {
    FuriHalRtcDateTime tm;

    epoch_1900_datetime_to_furi(tmst_1900, &tm);

    FuriString* date_str = furi_string_alloc();
    locale_format_date(date_str, &tm, locale_get_date_format(), "-");

    FuriString* time_str = furi_string_alloc();
    locale_format_time(time_str, &tm, locale_get_time_format(), true);

    furi_string_cat_printf(
        str,
        "%s%s%s%s (UTC)",
        date_hdr,
        furi_string_get_cstr(date_str),
        time_hdr,
        furi_string_get_cstr(time_str));

    furi_string_free(date_str);
    furi_string_free(time_str);
}

// Convert a "1900"-based timestamp to Furi time, assuming a UTC/GMT timezone.
static void epoch_1900_datetime_to_furi(uint32_t seconds, FuriHalRtcDateTime* out) {
    uint16_t year, month, day, hour, minute, second;

    // Calculate absolute number of days elapsed since the 1900 epoch
    // and save the residual for the time within the day.
    uint32_t absolute_days = seconds / 86400;
    uint32_t seconds_within_day = seconds % 86400;

    // Calculate day of the week.
    // January 1, 1900 was a Monday ("day of week" = 1)
    uint8_t dow = (absolute_days + 1) % 7;

    //
    // Compute the date by simply marching through time in as large chunks
    // as possible.
    //

    for(year = 1900;; year++) {
        uint16_t year_days = furi_hal_rtc_get_days_per_year(year);
        if(absolute_days >= year_days)
            absolute_days -= year_days;
        else
            break;
    }

    bool is_leap = furi_hal_rtc_is_leap_year(year);

    for(month = 1;; month++) {
        uint8_t days_in_month = furi_hal_rtc_get_days_per_month(is_leap, month);
        if(absolute_days >= days_in_month)
            absolute_days -= days_in_month;
        else
            break;
    }

    day = absolute_days + 1;
    hour = seconds_within_day / 3600;
    uint16_t sub_hour = seconds_within_day % 3600;
    minute = sub_hour / 60;
    second = sub_hour % 60;

    out->year = year;
    out->month = month;
    out->day = day;
    out->hour = hour;
    out->minute = minute;
    out->second = second;
    out->weekday = dow;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin clipper_plugin = {
    .protocol = NfcProtocolMfDesfire,
    .verify = NULL,
    .read = NULL,
    .parse = clipper_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor clipper_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &clipper_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* clipper_plugin_ep() {
    return &clipper_plugin_descriptor;
}
