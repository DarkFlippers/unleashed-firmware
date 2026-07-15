// Parser for CTA Ventra Ultralight cards
// Made by @hazardousvoltage
// Based on my own research, with...
// Credit to https://www.lenrek.net/experiments/compass-tickets/ & MetroDroid project for underlying info
//
// This parser can decode the paper single-use and single/multi-day paper passes using Ultralight EV1
// The plastic cards are DESFire and fully locked down, not much useful info extractable
// TODO:
// - Sort the duplicate/rare ticket types
// - Database of stop IDs for trains?  Buses there's just too damn many, but you can find them here:
//   https://data.cityofchicago.org/Transportation/CTA-Bus-Stops-kml/84eu-buny/about_data
// - Generalize to handle all known Cubic Nextfare Ultralight systems?  Anyone wants to send me specimen dumps, hit me up on Discord.

#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>
#include "datetime.h"
#include <furi_hal.h>

#define TAG "Ventra"

DateTime ventra_exp_date = {0}, ventra_validity_date = {0};
uint8_t ventra_high_seq = 0, ventra_cur_blk = 0, ventra_mins_active = 0;

uint32_t time_now() {
    return furi_hal_rtc_get_timestamp();
}

static DateTime dt_delta(DateTime dt, uint8_t delta_days) {
    // returns shifted DateTime, from initial DateTime and time offset in seconds
    DateTime dt_shifted = {0};
    datetime_timestamp_to_datetime(
        datetime_datetime_to_timestamp(&dt) - (uint64_t)delta_days * 86400, &dt_shifted);
    return dt_shifted;
}

/*
static long dt_diff(DateTime dta, DateTime dtb) {
    // returns difference in seconds between two DateTimes
    long diff;
    diff = datetime_datetime_to_timestamp(&dta) - datetime_datetime_to_timestamp(&dtb); 
    return diff;
}
*/

// Card is expired if:
// - Hard expiration date passed (90 days from purchase, encoded in product record)
// - Soft expiration date passed:
//   - For passes, n days after first use
//   - For tickets, 2 hours after first use
//   Calculating these is dumber than it needs to be, see xact record parser.
bool isExpired(void) {
    uint32_t ts_hard_exp = datetime_datetime_to_timestamp(&ventra_exp_date);
    uint32_t ts_soft_exp = datetime_datetime_to_timestamp(&ventra_validity_date);
    uint32_t ts_now = time_now();
    return (ts_now >= ts_hard_exp || ts_now > ts_soft_exp);
}

static FuriString* ventra_parse_xact(const MfUltralightData* data, uint8_t blk, bool is_pass) {
    FuriString* ventra_xact_str = furi_string_alloc();
    uint16_t ts = data->page[blk].data[0] | data->page[blk].data[1] << 8;
    uint8_t tran_type = ts & 0x1F;
    ts >>= 5;
    uint8_t day = data->page[blk].data[2];
    uint32_t work = data->page[blk + 1].data[0] | data->page[blk + 1].data[1] << 8 |
                    data->page[blk + 1].data[2] << 16;
    uint8_t seq = work & 0x7F;
    uint16_t exp = (work >> 7) & 0x7FF;
    uint8_t exp_day = data->page[blk + 2].data[0];
    uint16_t locus = data->page[blk + 2].data[1] | data->page[blk + 2].data[2] << 8;
    uint8_t line = data->page[blk + 2].data[3];

    // This computes the block timestamp, based on the card expiration date and delta from it
    DateTime dt = dt_delta(ventra_exp_date, day);
    dt.hour = (ts & 0x7FF) / 60;
    dt.minute = (ts & 0x7FF) % 60;

    // If sequence is 0, block isn't used yet (new card with only one active block, typically the first one.
    // Otherwise, the block with higher sequence is the latest transaction, and the other block is prior transaction.
    // Not necessarily in that order on the card.  We need the latest data to compute validity and pretty-print them
    // in reverse chrono.  So this mess sets some globals as to which block is current, computes the validity times, etc.
    if(seq == 0) {
        furi_string_printf(ventra_xact_str, "-- EMPTY --");
        return (ventra_xact_str);
    }
    if(seq > ventra_high_seq) {
        ventra_high_seq = seq;
        ventra_cur_blk = blk;
        ventra_mins_active = data->page[blk + 1].data[3];
        // Figure out the soft expiration.  For passes it's easy, the readers update the "exp" field in the transaction record.
        // Tickets, not so much, readers don't update "exp", but each xact record has "minutes since last tap" which is
        // updated and carried forward.  That, plus transaction timestamp, gives the expiration time.
        if(tran_type == 6) { // Furthermore, purchase transactions set bogus expiration dates
            if(is_pass) {
                ventra_validity_date = dt_delta(ventra_exp_date, exp_day);
                ventra_validity_date.hour = (exp & 0x7FF) / 60;
                ventra_validity_date.minute = (exp & 0x7FF) % 60;
            } else {
                uint32_t validity_ts = datetime_datetime_to_timestamp(&dt);
                validity_ts += (120 - ventra_mins_active) * 60;
                datetime_timestamp_to_datetime(validity_ts, &ventra_validity_date);
            }
        }
    }

    // Type 0 = Purchase, 1 = Train ride, 2 = Bus ride
    // TODO: Check PACE and see if it uses a different line code
    char linemap[3] = "PTB";
    char* xact_fmt = (line == 2) ? "%c %5d %04d-%02d-%02d %02d:%02d" :
                                   "%c %04X %04d-%02d-%02d %02d:%02d";
    // I like a nice concise display showing all the relevant infos without having to scroll...
    // Line   StopID   DateTime
    furi_string_printf(
        ventra_xact_str,
        xact_fmt,
        (line < 3) ? linemap[line] : '?',
        locus,
        dt.year,
        dt.month,
        dt.day,
        dt.hour,
        dt.minute);
    return (ventra_xact_str);
}

