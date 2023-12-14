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
    return (data->blocks[5] & 0xFF) == 0x7F;
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

    if((data->blocks[8] >> 16 & 0xFF) > 0x31 || (data->blocks[8] >> 8 & 0xFF) > 0x12) {
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
    furi_string_cat_printf(parsed_data, "Serial#: %08lX\n", __bswap32(data->blocks[7]));
    furi_string_cat_printf(parsed_data, "Blank: %s\n", is_blank ? "yes" : "no");
    furi_string_cat_printf(parsed_data, "LockID: %s\n", mykey_has_lockid(data) ? "maybe" : "no");

    uint32_t block8 = data->blocks[8];
    furi_string_cat_printf(
        parsed_data,
        "Prod. date: %02lX/%02lX/%04lX",
        block8 >> 16 & 0xFF,
        block8 >> 8 & 0xFF,
        0x2000 + (block8 & 0xFF));

    if(!is_blank) {
        furi_string_cat_printf(
            parsed_data, "\nOp. count: %ld\n", __bswap32(data->blocks[0x12] & 0xFFFFFF00));

        uint32_t block3C = data->blocks[0x3C];
        if(block3C == 0xFFFFFFFF) {
            furi_string_cat(parsed_data, "No history available!");
        } else {
            block3C ^= data->blocks[0x07];
            uint32_t startingOffset = ((block3C & 0x30000000) >> 28) |
                                      ((block3C & 0x00100000) >> 18);
            furi_check(startingOffset < 8);
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
const FlipperAppPluginDescriptor* mykey_plugin_ep() {
    return &mykey_plugin_descriptor;
}
