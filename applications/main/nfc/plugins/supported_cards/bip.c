#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>

#include "protocols/mf_classic/mf_classic.h"
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#include <bit_lib.h>
#include <locale/locale.h>

#define TAG "Bip"

#define BIP_CARD_ID_SECTOR_NUMBER          (0)
#define BIP_BALANCE_SECTOR_NUMBER          (8)
#define BIP_TRIP_TIME_WINDOW_SECTOR_NUMBER (5)
#define BIP_LAST_TOP_UPS_SECTOR_NUMBER     (10)
#define BIP_TRIPS_INFO_SECTOR_NUMBER       (11)

typedef struct {
    DateTime datetime;
    uint16_t amount;
} BipTransaction;

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static const MfClassicKeyPair bip_1k_keys[] = {
    {.a = 0x3a42f33af429, .b = 0x1fc235ac1309},
    {.a = 0x6338a371c0ed, .b = 0x243f160918d1},
    {.a = 0xf124c2578ad0, .b = 0x9afc42372af1},
    {.a = 0x32ac3b90ac13, .b = 0x682d401abb09},
    {.a = 0x4ad1e273eaf1, .b = 0x067db45454a9},
    {.a = 0xe2c42591368a, .b = 0x15fc4c7613fe},
    {.a = 0x2a3c347a1200, .b = 0x68d30288910a},
    {.a = 0x16f3d5ab1139, .b = 0xf59a36a2546d},
    {.a = 0x937a4fff3011, .b = 0x64e3c10394c2},
    {.a = 0x35c3d2caee88, .b = 0xb736412614af},
    {.a = 0x693143f10368, .b = 0x324f5df65310},
    {.a = 0xa3f97428dd01, .b = 0x643fb6de2217},
    {.a = 0x63f17a449af0, .b = 0x82f435dedf01},
    {.a = 0xc4652c54261c, .b = 0x0263de1278f3},
    {.a = 0xd49e2826664f, .b = 0x51284c3686a6},
    {.a = 0x3df14c8000a1, .b = 0x6a470d54127c},
};

static void bip_parse_datetime(const MfClassicBlock* block, DateTime* parsed_data) {
    furi_assert(block);
    furi_assert(parsed_data);

    parsed_data->day = (((block->data[1] << 8) + block->data[0]) >> 6) & 0x1f;
    parsed_data->month = (((block->data[1] << 8) + block->data[0]) >> 11) & 0xf;
    parsed_data->year = 2000 + ((((block->data[2] << 8) + block->data[1]) >> 7) & 0x1f);
    parsed_data->hour = (((block->data[3] << 8) + block->data[2]) >> 4) & 0x1f;
    parsed_data->minute = (((block->data[3] << 8) + block->data[2]) >> 9) & 0x3f;
    parsed_data->second = (((block->data[4] << 8) + block->data[3]) >> 7) & 0x3f;
}

static void bip_print_datetime(const DateTime* datetime, FuriString* str) {
    furi_assert(datetime);
    furi_assert(str);

    LocaleDateFormat date_format = locale_get_date_format();
    const char* separator = (date_format == LocaleDateFormatDMY) ? "." : "/";

    FuriString* date_str = furi_string_alloc();
    locale_format_date(date_str, datetime, date_format, separator);

    FuriString* time_str = furi_string_alloc();
    locale_format_time(time_str, datetime, locale_get_time_format(), true);

    furi_string_cat_printf(
        str, "%s %s", furi_string_get_cstr(date_str), furi_string_get_cstr(time_str));

    furi_string_free(date_str);
    furi_string_free(time_str);
}

