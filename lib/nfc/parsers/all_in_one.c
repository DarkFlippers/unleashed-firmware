#include "nfc_supported_card.h"
#include "all_in_one.h"

#include <gui/modules/widget.h>
#include <nfc_worker_i.h>

#include <furi_hal.h>

#define ALL_IN_ONE_LAYOUT_UNKNOWN 0
#define ALL_IN_ONE_LAYOUT_A 1
#define ALL_IN_ONE_LAYOUT_D 2
#define ALL_IN_ONE_LAYOUT_E2 3
#define ALL_IN_ONE_LAYOUT_E3 4
#define ALL_IN_ONE_LAYOUT_E5 5
#define ALL_IN_ONE_LAYOUT_2 6

uint8_t all_in_one_get_layout(NfcDeviceData* dev_data) {
    // I absolutely hate what's about to happen here.

    // Switch on the second half of the third byte of page 5
    FURI_LOG_I("all_in_one", "Layout byte: %02x", dev_data->mf_ul_data.data[(4 * 5) + 2]);
    FURI_LOG_I(
        "all_in_one", "Layout half-byte: %02x", dev_data->mf_ul_data.data[(4 * 5) + 3] & 0x0F);
    switch(dev_data->mf_ul_data.data[(4 * 5) + 2] & 0x0F) {
    // If it is A, the layout type is a type A layout
    case 0x0A:
        return ALL_IN_ONE_LAYOUT_A;
    case 0x0D:
        return ALL_IN_ONE_LAYOUT_D;
    case 0x02:
        return ALL_IN_ONE_LAYOUT_2;
    default:
        FURI_LOG_I(
            "all_in_one",
            "Unknown layout type: %d",
            dev_data->mf_ul_data.data[(4 * 5) + 2] & 0x0F);
        return ALL_IN_ONE_LAYOUT_UNKNOWN;
    }
}

bool all_in_one_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    UNUSED(nfc_worker);
    // If this is a all_in_one pass, first 2 bytes of page 4 are 0x45 0xD9
    MfUltralightReader reader = {};
    MfUltralightData data = {};

    if(!mf_ul_read_card(tx_rx, &reader, &data)) {
        return false;
    } else {
        if(data.data[4 * 4] == 0x45 && data.data[4 * 4 + 1] == 0xD9) {
            FURI_LOG_I("all_in_one", "Pass verified");
            return true;
        }
    }
    return false;
}

bool all_in_one_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    MfUltralightReader reader = {};
    MfUltralightData data = {};
    if(!mf_ul_read_card(tx_rx, &reader, &data)) {
        return false;
    } else {
        memcpy(&nfc_worker->dev_data->mf_ul_data, &data, sizeof(data));
        FURI_LOG_I("all_in_one", "Card read");
        return true;
    }
}

bool all_in_one_parser_parse(NfcDeviceData* dev_data) {
    if(dev_data->mf_ul_data.data[4 * 4] != 0x45 || dev_data->mf_ul_data.data[4 * 4 + 1] != 0xD9) {
        FURI_LOG_I("all_in_one", "Pass not verified");
        return false;
    }

    uint8_t ride_count = 0;
    uint32_t serial = 0;
    if(all_in_one_get_layout(dev_data) == ALL_IN_ONE_LAYOUT_A) {
        // If the layout is A then the ride count is stored in the first byte of page 8
        ride_count = dev_data->mf_ul_data.data[4 * 8];
    } else if(all_in_one_get_layout(dev_data) == ALL_IN_ONE_LAYOUT_D) {
        // If the layout is D, the ride count is stored in the second byte of page 9
        ride_count = dev_data->mf_ul_data.data[4 * 9 + 1];
    } else {
        FURI_LOG_I("all_in_one", "Unknown layout: %d", all_in_one_get_layout(dev_data));
        ride_count = 137;
    }

    // I hate this with a burning passion.

    // The number starts at the second half of the third byte on page 4, and is 32 bits long
    // So we get the second half of the third byte, then bytes 4-6, and then the first half of the 7th byte
    // B8 17 A2 A4 BD becomes 81 7A 2A 4B
    serial =
        (dev_data->mf_ul_data.data[4 * 4 + 2] & 0x0F) << 28 |
        dev_data->mf_ul_data.data[4 * 4 + 3] << 20 | dev_data->mf_ul_data.data[4 * 4 + 4] << 12 |
        dev_data->mf_ul_data.data[4 * 4 + 5] << 4 | (dev_data->mf_ul_data.data[4 * 4 + 6] >> 4);

    // Format string for rides count
    furi_string_printf(
        dev_data->parsed_data, "\e#All-In-One\nNumber: %lu\nRides left: %u", serial, ride_count);
    return true;
}
