// Flipper Zero parser for for Tianjin Railway Transit (TRT)
// https://en.wikipedia.org/wiki/Tianjin_Metro
// Reverse engineering and parser development by @Torron (Github: @zinongli)

#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>
#include <bit_lib.h>

#define TAG                       "TrtParser"
#define LATEST_SALE_MARKER        0x02
#define FULL_SALE_TIME_STAMP_PAGE 0x09
#define BALANCE_PAGE              0x08
#define SALE_RECORD_TIME_STAMP_A  0x0C
#define SALE_RECORD_TIME_STAMP_B  0x0E
#define SALE_YEAR_OFFSET          2000

static bool trt_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfUltralightData* data = nfc_device_get_data(device, NfcProtocolMfUltralight);

    bool parsed = false;

    do {
        uint8_t latest_sale_page = 0;

        // Look for sale record signature
        if(data->page[SALE_RECORD_TIME_STAMP_A].data[0] == LATEST_SALE_MARKER) {
            latest_sale_page = SALE_RECORD_TIME_STAMP_A;
        } else if(data->page[SALE_RECORD_TIME_STAMP_B].data[0] == LATEST_SALE_MARKER) {
            latest_sale_page = SALE_RECORD_TIME_STAMP_B;
        } else {
            break;
        }

        // Check if the sale record was backed up
        const uint8_t* partial_record_pointer = &data->page[latest_sale_page - 1].data[0];
        const uint8_t* full_record_pointer = &data->page[FULL_SALE_TIME_STAMP_PAGE].data[0];
        uint32_t latest_sale_record = bit_lib_get_bits_32(partial_record_pointer, 3, 20);
        uint32_t latest_sale_full_record = bit_lib_get_bits_32(full_record_pointer, 0, 27);
        if(latest_sale_record != (latest_sale_full_record & 0xFFFFF))
            break; // check if the copy matches
        if((latest_sale_record == 0) || (latest_sale_full_record == 0))
            break; // prevent false positive

        // Parse date
        // yyy yyyymmmm dddddhhh hhnnnnnn
        uint16_t sale_year = ((latest_sale_full_record & 0x7F00000) >> 20) + SALE_YEAR_OFFSET;
        uint8_t sale_month = (latest_sale_full_record & 0xF0000) >> 16;
        uint8_t sale_day = (latest_sale_full_record & 0xF800) >> 11;
        uint8_t sale_hour = (latest_sale_full_record & 0x7C0) >> 6;
        uint8_t sale_minute = latest_sale_full_record & 0x3F;

        // Parse balance
        uint16_t balance = bit_lib_get_bits_16(&data->page[BALANCE_PAGE].data[2], 0, 16);
        uint16_t balance_yuan = balance / 100;
        uint8_t balance_cent = balance % 100;

        // Format string for rendering
        furi_string_cat_printf(parsed_data, "\e#TRT Tianjin Metro\n");
        furi_string_cat_printf(parsed_data, "Single-Use Ticket\n");
        furi_string_cat_printf(parsed_data, "Balance: %u.%02u RMB\n", balance_yuan, balance_cent);
        furi_string_cat_printf(
            parsed_data,
            "Sale Date: \n%04u-%02d-%02d %02d:%02d",
            sale_year,
            sale_month,
            sale_day,
            sale_hour,
            sale_minute);
        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin trt_plugin = {
    .protocol = NfcProtocolMfUltralight,
    .verify = NULL,
    .read = NULL,
    .parse = trt_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor trt_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &trt_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* trt_plugin_ep(void) {
    return &trt_plugin_descriptor;
}
