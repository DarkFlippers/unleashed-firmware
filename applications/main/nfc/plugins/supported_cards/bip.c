#include "nfc_supported_card_plugin.h"

#include <bit_lib/bit_lib.h>
#include <flipper_application/flipper_application.h>
#include <nfc/nfc_device.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "Bip"

#define SECTOR_BLOCK_OFFSET(sector, block) (((sector) * 4) + (block))

static const uint64_t bip_keys_a[] = {
    0x3a42f33af429,
    0x6338a371c0ed,
    0xf124c2578ad0,
    0x32ac3b90ac13,
    0x4ad1e273eaf1,
    0xe2c42591368a,
    0x2a3c347a1200,
    0x16f3d5ab1139,
    0x937a4fff3011,
    0x35c3d2caee88,
    0x693143f10368,
    0xa3f97428dd01,
    0x63f17a449af0,
    0xc4652c54261c,
    0xd49e2826664f,
    0x3df14c8000a1,
};

static const uint64_t bip_keys_b[] = {
    0x1fc235ac1309,
    0x243f160918d1,
    0x9afc42372af1,
    0x682d401abb09,
    0x067db45454a9,
    0x15fc4c7613fe,
    0x68d30288910a,
    0xf59a36a2546d,
    0x64e3c10394c2,
    0xb736412614af,
    0x324f5df65310,
    0x643fb6de2217,
    0x82f435dedf01,
    0x0263de1278f3,
    0x51284c3686a6,
    0x6a470d54127c,
};

bool bip_verify(Nfc* nfc) {
    bool verified = true;

    const uint8_t verify_sector = 0;
    uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);
    FURI_LOG_D(TAG, "Verifying sector %u", verify_sector);

    MfClassicKey key_a_0 = {};
    bit_lib_num_to_bytes_be(bip_keys_a[0], COUNT_OF(key_a_0.data), key_a_0.data);

    MfClassicAuthContext auth_ctx = {};
    MfClassicError error =
        mf_classic_poller_sync_auth(nfc, block_num, &key_a_0, MfClassicKeyTypeA, &auth_ctx);

    if(error != MfClassicErrorNone) {
        FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
        verified = false;
    }

    return verified;
}

static bool bip_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicType1k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;
        if(type != MfClassicType1k) {
            FURI_LOG_W(TAG, "Card not MIFARE Classic 1k");
            break;
        }

        data->type = type;
        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(bip_keys_a[i], sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(bip_keys_b[i], sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error == MfClassicErrorNotPresent) {
            FURI_LOG_W(TAG, "Failed to read data. Bad keys?");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = true;
    } while(false);

    mf_classic_free(data);

    return is_read;
}

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} BipTimestamp;

static void parse_bip_timestamp(const MfClassicBlock* block, BipTimestamp* timestamp) {
    furi_assert(block);
    furi_assert(timestamp);

    timestamp->day = (((block->data[1] << 8) + block->data[0]) >> 6) & 0x1f;
    timestamp->month = (((block->data[1] << 8) + block->data[0]) >> 11) & 0xf;
    timestamp->year = 2000 + ((((block->data[2] << 8) + block->data[1]) >> 7) & 0x1f);
    timestamp->hour = (((block->data[3] << 8) + block->data[2]) >> 4) & 0x1f;
    timestamp->minute = (((block->data[3] << 8) + block->data[2]) >> 9) & 0x3f;
    timestamp->second = (((block->data[4] << 8) + block->data[3]) >> 7) & 0x3f;
}

static int compare_bip_timestamp(const BipTimestamp* t1, const BipTimestamp* t2) {
    furi_assert(t1);
    furi_assert(t2);
    if(t1->year != t2->year) {
        return t1->year - t2->year;
    }
    if(t1->month != t2->month) {
        return t1->month - t2->month;
    }
    if(t1->day != t2->day) {
        return t1->day - t2->day;
    }
    if(t1->hour != t2->hour) {
        return t1->hour - t2->hour;
    }
    if(t1->minute != t2->minute) {
        return t1->minute - t2->minute;
    }
    if(t1->second != t2->second) {
        return t1->second - t2->second;
    }
    return 0;
}

static void print_bip_timestamp(const BipTimestamp* timestamp, FuriString* str) {
    furi_assert(timestamp);
    furi_assert(str);
    furi_string_cat_printf(
        str,
        "%04u-%02u-%02u %02u:%02u:%02u",
        timestamp->year,
        timestamp->month,
        timestamp->day,
        timestamp->hour,
        timestamp->minute,
        timestamp->second);
}

static bool is_bip_block_empty(const MfClassicBlock* block) {
    furi_assert(block);
    // check if all but last byte are zero (last is checksum)
    for(size_t i = 0; i < sizeof(block->data) - 1; i++) {
        if(block->data[i] != 0) {
            return false;
        }
    }
    return true;
}

static void parse_uint16_le(const uint8_t* data, uint16_t* value) {
    furi_assert(data);
    furi_assert(value);

    *value = (data[0]) | (data[1] << 8);
}