static int datetime_cmp(const DateTime* dt_1, const DateTime* dt_2) {
    furi_assert(dt_1);
    furi_assert(dt_2);

    if(dt_1->year != dt_2->year) {
        return dt_1->year - dt_2->year;
    }
    if(dt_1->month != dt_2->month) {
        return dt_1->month - dt_2->month;
    }
    if(dt_1->day != dt_2->day) {
        return dt_1->day - dt_2->day;
    }
    if(dt_1->hour != dt_2->hour) {
        return dt_1->hour - dt_2->hour;
    }
    if(dt_1->minute != dt_2->minute) {
        return dt_1->minute - dt_2->minute;
    }
    if(dt_1->second != dt_2->second) {
        return dt_1->second - dt_2->second;
    }
    return 0;
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

bool bip_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 0;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);
        FURI_LOG_D(TAG, "Verifying sector %u", verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(bip_1k_keys[0].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);

        if(error == MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool bip_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicTypeMini;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        if(type != MfClassicType1k) break;

        MfClassicDeviceKeys keys = {
            .key_a_mask = 0,
            .key_b_mask = 0,
        };
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(bip_1k_keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(bip_1k_keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }

        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error == MfClassicErrorNotPresent) {
            FURI_LOG_W(TAG, "Failed to read data");
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (error == MfClassicErrorNone);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static bool bip_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    struct {
        uint32_t card_id;
        uint16_t balance;
        uint16_t flags;
        DateTime trip_time_window;
        BipTransaction top_ups[3];
        BipTransaction charges[3];
    } bip_data = {0};

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;

    do {
        // verify sector 0 key A
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 0);

        if(data->type != MfClassicType1k) break;

        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != bip_1k_keys[0].a) {
            break;
        }

        // verify sector 0 key B
        key = bit_lib_bytes_to_num_be(sec_tr->key_b.data, 6);
        if(key != bip_1k_keys[0].b) {
            break;
        }

        // Get Card ID, little-endian 4 bytes at sector 0 block 1, bytes 4-7
        const uint8_t card_id_start_block_num =
            mf_classic_get_first_block_num_of_sector(BIP_CARD_ID_SECTOR_NUMBER);
        const uint8_t* block_start_ptr = &data->block[card_id_start_block_num + 1].data[0];

        bip_data.card_id = bit_lib_bytes_to_num_le(block_start_ptr + 4, 4);

        // Get balance, little-endian 2 bytes at sector 8 block 1, bytes 0-1
        const uint8_t balance_start_block_num =
            mf_classic_get_first_block_num_of_sector(BIP_BALANCE_SECTOR_NUMBER);
        block_start_ptr = &data->block[balance_start_block_num + 1].data[0];

        bip_data.balance = bit_lib_bytes_to_num_le(block_start_ptr, 2);

        // Get balance flags (negative balance, etc.), little-endian 2 bytes at sector 8 block 1, bytes 2-3
        bip_data.flags = bit_lib_bytes_to_num_le(block_start_ptr + 2, 2);

        // Get trip time window, proprietary format, at sector 5 block 1, bytes 0-7
        const uint8_t trip_time_window_start_block_num =
            mf_classic_get_first_block_num_of_sector(BIP_TRIP_TIME_WINDOW_SECTOR_NUMBER);
        const MfClassicBlock* trip_window_block_ptr =
            &data->block[trip_time_window_start_block_num + 1];

        bip_parse_datetime(trip_window_block_ptr, &bip_data.trip_time_window);

        // Last 3 top-ups: sector 10, ring-buffer of 3 blocks, timestamp in bytes 0-7, amount in bytes 9-10
        const uint8_t top_ups_start_block_num =
            mf_classic_get_first_block_num_of_sector(BIP_LAST_TOP_UPS_SECTOR_NUMBER);
        for(size_t i = 0; i < 3; i++) {
            const MfClassicBlock* block = &data->block[top_ups_start_block_num + i];

            if(is_bip_block_empty(block)) continue;

            BipTransaction* top_up = &bip_data.top_ups[i];
            bip_parse_datetime(block, &top_up->datetime);

            top_up->amount = bit_lib_bytes_to_num_le(&block->data[9], 2) >> 2;
        }

        // Last 3 charges (i.e. trips), sector 11, ring-buffer of 3 blocks, timestamp in bytes 0-7, amount in bytes 10-11
        const uint8_t trips_start_block_num =
            mf_classic_get_first_block_num_of_sector(BIP_TRIPS_INFO_SECTOR_NUMBER);
        for(size_t i = 0; i < 3; i++) {
            const MfClassicBlock* block = &data->block[trips_start_block_num + i];

            if(is_bip_block_empty(block)) continue;

            BipTransaction* charge = &bip_data.charges[i];
            bip_parse_datetime(block, &charge->datetime);

            charge->amount = bit_lib_bytes_to_num_le(&block->data[10], 2) >> 2;
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

        bip_print_datetime(&bip_data.trip_time_window, parsed_data);

        // Find newest top-up
        size_t newest_top_up = 0;
        for(size_t i = 1; i < 3; i++) {
            const DateTime* newest = &bip_data.top_ups[newest_top_up].datetime;
            const DateTime* current = &bip_data.top_ups[i].datetime;
            if(datetime_cmp(current, newest) > 0) {
                newest_top_up = i;
            }
        }

        // Print top-ups, newest first
        furi_string_cat_printf(parsed_data, "\n\e#Last Top-ups");
        for(size_t i = 0; i < 3; i++) {
            const BipTransaction* top_up = &bip_data.top_ups[(3u + newest_top_up - i) % 3];
            furi_string_cat_printf(parsed_data, "\n+$%d\n  @", top_up->amount);
            bip_print_datetime(&top_up->datetime, parsed_data);
        }

        // Find newest charge
        size_t newest_charge = 0;
        for(size_t i = 1; i < 3; i++) {
            const DateTime* newest = &bip_data.charges[newest_charge].datetime;
            const DateTime* current = &bip_data.charges[i].datetime;
            if(datetime_cmp(current, newest) > 0) {
                newest_charge = i;
            }
        }

        // Print charges
        furi_string_cat_printf(parsed_data, "\n\e#Last Charges (Trips)");
        for(size_t i = 0; i < 3; i++) {
            const BipTransaction* charge = &bip_data.charges[(3u + newest_charge - i) % 3];
            furi_string_cat_printf(parsed_data, "\n-$%d\n  @", charge->amount);
            bip_print_datetime(&charge->datetime, parsed_data);
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
