#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>
#include <machine/endian.h>
#include <nfc/protocols/st25tb/st25tb.h>

#define TAG "MyKey"

const uint32_t blankBlock18 = 0x480FCD8F, blankBlock19 = 0x070082C0;

static bool mykey_is_blank(const St25tbData* data) {
    return data->blocks[0x18] == blankBlock18 && data->blocks[0x19] == blankBlock19;
}

static bool mykey_has_lockid(const St25tbData* data) {
    return (data->blocks[5] >> 24) == 0x7F;
}

static bool check_invalid_low_nibble(uint8_t value) {
    uint8_t value_lo = value & 0xF;
    return value_lo >= 0xA;
}

static bool mykey_get_production_date(
    const St25tbData* data,
    uint16_t* year_ptr,
    uint8_t* month_ptr,
    uint8_t* day_ptr) {
    uint32_t date_block = data->blocks[8];
    uint8_t year = date_block >> 16 & 0xFF;
    uint8_t month = date_block >> 8 & 0xFF;
    uint8_t day = date_block & 0xFF;
    // dates are coded in a peculiar way, the hexadecimal value should in fact be interpreted as a decimal value
    // so anything in range A-F is invalid.
    if(day > 0x31 || month > 0x12 || day == 0 || month == 0 || year == 0) {
        return false;
    }
    if(check_invalid_low_nibble(day) || check_invalid_low_nibble(month) ||
       check_invalid_low_nibble(year) || check_invalid_low_nibble(year >> 4)) {
        return false;
    }
    *year_ptr = year + 0x2000;
    *month_ptr = month;
    *day_ptr = day;
    return true;
}

static bool mykey_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const St25tbData* data = nfc_device_get_data(device, NfcProtocolSt25tb);

    if(data->type != St25tbType04k && data->type != St25tbTypeX4k) {
        FURI_LOG_D(TAG, "bad type");
        return false;
    }

    for(int i = 0; i < 5; i++) {
        if(data->blocks[i] != 0xFFFFFFFF) {
            FURI_LOG_D(TAG, "bad otp block %d", i);
            return false;
        }
    }

    uint16_t mfg_year;
    uint8_t mfg_month, mfg_day;

    if(!mykey_get_production_date(data, &mfg_year, &mfg_month, &mfg_day)) {
        FURI_LOG_D(TAG, "bad mfg date");
        return false;
    }

    if(data->system_otp_block != 0xFEFFFFFF) {
        FURI_LOG_D(TAG, "bad sys otp block");
        return false;
    }

    furi_string_cat(parsed_data, "\e#MyKey\n");

    if(data->blocks[6] == 0) { // Tag is actually a MyKey but it has been bricked by a reader
        furi_string_cat(parsed_data, "\e#Bricked!\nBlock 6 is 0!");
        return true;
    }

    bool is_blank = mykey_is_blank(data);
    furi_string_cat_printf(parsed_data, "Serial#: %08lX\n", (uint32_t)__bswap32(data->blocks[7]));
    furi_string_cat_printf(
        parsed_data, "Prod. date: %02X/%02X/%04X\n", mfg_day, mfg_month, mfg_year);
    furi_string_cat_printf(parsed_data, "Blank: %s\n", is_blank ? "yes" : "no");
    furi_string_cat_printf(parsed_data, "LockID: %s", mykey_has_lockid(data) ? "maybe" : "no");

    if(!is_blank) {
        furi_string_cat_printf(
            parsed_data, "\nOp. count: %zu\n", (size_t)__bswap32(data->blocks[0x12] & 0xFFFFFF00));

        uint32_t block3C = data->blocks[0x3C];
        if(block3C == 0xFFFFFFFF) {
            furi_string_cat(parsed_data, "No history available!");
        } else {
            block3C ^= data->blocks[0x07];
            uint32_t startingOffset = ((block3C & 0x30000000) >> 28) |
                                      ((block3C & 0x00100000) >> 18);
            furi_check(startingOffset < 8); //-V547
            for(int txnOffset = 8; txnOffset > 0; txnOffset--) {
                uint32_t txnBlock =
                    __bswap32(data->blocks[0x34 + ((startingOffset + txnOffset) % 8)]);

                if(txnBlock == 0xFFFFFFFF) {
                    break;
                }

                uint8_t day = txnBlock >> 27;
                uint8_t month = txnBlock >> 23 & 0xF;
                uint16_t year = 2000 + (txnBlock >> 16 & 0x7F);
                uint16_t credit = txnBlock & 0xFFFF;

                if(txnOffset == 8) {
                    furi_string_cat_printf(
                        parsed_data, "Current credit: %d.%02d euros\n", credit / 100, credit % 100);
                    furi_string_cat(parsed_data, "Op. history (newest first):");
                }

                furi_string_cat_printf(
                    parsed_data,
                    "\n    %02d/%02d/%04d %d.%02d",
                    day,
                    month,
                    year,
                    credit / 100,
                    credit % 100);
            }
        }
    }
    return true;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin mykey_plugin = {
    .protocol = NfcProtocolSt25tb,
    .verify = NULL,
    .read = NULL,
    .parse = mykey_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor mykey_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &mykey_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* mykey_plugin_ep(void) {
    return &mykey_plugin_descriptor;
}
