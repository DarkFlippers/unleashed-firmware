/*
* Parser for CSC Service Works Reloadable Cash Card (US)
* Date created 2024/5/26
* Zinong Li  
* Discord  @torron0483 
* Github   @zinongli
*/

#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <bit_lib.h>
#include <furi/core/string.h>

#include <nfc/nfc.h>
#include <nfc/nfc_device.h>

#define TAG "CSC"

bool csc_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;

    do {
        if(data->type != MfClassicType1k) break; // Check card type

        // Verify memory format (checksum is later)
        const uint8_t refill_block_num = 2;
        const uint8_t current_balance_block_num = 4;
        const uint8_t current_balance_copy_block_num = 8;

        const uint8_t* current_balance_block_start_ptr =
            &data->block[current_balance_block_num].data[0];
        const uint8_t* current_balance_copy_block_start_ptr =
            &data->block[current_balance_copy_block_num].data[0];

        uint32_t current_balance_and_times =
            bit_lib_bytes_to_num_le(current_balance_block_start_ptr, 4);
        uint32_t current_balance_and_times_copy =
            bit_lib_bytes_to_num_le(current_balance_copy_block_start_ptr, 4);

        // Failed verification if balance != backup
        if(current_balance_and_times != current_balance_and_times_copy) {
            FURI_LOG_D(TAG, "Backup verification failed");
            break;
        }

        // Even if balance = 0, e.g. new card, refilled times can't be zero
        if(current_balance_and_times == 0 || current_balance_and_times_copy == 0) {
            FURI_LOG_D(TAG, "Value bytes empty");
            break;
        }

        // Parse data
        const uint8_t card_lives_block_num = 9;
        const uint8_t refill_sign_block_num = 13;

        const uint8_t* refilled_balance_block_start_ptr = &data->block[refill_block_num].data[9];
        const uint8_t* refill_times_block_start_ptr = &data->block[refill_block_num].data[5];
        const uint8_t* card_lives_block_start_ptr = &data->block[card_lives_block_num].data[0];
        const uint8_t* refill_sign_block_start_ptr = &data->block[refill_sign_block_num].data[0];

        uint32_t refilled_balance = bit_lib_bytes_to_num_le(refilled_balance_block_start_ptr, 2);
        uint32_t refilled_balance_dollar = refilled_balance / 100;
        uint8_t refilled_balance_cent = refilled_balance % 100;

        uint32_t current_balance = bit_lib_bytes_to_num_le(current_balance_block_start_ptr, 2);
        uint32_t current_balance_dollar = current_balance / 100;
        uint8_t current_balance_cent = current_balance % 100;

        // How many times it can still be used
        uint32_t card_lives = bit_lib_bytes_to_num_le(card_lives_block_start_ptr, 2);

        uint32_t refill_times = bit_lib_bytes_to_num_le(refill_times_block_start_ptr, 2);
        // This is zero when you buy the card. but after refilling it, the refilling machine will leave a non-zero signature here
        uint64_t refill_sign = bit_lib_bytes_to_num_le(refill_sign_block_start_ptr, 8);

        size_t uid_len = 0;
        const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
        uint32_t card_uid = bit_lib_bytes_to_num_le(uid, 4);

        // Last byte of refill block is checksum
        const uint8_t* checksum_block = data->block[refill_block_num].data;
        uint8_t xor_result = 0;
        for(size_t i = 0; i < 16; ++i) {
            xor_result ^= checksum_block[i];
        }

        if(refill_sign == 0 && refill_times == 1) {
            // New cards don't comply to checksum but refill time should be once
            furi_string_printf(
                parsed_data,
                "\e#CSC Service Works\n"
                "UID: %lu\n"
                "New Card\n"
                "Card Value: %lu.%02u USD\n"
                "Card Usages Left: %lu",
                card_uid,
                refilled_balance_dollar,
                refilled_balance_cent,
                card_lives);
        } else {
            if(xor_result != 0) {
                FURI_LOG_D(TAG, "Checksum failed");
                break;
            }
            furi_string_printf(
                parsed_data,
                "\e#CSC Service Works\n"
                "UID: %lu\n"
                "Balance: %lu.%02u USD\n"
                "Last Top-up: %lu.%02u USD\n"
                "Top-up Count: %lu\n"
                "Card Usages Left: %lu",
                card_uid,
                current_balance_dollar,
                current_balance_cent,
                refilled_balance_dollar,
                refilled_balance_cent,
                refill_times,
                card_lives);
        }

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin csc_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = NULL,
    .read = NULL,
    .parse = csc_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor csc_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &csc_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* csc_plugin_ep(void) {
    return &csc_plugin_descriptor;
}
