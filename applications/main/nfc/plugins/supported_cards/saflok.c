// KDF from: https://gitee.com/jadenwu/Saflok_KDF/blob/master/saflok.c
// KDF published and reverse engineered by Jaden Wu
// FZ plugin by @noproto

// Decryption and parsing from: https://gitee.com/wangshuoyue/unsaflok
// Decryption algorithm and parsing published by Shuoyue Wang
// Parsing also inspired by Lennert Wouters and Ian Carroll's DEFCON 32 talk
// https://defcon.org/html/defcon-32/dc-32-speakers.html
// FZ parser by @Torron, with help from @xtruan, @zacharyweiss, @evilmog and kara (@Arkwin)
#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>

#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <nfc_app_i.h>
#include <bit_lib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG "Saflok"

#define MAGIC_TABLE_SIZE      192
#define KEY_LENGTH            6
#define UID_LENGTH            4
#define CHECK_SECTOR          1
#define BASIC_ACCESS_BYTE_NUM 17
#define SAFLOK_YEAR_OFFSET    1980

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    uint8_t level_num;
    char* level_name;
} SaflokKeyLevel;

static SaflokKeyLevel key_levels[] = {
    {1, "Guest Key"},
    {2, "Connectors"},
    {3, "Suite"},
    {4, "Limited Use"},
    {5, "Failsafe"},
    {6, "Inhibit"},
    {7, "Pool/Meeting Master"},
    {8, "Housekeeping"},
    {9, "Floor Key"},
    {10, "Section Key"},
    {11, "Rooms Master"},
    {12, "Grand Master"},
    {13, "Emergency"},
    {14, "Electronic Lockout"},
    {15, "Secondary Programming Key (SPK)"},
    {16, "Primary Programming Key (PPK)"},
};

