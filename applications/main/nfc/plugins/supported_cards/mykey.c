#include "nfc_supported_card_plugin.h"
#include <flipper_application/flipper_application.h>
#include <lib/nfc/protocols/st25tb/st25tb.h>
#include <nfc/nfc_device.h>
#include <nfc/helpers/nfc_util.h>

//Structures data of mykey card
enum {
    MYKEY_BLOCK_KEY_ID = 0x07,
    MYKEY_BLOCK_PRODUCTION_DATE = 0x08,
    MYKEY_BLOCK_VENDOR_ID_1 = 0x18,
    MYKEY_BLOCK_VENDOR_ID_2 = 0x19,
    MYKEY_BLOCK_CURRENT_CREDIT = 0x21,
    MYKEY_BLOCK_PREVIOUS_CREDIT = 0x23,
    MYKEY_DEFAULT_VENDOR_ID = 0xFEDC0123,
    MYKEY_DEFAULT_VENDOR_ID_1 = 0xFEDC,
    MYKEY_DEFAULT_VENDOR_ID_2 = 0x0123,
};

typedef enum {
    LockIdStatusNone,
    LockIdStatusActive,
} LockIdStatus;

/* Function to obtain the UID as a 32-bit */
uint32_t get_uid(const uint8_t uid[8]) {
    return (uid[7] | (uid[6] << 8) | (uid[5] << 16) | (uid[4] << 24));
}

/* OTP calculation (reverse block 6, incremental. 1,2,3, ecc.) */
uint32_t new_get_count_down_counter(uint32_t b6) {
    return ~(b6 << 24 | (b6 & 0x0000FF00) << 8 | (b6 & 0x00FF0000) >> 8 | b6 >> 24);
}

/* Function to check if the vendor is bound */
int get_is_bound(uint32_t vendor_id) {
    return (vendor_id != MYKEY_DEFAULT_VENDOR_ID);
}

/* MK = UID * VENDOR */
uint32_t get_master_key(uint32_t uid, uint32_t vendor_id) {
    return uid * (vendor_id + 1);
}

/* SK (Encryption key) = MK * OTP */
uint32_t get_encryption_key(uint32_t master_key, uint32_t count_down_counter) {
    return master_key * (count_down_counter + 1);
}

/* Encode or decode a MyKey block */
uint32_t encode_decode_block(uint32_t input) {
    /*
     * Swap all values using XOR
     * 32 bit: 1111222233334444
     */
    input ^= (input & 0x00C00000) << 6 | (input & 0x0000C000) << 12 | (input & 0x000000C0) << 18 |
             (input & 0x000C0000) >> 6 | (input & 0x00030000) >> 12 | (input & 0x00000300) >> 6;
    input ^= (input & 0x30000000) >> 6 | (input & 0x0C000000) >> 12 | (input & 0x03000000) >> 18 |
             (input & 0x00003000) << 6 | (input & 0x00000030) << 12 | (input & 0x0000000C) << 6;
    input ^= (input & 0x00C00000) << 6 | (input & 0x0000C000) << 12 | (input & 0x000000C0) << 18 |
             (input & 0x000C0000) >> 6 | (input & 0x00030000) >> 12 | (input & 0x00000300) >> 6;
    return input;
}

uint32_t get_block(uint32_t block) {
    return encode_decode_block(__bswap32(block));
}

uint32_t get_xored_block(uint32_t block, uint32_t key) {
    return encode_decode_block(__bswap32(block) ^ key);
}

uint32_t get_vendor(uint32_t b1, uint32_t b2) {
    return b1 << 16 | (b2 & 0x0000FFFF);
}

static bool mykey_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    bool parsed = false;

    do {
        //Get data
        const St25tbData* data = nfc_device_get_data(device, NfcProtocolSt25tb);

        //Calc data
        uint32_t _uid = get_uid(data->uid);
        uint32_t _count_down_counter_new = new_get_count_down_counter(__bswap32(data->blocks[6]));
        uint32_t _vendor_id = get_vendor(
            get_block(data->blocks[MYKEY_BLOCK_VENDOR_ID_1]),
            get_block(data->blocks[MYKEY_BLOCK_VENDOR_ID_2]));
        uint32_t _master_key = get_master_key(_uid, _vendor_id);
        uint32_t _encryption_key = get_encryption_key(_master_key, _count_down_counter_new);
        uint16_t credit =
            get_xored_block(data->blocks[MYKEY_BLOCK_CURRENT_CREDIT], _encryption_key);
        uint16_t _previous_credit = get_block(data->blocks[MYKEY_BLOCK_PREVIOUS_CREDIT]);
        bool _is_bound = get_is_bound(_vendor_id);

        //parse data
        furi_string_cat_printf(parsed_data, "\e#MyKey Card\n");
        furi_string_cat_printf(parsed_data, "UID: %08lX\n", _uid);
        furi_string_cat_printf(parsed_data, "Vendor ID: %08lX\n", _vendor_id);
        furi_string_cat_printf(
            parsed_data, "Current Credit: %d.%02d E \n", credit / 100, credit % 100);
        furi_string_cat_printf(
            parsed_data,
            "Previus Credit: %d.%02d E \n",
            _previous_credit / 100,
            _previous_credit % 100);
        furi_string_cat_printf(parsed_data, "Is Bound: %s\n", _is_bound ? "yes" : "no");

        parsed = true;
    } while(false);

    return parsed;
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