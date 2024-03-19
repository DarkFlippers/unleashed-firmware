#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>

#define TAG "AllInOne"

typedef enum {
    AllInOneLayoutTypeA,
    AllInOneLayoutTypeD,
    AllInOneLayoutTypeE2,
    AllInOneLayoutTypeE3,
    AllInOneLayoutTypeE5,
    AllInOneLayoutType2,
    AllInOneLayoutTypeUnknown,
} AllInOneLayoutType;

static AllInOneLayoutType all_in_one_get_layout(const MfUltralightData* data) {
    // Switch on the second half of the third byte of page 5
    const uint8_t layout_byte = data->page[5].data[2];
    const uint8_t layout_half_byte = data->page[5].data[2] & 0x0F;

    FURI_LOG_I(TAG, "Layout byte: %02x", layout_byte);
    FURI_LOG_I(TAG, "Layout half-byte: %02x", layout_half_byte);

    switch(layout_half_byte) {
    // If it is A, the layout type is a type A layout
    case 0x0A:
        return AllInOneLayoutTypeA;
    case 0x0D:
        return AllInOneLayoutTypeD;
    case 0x02:
        return AllInOneLayoutType2;
    default:
        FURI_LOG_I(TAG, "Unknown layout type: %d", layout_half_byte);
        return AllInOneLayoutTypeUnknown;
    }
}

static bool all_in_one_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const MfUltralightData* data = nfc_device_get_data(device, NfcProtocolMfUltralight);

    bool parsed = false;

    do {
        if(data->page[4].data[0] != 0x45 || data->page[4].data[1] != 0xD9) {
            FURI_LOG_I(TAG, "Pass not verified");
            break;
        }

        uint8_t ride_count = 0;
        uint32_t serial = 0;

        const AllInOneLayoutType layout_type = all_in_one_get_layout(data);

        if(layout_type == AllInOneLayoutTypeA) {
            // If the layout is A then the ride count is stored in the first byte of page 8
            ride_count = data->page[8].data[0];
        } else if(layout_type == AllInOneLayoutTypeD) {
            // If the layout is D, the ride count is stored in the second byte of page 9
            ride_count = data->page[9].data[1];
        } else {
            FURI_LOG_I(TAG, "Unknown layout: %d", layout_type);
            ride_count = 137;
        }

        // // The number starts at the second half of the third byte on page 4, and is 32 bits long
        // // So we get the second half of the third byte, then bytes 4-6, and then the first half of the 7th byte
        // // B8 17 A2 A4 BD becomes 81 7A 2A 4B
        const uint8_t* serial_data_lo = data->page[4].data;
        const uint8_t* serial_data_hi = data->page[5].data;

        serial = (serial_data_lo[2] & 0x0F) << 28 | serial_data_lo[3] << 20 |
                 serial_data_hi[0] << 12 | serial_data_hi[1] << 4 | serial_data_hi[2] >> 4;

        // Format string for rides count
        furi_string_printf(
            parsed_data, "\e#All-In-One\nNumber: %lu\nRides left: %u", serial, ride_count);

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin all_in_one_plugin = {
    .protocol = NfcProtocolMfUltralight,
    .verify = NULL,
    .read = NULL,
    .parse = all_in_one_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor all_in_one_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &all_in_one_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* all_in_one_plugin_ep(void) {
    return &all_in_one_plugin_descriptor;
}