const char* weekdays[] =
    {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

static MfClassicKeyPair saflok_1k_keys[] = {
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 000
    {.a = 0x2a2c13cc242a, .b = 0xffffffffffff}, // 001
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, // 002
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, // 003
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 004
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 005
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 006
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 007
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 008
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 009
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 010
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 011
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 012
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 013
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 014
    {.a = 0x000000000000, .b = 0xffffffffffff}, // 015
};

void generate_saflok_key(const uint8_t* uid, uint8_t* key) {
    static const uint8_t magic_table[MAGIC_TABLE_SIZE] = {
        0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0xF0, 0x57, 0xB3, 0x9E, 0xE3, 0xD8, 0x00, 0x00, 0xAA,
        0x00, 0x00, 0x00, 0x96, 0x9D, 0x95, 0x4A, 0xC1, 0x57, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00,
        0x8F, 0x43, 0x58, 0x0D, 0x2C, 0x9D, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0xFF, 0xCC, 0xE0,
        0x05, 0x0C, 0x43, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x34, 0x1B, 0x15, 0xA6, 0x90, 0xCC,
        0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x89, 0x58, 0x56, 0x12, 0xE7, 0x1B, 0x00, 0x00, 0xAA,
        0x00, 0x00, 0x00, 0xBB, 0x74, 0xB0, 0x95, 0x36, 0x58, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00,
        0xFB, 0x97, 0xF8, 0x4B, 0x5B, 0x74, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0xC9, 0xD1, 0x88,
        0x35, 0x9F, 0x92, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x8F, 0x92, 0xE9, 0x7F, 0x58, 0x97,
        0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x16, 0x6C, 0xA2, 0xB0, 0x9F, 0xD1, 0x00, 0x00, 0xAA,
        0x00, 0x00, 0x00, 0x27, 0xDD, 0x93, 0x10, 0x1C, 0x6C, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00,
        0xDA, 0x3E, 0x3F, 0xD6, 0x49, 0xDD, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x58, 0xDD, 0xED,
        0x07, 0x8E, 0x3E, 0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x5C, 0xD0, 0x05, 0xCF, 0xD9, 0x07,
        0x00, 0x00, 0xAA, 0x00, 0x00, 0x00, 0x11, 0x8D, 0xD0, 0x01, 0x87, 0xD0};

    uint8_t magic_byte = (uid[3] >> 4) + (uid[2] >> 4) + (uid[0] & 0x0F);
    uint8_t magickal_index = (magic_byte & 0x0F) * 12 + 11;

    uint8_t temp_key[KEY_LENGTH] = {magic_byte, uid[0], uid[1], uid[2], uid[3], magic_byte};
    uint8_t carry_sum = 0;

    for(int i = KEY_LENGTH - 1; i >= 0; i--, magickal_index--) {
        uint16_t keysum = temp_key[i] + magic_table[magickal_index] + carry_sum;
        temp_key[i] = (keysum & 0xFF);
        carry_sum = keysum >> 8;
    }

    memcpy(key, temp_key, KEY_LENGTH);
}

// Lookup table
static const uint8_t c_aDecode[256] = {
    0xEA, 0x0D, 0xD9, 0x74, 0x4E, 0x28, 0xFD, 0xBA, 0x7B, 0x98, 0x87, 0x78, 0xDD, 0x8D, 0xB5,
    0x1A, 0x0E, 0x30, 0xF3, 0x2F, 0x6A, 0x3B, 0xAC, 0x09, 0xB9, 0x20, 0x6E, 0x5B, 0x2B, 0xB6,
    0x21, 0xAA, 0x17, 0x44, 0x5A, 0x54, 0x57, 0xBE, 0x0A, 0x52, 0x67, 0xC9, 0x50, 0x35, 0xF5,
    0x41, 0xA0, 0x94, 0x60, 0xFE, 0x24, 0xA2, 0x36, 0xEF, 0x1E, 0x6B, 0xF7, 0x9C, 0x69, 0xDA,
    0x9B, 0x6F, 0xAD, 0xD8, 0xFB, 0x97, 0x62, 0x5F, 0x1F, 0x38, 0xC2, 0xD7, 0x71, 0x31, 0xF0,
    0x13, 0xEE, 0x0F, 0xA3, 0xA7, 0x1C, 0xD5, 0x11, 0x4C, 0x45, 0x2C, 0x04, 0xDB, 0xA6, 0x2E,
    0xF8, 0x64, 0x9A, 0xB8, 0x53, 0x66, 0xDC, 0x7A, 0x5D, 0x03, 0x07, 0x80, 0x37, 0xFF, 0xFC,
    0x06, 0xBC, 0x26, 0xC0, 0x95, 0x4A, 0xF1, 0x51, 0x2D, 0x22, 0x18, 0x01, 0x79, 0x5E, 0x76,
    0x1D, 0x7F, 0x14, 0xE3, 0x9E, 0x8A, 0xBB, 0x34, 0xBF, 0xF4, 0xAB, 0x48, 0x63, 0x55, 0x3E,
    0x56, 0x8C, 0xD1, 0x12, 0xED, 0xC3, 0x49, 0x8E, 0x92, 0x9D, 0xCA, 0xB1, 0xE5, 0xCE, 0x4D,
    0x3F, 0xFA, 0x73, 0x05, 0xE0, 0x4B, 0x93, 0xB2, 0xCB, 0x08, 0xE1, 0x96, 0x19, 0x3D, 0x83,
    0x39, 0x75, 0xEC, 0xD6, 0x3C, 0xD0, 0x70, 0x81, 0x16, 0x29, 0x15, 0x6C, 0xC7, 0xE7, 0xE2,
    0xF6, 0xB7, 0xE8, 0x25, 0x6D, 0x3A, 0xE6, 0xC8, 0x99, 0x46, 0xB0, 0x85, 0x02, 0x61, 0x1B,
    0x8B, 0xB3, 0x9F, 0x0B, 0x2A, 0xA8, 0x77, 0x10, 0xC1, 0x88, 0xCC, 0xA4, 0xDE, 0x43, 0x58,
    0x23, 0xB4, 0xA1, 0xA5, 0x5C, 0xAE, 0xA9, 0x7E, 0x42, 0x40, 0x90, 0xD2, 0xE9, 0x84, 0xCF,
    0xE4, 0xEB, 0x47, 0x4F, 0x82, 0xD4, 0xC5, 0x8F, 0xCD, 0xD3, 0x86, 0x00, 0x59, 0xDF, 0xF2,
    0x0C, 0x7C, 0xC6, 0xBD, 0xF9, 0x7D, 0xC4, 0x91, 0x27, 0x89, 0x32, 0x72, 0x33, 0x65, 0x68,
    0xAF};

void DecryptCard(
    uint8_t strCard[BASIC_ACCESS_BYTE_NUM],
    int length,
    uint8_t decryptedCard[BASIC_ACCESS_BYTE_NUM]) {
    int i, num, num2, num3, num4, b = 0, b2 = 0;
    for(i = 0; i < length; i++) {
        num = c_aDecode[strCard[i]] - (i + 1);
        if(num < 0) num += 256;
        decryptedCard[i] = num;
    }

    if(length == 17) {
        b = decryptedCard[10];
        b2 = b & 1;
    }

    for(num2 = length; num2 > 0; num2--) {
        b = decryptedCard[num2 - 1];
        for(num3 = 8; num3 > 0; num3--) {
            num4 = num2 + num3;
            if(num4 > length) num4 -= length;
            int b3 = decryptedCard[num4 - 1];
            int b4 = (b3 & 0x80) >> 7;
            b3 = ((b3 << 1) & 0xFF) | b2;
            b2 = (b & 0x80) >> 7;
            b = ((b << 1) & 0xFF) | b4;
            decryptedCard[num4 - 1] = b3;
        }
        decryptedCard[num2 - 1] = b;
    }
}

uint8_t CalculateCheckSum(uint8_t data[BASIC_ACCESS_BYTE_NUM]) {
    int sum = 0;
    for(int i = 0; i < BASIC_ACCESS_BYTE_NUM - 1; i++) {
        sum += data[i];
    }
    sum = 255 - (sum & 0xFF);
    return sum & 0xFF;
}

static bool saflok_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t block_num = mf_classic_get_first_block_num_of_sector(CHECK_SECTOR);
        FURI_LOG_D(TAG, "Saflok: Verifying sector %i", CHECK_SECTOR);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(saflok_1k_keys[CHECK_SECTOR].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Saflok: Failed to read block %u: %d", block_num, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool saflok_read(Nfc* nfc, NfcDevice* device) {
    FURI_LOG_D(TAG, "Entering Saflok KDF");

    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type = MfClassicType1k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;
        data->type = type;

        size_t uid_len;
        const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
        FURI_LOG_D(
            TAG, "Saflok: UID identified: %02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
        if(uid_len != UID_LENGTH) break;

        uint8_t key[KEY_LENGTH];
        generate_saflok_key(uid, key);
        uint64_t num_key = bit_lib_bytes_to_num_be(key, KEY_LENGTH);
        FURI_LOG_D(TAG, "Saflok: Key generated for UID: %012llX", num_key);

        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            if(saflok_1k_keys[i].a == 0x000000000000) {
                saflok_1k_keys[i].a = num_key;
            }
        }

        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(saflok_1k_keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(saflok_1k_keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
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

bool saflok_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;

    do {
        // Check card type
        if(data->type != MfClassicType1k) break;

        // Verify key
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, CHECK_SECTOR);

        const uint64_t key_a =
            bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        if(key_a != saflok_1k_keys[CHECK_SECTOR].a) break;

        // Decrypt basic access
        uint8_t basicAccess[BASIC_ACCESS_BYTE_NUM];
        memcpy(&basicAccess, &data->block[1].data, 16);
        memcpy(&basicAccess[16], &data->block[2].data[0], 1);
        uint8_t decodedBA[BASIC_ACCESS_BYTE_NUM];
        DecryptCard(basicAccess, BASIC_ACCESS_BYTE_NUM, decodedBA);

        // Byte 0: Key level, LED warning bit, and subgroup functions
        uint8_t key_level = (decodedBA[0] & 0xF0) >> 4;
        uint8_t led_warning = (decodedBA[0] & 0x08) >> 3;

        // Byte 1: Key ID
        uint8_t key_id = decodedBA[1];

        // Byte 2 & 3: KeyRecord, including OpeningKey flag
        uint8_t key_record_high = decodedBA[2] & 0x7F;
        uint8_t opening_key = (decodedBA[2] & 0x80) >> 7;
        uint16_t key_record = (key_record_high << 8) | decodedBA[3];

        // Byte 4 & 5: Pass level in reversed binary
        // This part is commented because the relevance of this info is still unknown
        // uint16_t pass_level = ((decodedBA[4] & 0xFF) << 8) | decodedBA[5];
        // uint8_t pass_levels[12];
        // int pass_levels_count = 0;

        // for (int i = 0; i < 12; i++) {
        //     if ((pass_level >> i) & 1) {
        //         pass_levels[pass_levels_count++] = i + 1;
        //     }
        // }

        // Byte 5 & 6: EncryptSequence + Combination
        uint16_t sequence_combination_number = ((decodedBA[5] & 0x0F) << 8) | decodedBA[6];
        // Bytes 14-15: Property number and year
        uint8_t creation_year_bits = (decodedBA[14] & 0xF0);
        uint16_t property_id = ((decodedBA[14] & 0x0F) << 8) | decodedBA[15];

        // Byte 7: OverrideDeadbolt and Days
        uint8_t override_deadbolt = (decodedBA[7] & 0x80) >> 7;
        uint8_t restricted_weekday = decodedBA[7] & 0x7F;
        // Counter to keep track of the number of restricted days
        int restricted_count = 0;
        // Buffer to store the resulting string
        FuriString* restricted_weekday_string = furi_string_alloc();
        // Check each bit from Monday to Sunday
        for(int i = 0; i < 7; i++) {
            if(restricted_weekday & (0b01000000 >> i)) {
                // If the bit is set, append the corresponding weekday to the buffer
                if(restricted_count > 0) {
                    furi_string_cat_printf(restricted_weekday_string, ", ");
                }
                furi_string_cat_printf(restricted_weekday_string, "%s", weekdays[i]);
                restricted_count++;
            }
        }

        // Determine if all weekdays are restricted
        if(restricted_weekday == 0b01111100) {
            furi_string_printf(restricted_weekday_string, "weekdays");
        }
        // If there are specific restricted days
        else if(restricted_weekday == 0b00000011) {
            furi_string_printf(restricted_weekday_string, "weekends");
        }
        // If no weekdays are restricted
        else if(restricted_weekday == 0) {
            furi_string_printf(restricted_weekday_string, "none");
        }

        // Bytes 8-10: Expiry interval
        uint16_t interval_year = (decodedBA[8] >> 4);
        uint8_t interval_month = decodedBA[8] & 0x0F;
        uint8_t interval_day = (decodedBA[9] >> 3) & 0x1F;
        uint8_t interval_hour = ((decodedBA[9] & 0x07) << 2) | (decodedBA[10] >> 6);
        uint8_t interval_minute = decodedBA[10] & 0x3F;

        // Bytes 11-13: Creation date since 1980 Jan 1st
        uint16_t creation_year = (((decodedBA[11] & 0xF0) >> 4) + SAFLOK_YEAR_OFFSET) |
                                 creation_year_bits;
        uint8_t creation_month = decodedBA[11] & 0x0F;
        uint8_t creation_day = (decodedBA[12] >> 3) & 0x1F;
        uint8_t creation_hour = ((decodedBA[12] & 0x07) << 2) | (decodedBA[13] >> 6);
        uint8_t creation_minute = decodedBA[13] & 0x3F;

        uint16_t expire_year = creation_year + interval_year;
        uint8_t expire_month = creation_month + interval_month;
        uint8_t expire_day = creation_day + interval_day;
        uint8_t expire_hour = interval_hour;
        uint8_t expire_minute = interval_minute;

        // Handle month rollover
        while(expire_month > 12) {
            expire_month -= 12;
            expire_year++;
        }

        // Handle day rollover
        static const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        while(true) {
            uint8_t max_days = days_in_month[expire_month - 1];
            // Adjust for leap years
            if(expire_month == 2 &&
               (expire_year % 4 == 0 && (expire_year % 100 != 0 || expire_year % 400 == 0))) {
                max_days = 29;
            }
            if(expire_day <= max_days) {
                break;
            }
            expire_day -= max_days;
            expire_month++;
            if(expire_month > 12) {
                expire_month = 1;
                expire_year++;
            }
        }

        // Byte 16: Checksum
        uint8_t checksum = decodedBA[16];
        uint8_t checksum_calculated = CalculateCheckSum(decodedBA);
        bool checksum_valid = (checksum_calculated == checksum);
        for(int i = 0; i < 17; i++) {
            FURI_LOG_D(TAG, "%02X", decodedBA[i]);
        }
        FURI_LOG_D(TAG, "CS decrypted: %02X", checksum);
        FURI_LOG_D(TAG, "CS calculated: %02X", checksum_calculated);

        furi_string_cat_printf(parsed_data, "\e#Saflok Card\n");
        furi_string_cat_printf(
            parsed_data,
            "Key Level: %u, %s\n",
            key_levels[key_level].level_num,
            key_levels[key_level].level_name);
        furi_string_cat_printf(parsed_data, "LED Exp. Warning: %s\n", led_warning ? "Yes" : "No");
        furi_string_cat_printf(parsed_data, "Key ID: %02X\n", key_id);
        furi_string_cat_printf(parsed_data, "Key Record: %04X\n", key_record);
        furi_string_cat_printf(parsed_data, "Opening key: %s\n", opening_key ? "Yes" : "No");
        furi_string_cat_printf(
            parsed_data, "Seq. & Combination: %04X\n", sequence_combination_number);
        furi_string_cat_printf(
            parsed_data, "Override Deadbolt: %s\n", override_deadbolt ? "Yes" : "No");
        furi_string_cat_printf(
            parsed_data,
            "Restricted Weekday: %s\n",
            furi_string_get_cstr(restricted_weekday_string));
        furi_string_cat_printf(
            parsed_data,
            "Valid Start Date: \n%u-%02d-%02d\n%02d:%02d:00\n",
            creation_year,
            creation_month,
            creation_day,
            creation_hour,
            creation_minute);
        furi_string_cat_printf(
            parsed_data,
            "Expiration Date: \n%u-%02d-%02d\n%02d:%02d:00\n",
            expire_year,
            expire_month,
            expire_day,
            expire_hour,
            expire_minute);
        furi_string_cat_printf(parsed_data, "Property Number: %u\n", property_id);
        furi_string_cat_printf(parsed_data, "Checksum Valid: %s", checksum_valid ? "Yes" : "No");
        parsed = true;
    } while(false);
    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin saflok_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = saflok_verify,
    .read = saflok_read,
    .parse = saflok_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor saflok_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &saflok_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* saflok_plugin_ep(void) {
    return &saflok_plugin_descriptor;
}
