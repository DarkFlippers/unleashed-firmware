/*
 * opal.c - Parser for Opal card (Sydney, Australia).
 *
 * Copyright 2023 Michael Farrell <micolous+git@gmail.com>
 *
 * This will only read "standard" MIFARE DESFire-based Opal cards. Free travel
 * cards (including School Opal cards, veteran, vision-impaired persons and
 * TfNSW employees' cards) and single-trip tickets are MIFARE Ultralight C
 * cards and not supported.
 *
 * Reference: https://github.com/metrodroid/metrodroid/wiki/Opal
 *
 * Note: The card values are all little-endian (like Flipper), but the above
 * reference was originally written based on Java APIs, which are big-endian.
 * This implementation presumes a little-endian system.
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
#include "nfc_supported_card.h"
#include "opal.h"

#include <applications/services/locale/locale.h>
#include <gui/modules/widget.h>
#include <nfc_worker_i.h>

#include <furi_hal.h>

static const uint8_t opal_aid[3] = {0x31, 0x45, 0x53};
static const char* opal_modes[5] =
    {"Rail / Metro", "Ferry / Light Rail", "Bus", "Unknown mode", "Manly Ferry"};
static const char* opal_usages[14] = {
    "New / Unused",
    "Tap on: new journey",
    "Tap on: transfer from same mode",
    "Tap on: transfer from other mode",
    "", // Manly Ferry: new journey
    "", // Manly Ferry: transfer from ferry
    "", // Manly Ferry: transfer from other
    "Tap off: distance fare",
    "Tap off: flat fare",
    "Automated tap off: failed to tap off",
    "Tap off: end of trip without start",
    "Tap off: reversal",
    "Tap on: rejected",
    "Unknown usage",
};

// Opal file 0x7 structure. Assumes a little-endian CPU.
typedef struct __attribute__((__packed__)) {
    uint32_t serial : 32;
    uint8_t check_digit : 4;
    bool blocked : 1;
    uint16_t txn_number : 16;
    int32_t balance : 21;
    uint16_t days : 15;
    uint16_t minutes : 11;
    uint8_t mode : 3;
    uint16_t usage : 4;
    bool auto_topup : 1;
    uint8_t weekly_journeys : 4;
    uint16_t checksum : 16;
} OpalFile;

static_assert(sizeof(OpalFile) == 16);

// Converts an Opal timestamp to FuriHalRtcDateTime.
//
// Opal measures days since 1980-01-01 and minutes since midnight, and presumes
// all days are 1440 minutes.
void opal_date_time_to_furi(uint16_t days, uint16_t minutes, FuriHalRtcDateTime* out) {
    if(!out) return;
    uint16_t diy;
    out->year = 1980;
    out->month = 1;
    // 1980-01-01 is a Tuesday
    out->weekday = ((days + 1) % 7) + 1;
    out->hour = minutes / 60;
    out->minute = minutes % 60;
    out->second = 0;

    // What year is it?
    for(;;) {
        diy = furi_hal_rtc_get_days_per_year(out->year);
        if(days < diy) break;
        days -= diy;
        out->year++;
    }

    // 1-index the day of the year
    days++;
    // What month is it?
    bool is_leap = furi_hal_rtc_is_leap_year(out->year);

    for(;;) {
        uint8_t dim = furi_hal_rtc_get_days_per_month(is_leap, out->month);
        if(days <= dim) break;
        days -= dim;
        out->month++;
    }

    out->day = days;
}

bool opal_parser_parse(NfcDeviceData* dev_data) {
    if(dev_data->protocol != NfcDeviceProtocolMifareDesfire) {
        return false;
    }

    MifareDesfireApplication* app = mf_df_get_application(&dev_data->mf_df_data, &opal_aid);
    if(app == NULL) {
        return false;
    }
    MifareDesfireFile* f = mf_df_get_file(app, 0x07);
    if(f == NULL || f->type != MifareDesfireFileTypeStandard || f->settings.data.size != 16 ||
       !f->contents) {
        return false;
    }

    OpalFile* o = (OpalFile*)f->contents;

    uint8_t serial2 = o->serial / 10000000;
    uint16_t serial3 = (o->serial / 1000) % 10000;
    uint16_t serial4 = (o->serial % 1000);

    if(o->check_digit > 9) {
        return false;
    }

    char* sign = "";
    if(o->balance < 0) {
        // Negative balance. Make this a positive value again and record the
        // sign separately, because then we can handle balances of -99..-1
        // cents, as the "dollars" division below would result in a positive
        // zero value.
        o->balance = abs(o->balance); //-V1081
        sign = "-";
    }
    uint8_t cents = o->balance % 100;
    int32_t dollars = o->balance / 100;

    FuriHalRtcDateTime timestamp;
    opal_date_time_to_furi(o->days, o->minutes, &timestamp);

    if(o->mode >= 3) {
        // 3..7 are "reserved", but we use 4 to indicate the Manly Ferry.
        o->mode = 3;
    }

    if(o->usage >= 4 && o->usage <= 6) {
        // Usages 4..6 associated with the Manly Ferry, which correspond to
        // usages 1..3 for other modes.
        o->usage -= 3;
        o->mode = 4;
    }

    const char* mode_str = (o->mode <= 4 ? opal_modes[o->mode] : opal_modes[3]); //-V547
    const char* usage_str = (o->usage <= 12 ? opal_usages[o->usage] : opal_usages[13]);

    furi_string_printf(
        dev_data->parsed_data,
        "\e#Opal: $%s%ld.%02hu\n3085 22%02hhu %04hu %03hu%01hhu\n%s, %s\n",
        sign,
        dollars,
        cents,
        serial2,
        serial3,
        serial4,
        o->check_digit,
        mode_str,
        usage_str);
    FuriString* timestamp_str = furi_string_alloc();
    locale_format_date(timestamp_str, &timestamp, locale_get_date_format(), "-");
    furi_string_cat(dev_data->parsed_data, timestamp_str);
    furi_string_cat_str(dev_data->parsed_data, " at ");

    locale_format_time(timestamp_str, &timestamp, locale_get_time_format(), false);
    furi_string_cat(dev_data->parsed_data, timestamp_str);

    furi_string_free(timestamp_str);
    furi_string_cat_printf(
        dev_data->parsed_data,
        "\nWeekly journeys: %hhu, Txn #%hu\n",
        o->weekly_journeys,
        o->txn_number);

    if(o->auto_topup) {
        furi_string_cat_str(dev_data->parsed_data, "Auto-topup enabled\n");
    }
    if(o->blocked) {
        furi_string_cat_str(dev_data->parsed_data, "Card blocked\n");
    }
    return true;
}
