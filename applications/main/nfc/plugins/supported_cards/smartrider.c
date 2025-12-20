#include "nfc_supported_card_plugin.h"
#include <bit_lib.h>
#include <flipper_application.h>
#include <furi.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <string.h>

#define MAX_TRIPS           10
#define TAG                 "SmartRider"
#define MAX_BLOCKS          64
#define MAX_DATE_ITERATIONS 366

static const uint8_t STANDARD_KEYS[3][6] = {
    {0x20, 0x31, 0xD1, 0xE5, 0x7A, 0x3B},
    {0x4C, 0xA6, 0x02, 0x9F, 0x94, 0x73},
    {0x19, 0x19, 0x53, 0x98, 0xE3, 0x2F}};

typedef struct {
    uint32_t timestamp;
    uint16_t cost;
    uint16_t transaction_number;
    uint16_t journey_number;
    char route[5];
    uint8_t tap_on : 1;
    uint8_t block;
} __attribute__((packed)) TripData;

typedef struct {
    uint32_t balance;
    uint16_t issued_days;
    uint16_t expiry_days;
    uint16_t purchase_cost;
    uint16_t auto_load_threshold;
    uint16_t auto_load_value;
    char card_serial_number[11];
    uint8_t token;
    TripData trips[MAX_TRIPS];
    uint8_t trip_count;
} __attribute__((packed)) SmartRiderData;

static const char* const CONCESSION_TYPES[] = {
    "Pre-issue",
    "Standard Fare",
    "Student",
    NULL,
    "Tertiary",
    NULL,
    "Seniors",
    "Health Care",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "PTA Staff",
    "Pensioner",
    "Free Travel"};

static inline const char* get_concession_type(uint8_t token) {
    return (token <= 0x10) ? CONCESSION_TYPES[token] : "Unknown";
}

static bool authenticate_and_read(
    Nfc* nfc,
    uint8_t sector,
    const uint8_t* key,
    MfClassicKeyType key_type,
    MfClassicBlock* block_data) {
    MfClassicKey mf_key;
    memcpy(mf_key.data, key, 6);
    uint8_t block = mf_classic_get_first_block_num_of_sector(sector);

    if(mf_classic_poller_sync_auth(nfc, block, &mf_key, key_type, NULL) != MfClassicErrorNone) {
        FURI_LOG_D(TAG, "Authentication failed for sector %d key type %d", sector, key_type);
        return false;
    }

    if(mf_classic_poller_sync_read_block(nfc, block, &mf_key, key_type, block_data) !=
       MfClassicErrorNone) {
        FURI_LOG_D(TAG, "Read failed for sector %d", sector);
        return false;
    }

    return true;
}

static bool smartrider_verify(Nfc* nfc) {
    furi_assert(nfc);
    MfClassicBlock block_data;

    for(int i = 0; i < 3; i++) {
        if(!authenticate_and_read(
               nfc,
               i * 6,
               STANDARD_KEYS[i],
               i % 2 == 0 ? MfClassicKeyTypeA : MfClassicKeyTypeB,
               &block_data) ||
           memcmp(block_data.data, STANDARD_KEYS[i], 6) != 0) {
            FURI_LOG_D(TAG, "Authentication or key mismatch for key %d", i);
            return false;
        }
    }

    FURI_LOG_I(TAG, "SmartRider card verified");
    return true;
}

static inline bool
    parse_trip_data(const MfClassicBlock* block_data, TripData* trip, uint8_t block_number) {
    trip->timestamp = bit_lib_bytes_to_num_le(block_data->data + 3, 4);
    trip->tap_on = (block_data->data[7] & 0x10) == 0x10;
    memcpy(trip->route, block_data->data + 8, 4);
    trip->route[4] = '\0';
    trip->cost = bit_lib_bytes_to_num_le(block_data->data + 13, 2);
    trip->transaction_number = bit_lib_bytes_to_num_le(block_data->data, 2);
    trip->journey_number = bit_lib_bytes_to_num_le(block_data->data + 2, 2);
    trip->block = block_number;
    return true;
}