static void parse_uint32_le(const uint8_t* data, uint32_t* value) {
    furi_assert(data);
    furi_assert(value);

    *value = (data[0]) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

static void parse_uint16_txn_amount(const uint8_t* data, uint16_t* value) {
    furi_assert(data);
    furi_assert(value);

    parse_uint16_le(data, value);
    *value = *value >> 2;
}

typedef struct {
    BipTimestamp timestamp;
    uint16_t amount;
} BipTransaction;

static bool bip_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    bool parsed = true;

    struct {
        uint32_t card_id;
        uint16_t balance;
        uint16_t flags;
        BipTimestamp trip_time_window;
        BipTransaction top_ups[3];
        BipTransaction charges[3];
    } bip_data = {
        .card_id = 0,
        .balance = 0,
        .flags = 0,
        .trip_time_window = {0, 0, 0, 0, 0, 0},
        .top_ups =
            {
                {{0, 0, 0, 0, 0, 0}, 0},
                {{0, 0, 0, 0, 0, 0}, 0},
                {{0, 0, 0, 0, 0, 0}, 0},
            },
        .charges =
            {
                {{0, 0, 0, 0, 0, 0}, 0},
                {{0, 0, 0, 0, 0, 0}, 0},
                {{0, 0, 0, 0, 0, 0}, 0},
            },
    };

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    do {
        // verify first sector keys
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 0);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != bip_keys_a[0]) {
            parsed = false;
            break;
        }
        key = bit_lib_bytes_to_num_be(sec_tr->key_b.data, 6);
        if(key != bip_keys_b[0]) {
            parsed = false;
            break;
        }

        // Get Card ID, little-endian 4 bytes at sector 0 block 1, bytes 4-7
        parse_uint32_le(&data->block[SECTOR_BLOCK_OFFSET(0, 1)].data[4], &bip_data.card_id);

        // Get balance, little-endian 2 bytes at sector 8 block 1, bytes 0-1
        parse_uint16_le(&data->block[SECTOR_BLOCK_OFFSET(8, 1)].data[0], &bip_data.balance);

        // Get balance flags (negative balance, etc.), little-endian 2 bytes at sector 8 block 1, bytes 2-3
        parse_uint16_le(&data->block[SECTOR_BLOCK_OFFSET(8, 1)].data[2], &bip_data.flags);

        // Get trip time window, proprietary format, at sector 5 block 1, bytes 0-7
        parse_bip_timestamp(&data->block[SECTOR_BLOCK_OFFSET(5, 1)], &bip_data.trip_time_window);

        // Last 3 top-ups: sector 10, ring-buffer of 3 blocks, timestamp in bytes 0-7, amount in bytes 9-10
        for(size_t i = 0; i < 3; i++) {
            if(is_bip_block_empty(&data->block[SECTOR_BLOCK_OFFSET(10, i)])) {
                continue;
            }
            BipTransaction* top_up = &bip_data.top_ups[i];
            parse_bip_timestamp(&data->block[SECTOR_BLOCK_OFFSET(10, i)], &top_up->timestamp);
            parse_uint16_txn_amount(
                &data->block[SECTOR_BLOCK_OFFSET(10, i)].data[9], &top_up->amount);
        }

        // Last 3 charges (i.e. trips), sector 11, ring-buffer of 3 blocks, timestamp in bytes 0-7, amount in bytes 10-11
        for(size_t i = 0; i < 3; i++) {
            if(is_bip_block_empty(&data->block[SECTOR_BLOCK_OFFSET(11, i)])) {
                continue;
            }
            BipTransaction* charge = &bip_data.charges[i];
            parse_bip_timestamp(&data->block[SECTOR_BLOCK_OFFSET(11, i)], &charge->timestamp);
            parse_uint16_txn_amount(
                &data->block[SECTOR_BLOCK_OFFSET(11, i)].data[10], &charge->amount);
        }

        // All data is now parsed and stored in bip_data, now print it

        // Print basic info
        furi_string_printf(
            parsed_data,
            "\e#Tarjeta Bip!\n"
            "Card Number: %lu\n"
            "Balance: $%hu (flags %hu)\n"
            "Current Trip Window Ends:\n  @",
            bip_data.card_id,
            bip_data.balance,
            bip_data.flags);

        print_bip_timestamp(&bip_data.trip_time_window, parsed_data);

        // Find newest top-up
        size_t newest_top_up = 0;
        for(size_t i = 1; i < 3; i++) {
            const BipTimestamp* newest = &bip_data.top_ups[newest_top_up].timestamp;
            const BipTimestamp* current = &bip_data.top_ups[i].timestamp;
            if(compare_bip_timestamp(current, newest) > 0) {
                newest_top_up = i;
            }
        }

        // Print top-ups, newest first
        furi_string_cat_printf(parsed_data, "\n\e#Last Top-ups");
        for(size_t i = 0; i < 3; i++) {
            const BipTransaction* top_up = &bip_data.top_ups[(3u + newest_top_up - i) % 3];
            furi_string_cat_printf(parsed_data, "\n+$%d\n  @", top_up->amount);
            print_bip_timestamp(&top_up->timestamp, parsed_data);
        }

        // Find newest charge
        size_t newest_charge = 0;
        for(size_t i = 1; i < 3; i++) {
            const BipTimestamp* newest = &bip_data.charges[newest_charge].timestamp;
            const BipTimestamp* current = &bip_data.charges[i].timestamp;
            if(compare_bip_timestamp(current, newest) > 0) {
                newest_charge = i;
            }
        }

        // Print charges
        furi_string_cat_printf(parsed_data, "\n\e#Last Charges (Trips)");
        for(size_t i = 0; i < 3; i++) {
            const BipTransaction* charge = &bip_data.charges[(3u + newest_charge - i) % 3];
            furi_string_cat_printf(parsed_data, "\n-$%d\n  @", charge->amount);
            print_bip_timestamp(&charge->timestamp, parsed_data);
        }

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin bip_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = bip_verify,
    .read = bip_read,
    .parse = bip_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor bip_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &bip_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* bip_plugin_ep(void) {
    return &bip_plugin_descriptor;
}