static bool ventra_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfUltralightData* data = nfc_device_get_data(device, NfcProtocolMfUltralight);

    bool parsed = false;

    do {
        // This test can probably be improved -- it matches every Ventra I've seen, but will also match others
        // in the same family.  Or maybe we just generalize this parser.
        if(data->page[4].data[0] != 0x0A || data->page[4].data[1] != 4 ||
           data->page[4].data[2] != 0 || data->page[6].data[0] != 0 ||
           data->page[6].data[1] != 0 || data->page[6].data[2] != 0) {
            FURI_LOG_D(TAG, "Not Ventra Ultralight");
            break;
        }

        // Parse the product record, display interesting data & extract info needed to parse transaction blocks
        // Had this in its own function, ended up just setting a bunch of shitty globals, so inlined it instead.
        FuriString* ventra_prod_str = furi_string_alloc();
        uint8_t otp = data->page[3].data[0];
        uint8_t prod_code = data->page[5].data[2];
        bool is_pass = false;
        switch(prod_code) {
        case 2:
        case 0x1F: // Only ever seen one of these, it parses like a Single
            furi_string_cat_printf(ventra_prod_str, "Single");
            break;
        case 3:
        case 0x3F:
            is_pass = true;
            furi_string_cat_printf(ventra_prod_str, "1-Day");
            break;
        case 4: // Last I checked, 3 day passes only available at airport TVMs & social service agencies
            is_pass = true;
            furi_string_cat_printf(ventra_prod_str, "3-Day");
            break;
        default:
            is_pass =
                true; // There are some card types I don't know what they are, but they parse like a pass, not a ticket.
            furi_string_cat_printf(ventra_prod_str, "0x%02X", data->page[5].data[2]);
            break;
        }

        uint16_t date_y = data->page[4].data[3] | (data->page[5].data[0] << 8);
        uint8_t date_d = date_y & 0x1F;
        uint8_t date_m = (date_y >> 5) & 0x0F;
        date_y >>= 9;
        date_y += 2000;
        ventra_exp_date.day = date_d;
        ventra_exp_date.month = date_m;
        ventra_exp_date.year = date_y;
        ventra_validity_date = ventra_exp_date; // Until we know otherwise

        // Parse the transaction blocks.  This sets a few sloppy globals, but it's too complex and repetitive to inline.
        FuriString* ventra_xact_str1 = ventra_parse_xact(data, 8, is_pass);
        FuriString* ventra_xact_str2 = ventra_parse_xact(data, 12, is_pass);

        uint8_t card_state = 1;
        uint8_t rides_left = 0;

        char* card_states[5] = {"???", "NEW", "ACT", "USED", "EXP"};

        if(ventra_high_seq > 1) card_state = 2;
        // On "ticket" product, the OTP bits mark off rides used.  Bit 0 seems to be unused, the next 3 are set as rides are used.
        // Some, not all, readers will set the high bits to 0x7 when a card is tapped after it's expired or depleted.  Have not
        // seen other combinations, but if we do, we'll make a nice ???.  1-day passes set the OTP bit 1 on first use.  3-day
        // passes do not.  But we don't really care, since they don't matter on passes, unless you're trying to rollback one.
        if(!is_pass) {
            switch(otp) {
            case 0:
                rides_left = 3;
                break;
            case 2:
                card_state = 2;
                rides_left = 2;
                break;
            case 6:
                card_state = 2;
                rides_left = 1;
                break;
            case 0x0E:
            case 0x7E:
                card_state = 3;
                rides_left = 0;
                break;
            default:
                card_state = 0;
                rides_left = 0;
                break;
            }
        }
        if(isExpired()) {
            card_state = 4;
            rides_left = 0;
        }

        furi_string_printf(
            parsed_data,
            "\e#Ventra %s (%s)\n",
            furi_string_get_cstr(ventra_prod_str),
            card_states[card_state]);

        furi_string_cat_printf(
            parsed_data,
            "Exp: %04d-%02d-%02d %02d:%02d\n",
            ventra_validity_date.year,
            ventra_validity_date.month,
            ventra_validity_date.day,
            ventra_validity_date.hour,
            ventra_validity_date.minute);

        if(rides_left) {
            furi_string_cat_printf(parsed_data, "Rides left: %d\n", rides_left);
        }

        furi_string_cat_printf(
            parsed_data,
            "%s\n",
            furi_string_get_cstr(ventra_cur_blk == 8 ? ventra_xact_str1 : ventra_xact_str2));

        furi_string_cat_printf(
            parsed_data,
            "%s\n",
            furi_string_get_cstr(ventra_cur_blk == 8 ? ventra_xact_str2 : ventra_xact_str1));

        furi_string_cat_printf(
            parsed_data, "TVM ID: %02X%02X\n", data->page[7].data[1], data->page[7].data[0]);
        furi_string_cat_printf(parsed_data, "Tx count: %d\n", ventra_high_seq);
        furi_string_cat_printf(
            parsed_data,
            "Hard Expiry: %04d-%02d-%02d",
            ventra_exp_date.year,
            ventra_exp_date.month,
            ventra_exp_date.day);

        furi_string_free(ventra_prod_str);
        furi_string_free(ventra_xact_str1);
        furi_string_free(ventra_xact_str2);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin ventra_plugin = {
    .protocol = NfcProtocolMfUltralight,
    .verify = NULL,
    .read = NULL,
    .parse = ventra_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor ventra_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &ventra_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* ventra_plugin_ep(void) {
    return &ventra_plugin_descriptor;
}