static bool smartrider_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);
    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    MfClassicType type;
    if(mf_classic_poller_sync_detect_type(nfc, &type) != MfClassicErrorNone ||
       type != MfClassicType1k) {
        mf_classic_free(data);
        return false;
    }
    data->type = type;

    MfClassicDeviceKeys keys = {.key_a_mask = 0, .key_b_mask = 0};
    for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
        memcpy(keys.key_a[i].data, STANDARD_KEYS[i == 0 ? 0 : 1], sizeof(STANDARD_KEYS[0]));
        if(i > 0) {
            memcpy(keys.key_b[i].data, STANDARD_KEYS[2], sizeof(STANDARD_KEYS[0]));
            FURI_BIT_SET(keys.key_b_mask, i);
        }
        FURI_BIT_SET(keys.key_a_mask, i);
    }

    MfClassicError error = mf_classic_poller_sync_read(nfc, &keys, data);
    if(error != MfClassicErrorNone) {
        FURI_LOG_W(TAG, "Failed to read data");
        mf_classic_free(data);
        return false;
    }

    nfc_device_set_data(device, NfcProtocolMfClassic, data);
    mf_classic_free(data);
    return true;
}

static bool is_leap_year(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static void calculate_date(uint32_t timestamp, char* date_str, size_t date_str_size) {
    uint32_t seconds_since_2000 = timestamp;
    uint32_t days_since_2000 = seconds_since_2000 / 86400;
    uint16_t year = 2000;
    uint8_t month = 1;
    uint16_t day = 1;

    static const uint16_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    while(days_since_2000 >= (is_leap_year(year) ? 366 : 365)) {
        days_since_2000 -= (is_leap_year(year) ? 366 : 365);
        year++;
    }

    for(month = 0; month < 12; month++) {
        uint16_t dim = days_in_month[month];
        if(month == 1 && is_leap_year(year)) {
            dim++;
        }
        if(days_since_2000 < dim) {
            break;
        }
        days_since_2000 -= dim;
    }

    day = days_since_2000 + 1;
    month++; // Adjust month to 1-based

    if(date_str_size > 0) {
        size_t written = 0;
        written += snprintf(date_str + written, date_str_size - written, "%02u", day);
        if(written < date_str_size - 1) {
            written += snprintf(date_str + written, date_str_size - written, "/");
        }
        if(written < date_str_size - 1) {
            written += snprintf(date_str + written, date_str_size - written, "%02u", month);
        }
        if(written < date_str_size - 1) {
            written += snprintf(date_str + written, date_str_size - written, "/");
        }
        if(written < date_str_size - 1) {
            snprintf(date_str + written, date_str_size - written, "%02u", year % 100);
        }
    } else {
        // If the buffer size is 0, do nothing
    }
}

static bool smartrider_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    SmartRiderData sr_data = {0};

    if(data->type != MfClassicType1k) {
        FURI_LOG_E(TAG, "Invalid card type");
        return false;
    }

    const MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 0);
    if(!sec_tr || memcmp(sec_tr->key_a.data, STANDARD_KEYS[0], 6) != 0) {
        FURI_LOG_E(TAG, "Key verification failed for sector 0");
        return false;
    }

    static const uint8_t required_blocks[] = {14, 4, 5, 1, 52, 50, 0};
    for(size_t i = 0; i < COUNT_OF(required_blocks); i++) {
        if(required_blocks[i] >= MAX_BLOCKS ||
           !mf_classic_is_block_read(data, required_blocks[i])) {
            FURI_LOG_E(TAG, "Required block %d is not read or out of range", required_blocks[i]);
            return false;
        }
    }

    sr_data.balance = bit_lib_bytes_to_num_le(data->block[14].data + 7, 2);
    sr_data.issued_days = bit_lib_bytes_to_num_le(data->block[4].data + 16, 2);
    sr_data.expiry_days = bit_lib_bytes_to_num_le(data->block[4].data + 18, 2);
    sr_data.auto_load_threshold = bit_lib_bytes_to_num_le(data->block[4].data + 20, 2);
    sr_data.auto_load_value = bit_lib_bytes_to_num_le(data->block[4].data + 22, 2);
    sr_data.token = data->block[5].data[8];
    sr_data.purchase_cost = bit_lib_bytes_to_num_le(data->block[0].data + 14, 2);

    snprintf(
        sr_data.card_serial_number,
        sizeof(sr_data.card_serial_number),
        "%02X%02X%02X%02X%02X",
        data->block[1].data[6],
        data->block[1].data[7],
        data->block[1].data[8],
        data->block[1].data[9],
        data->block[1].data[10]);

    for(uint8_t block_number = 40; block_number <= 52 && sr_data.trip_count < MAX_TRIPS;
        block_number++) {
        if((block_number != 43 && block_number != 47 && block_number != 51) &&
           mf_classic_is_block_read(data, block_number) &&
           parse_trip_data(
               &data->block[block_number], &sr_data.trips[sr_data.trip_count], block_number)) {
            sr_data.trip_count++;
        }
    }

    // Sort trips by timestamp (descending order)
    for(uint8_t i = 0; i < sr_data.trip_count - 1; i++) {
        for(uint8_t j = 0; j < sr_data.trip_count - i - 1; j++) {
            if(sr_data.trips[j].timestamp < sr_data.trips[j + 1].timestamp) {
                TripData temp = sr_data.trips[j];
                sr_data.trips[j] = sr_data.trips[j + 1];
                sr_data.trips[j + 1] = temp;
            }
        }
    }

    furi_string_printf(
        parsed_data,
        "\e#SmartRider\nBalance: $%lu.%02lu\nConcession: %s\nSerial: %s%s\n"
        "Total Cost: $%u.%02u\nAuto-Load: $%u.%02u/$%u.%02u\n\e#Tag On/Off History\n",
        sr_data.balance / 100,
        sr_data.balance % 100,
        get_concession_type(sr_data.token),
        memcmp(sr_data.card_serial_number, "00", 2) == 0 ? "SR0" : "",
        memcmp(sr_data.card_serial_number, "00", 2) == 0 ? sr_data.card_serial_number + 2 :
                                                           sr_data.card_serial_number,
        sr_data.purchase_cost / 100,
        sr_data.purchase_cost % 100,
        sr_data.auto_load_threshold / 100,
        sr_data.auto_load_threshold % 100,
        sr_data.auto_load_value / 100,
        sr_data.auto_load_value % 100);

    for(uint8_t i = 0; i < sr_data.trip_count; i++) {
        char date_str[9];
        calculate_date(sr_data.trips[i].timestamp, date_str, sizeof(date_str));

        uint32_t cost = sr_data.trips[i].cost;
        if(cost > 0) {
            furi_string_cat_printf(
                parsed_data,
                "%s %c $%lu.%02lu %s\n",
                date_str,
                sr_data.trips[i].tap_on ? '+' : '-',
                cost / 100,
                cost % 100,
                sr_data.trips[i].route);
        } else {
            furi_string_cat_printf(
                parsed_data,
                "%s %c %s\n",
                date_str,
                sr_data.trips[i].tap_on ? '+' : '-',
                sr_data.trips[i].route);
        }
    }

    return true;
}
static const NfcSupportedCardsPlugin smartrider_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = smartrider_verify,
    .read = smartrider_read,
    .parse = smartrider_parse,
};

__attribute__((used)) const FlipperAppPluginDescriptor* smartrider_plugin_ep() {
    static const FlipperAppPluginDescriptor plugin_descriptor = {
        .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
        .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
        .entry_point = &smartrider_plugin,
    };
    return &plugin_descriptor;
}

// made with love by jay candel <3
