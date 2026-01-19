#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <datetime.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_format/flipper_format.h>
//#include <locale/locale.h>

#define TAG "Plantain"
static const char* nfc_resources_header = "Flipper NFC resources";
static const uint32_t nfc_resources_file_version = 1;

void from_minutes_to_datetime(uint32_t minutes, DateTime* datetime, uint16_t start_year) {
    uint32_t timestamp = minutes * 60;
    DateTime start_datetime = {0};
    start_datetime.year = start_year - 1;
    start_datetime.month = 12;
    start_datetime.day = 31;
    timestamp += datetime_datetime_to_timestamp(&start_datetime);
    datetime_timestamp_to_datetime(timestamp, datetime);
}

static bool
    sz_UIC_to_sta(Storage* storage, const char* file_name, FuriString* key, FuriString* data) {
    bool parsed = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* temp_str;
    temp_str = furi_string_alloc();

    do {
        // Open file
        if(!flipper_format_file_open_existing(file, file_name)) break;
        // Read file header and version
        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(furi_string_cmp_str(temp_str, nfc_resources_header) ||
           (version != nfc_resources_file_version))
            break;
        flipper_format_read_string(file, furi_string_get_cstr(key), data);

        parsed = true;
    } while(false);

    furi_string_free(temp_str);
    flipper_format_free(file);
    return parsed;
}

bool sz_uic_search(Storage* storage, uint16_t uic, FuriString* name) {
    bool parsed = false;
    FuriString* key;

    key = furi_string_alloc_printf("%04X", uic);

    sz_UIC_to_sta(storage, EXT_PATH("nfc/assets/szppk_id.nfc"), key, name);
    parsed = true;
    furi_string_free(key);

    return parsed;
}

bool parse_ticket_data(
    FuriString* parsed_data,
    Storage* storage,
    uint16_t departure_uic,
    uint16_t destination_uic,
    FuriString* departure_name,
    FuriString* destination_name,
    uint8_t value_data,
    uint8_t current_status,
    uint16_t valid_from_date,
    uint16_t valid_till_date,
    uint32_t tap_data,
    DateTime v_from,
    DateTime v_till,
    DateTime tap_time,
    uint8_t second_ticket_marker,
    uint8_t sys_n_arr[],
    uint8_t direction) {
    bool parsed = false;
    uint32_t valid_from_timestamp = 946684800 + valid_from_date * 24 * 60 * 60;
    uint32_t valid_till_timestamp = 946684800 + valid_till_date * 24 * 60 * 60;
    uint32_t tap_timestamp = 1388530800 + tap_data * 60;
    datetime_timestamp_to_datetime(valid_from_timestamp, &v_from);
    datetime_timestamp_to_datetime(valid_till_timestamp, &v_till);
    datetime_timestamp_to_datetime(tap_timestamp, &tap_time);
    uint64_t sys_n = 0;
    for(size_t i = 0; i < 6; i++) {
        sys_n = (sys_n << 8) | sys_n_arr[i];
    }

    sz_uic_search(storage, departure_uic, departure_name);
    if(furi_string_utf8_length(departure_name) <= 2)
        furi_string_printf(departure_name, "1E%04X", departure_uic);
    sz_uic_search(storage, destination_uic, destination_name);
    if(furi_string_utf8_length(destination_name) <= 2)
        furi_string_printf(destination_name, "1E%04X", destination_uic);

    if(second_ticket_marker == 0)
        furi_string_cat_printf(parsed_data, "\e#SZPPK Ticket:\n");
    else
        furi_string_cat_printf(parsed_data, "\e#Second Ticket:\n");

    furi_string_cat_printf(
        parsed_data,
        "From:> %s\nTo: %s\n",
        furi_string_get_cstr(departure_name),
        furi_string_get_cstr(destination_name));
    if(valid_from_timestamp == valid_till_timestamp)
        furi_string_cat_printf(
            parsed_data, "Valid on: %02d-%02d-%04d\n", v_from.day, v_from.month, v_from.year);
    else
        furi_string_cat_printf(
            parsed_data,
            "Valid from: %02d-%02d-%04d\nValid thru:  %02d-%02d-%04d\n",
            v_from.day,
            v_from.month,
            v_from.year,
            v_till.day,
            v_till.month,
            v_till.year);
    if(direction == 2) {
        furi_string_cat_printf(parsed_data, "Direction: Round-trip  <<-->>\n");
    } else {
        furi_string_cat_printf(parsed_data, "Direction: One-way ->>\n");
    }

    furi_string_cat_printf(parsed_data, "Rides remain: %02d\n", value_data);
    if(current_status == 0x00)
        furi_string_cat_printf(parsed_data, "Status:> TICKET IS READY\n");
    else if(current_status == 0x80)
        furi_string_cat_printf(
            parsed_data,
            "Status:> ENTERED STATION\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\n\n",
            tap_time.day,
            tap_time.month,
            tap_time.year,
            tap_time.hour,
            tap_time.minute);
    else if(current_status == 0x1E)
        furi_string_cat_printf(
            parsed_data,
            "Status:> EXITED STATION\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\n\n",
            tap_time.day,
            tap_time.month,
            tap_time.year,
            tap_time.hour,
            tap_time.minute);
    else
        furi_string_cat_printf(parsed_data, "Status:> UNKNOWN (%02X)\n", current_status);

    furi_string_cat_printf(parsed_data, "SYS N:> %013lld\n", sys_n);

    parsed = true;

    return parsed;
}

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    const MfClassicKeyPair* keys;
    uint32_t data_sector;
} PlantainCardConfig;

static const MfClassicKeyPair plantain_1k_keys[] = {
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //0
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //1
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //2
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //3
    {.a = 0xe56ac127dd45, .b = 0x19fc84a3784b}, //4
    {.a = 0x77dabc9825e1, .b = 0x9764fec3154a}, //5
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //6
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //7
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7}, //8
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3}, //9
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb}, //10
    {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56}, //11
    {.a = 0xacffffffffff, .b = 0x71f3a315ad26}, //12
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //13
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //14
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //15
    {.a = 0x72f96bdd3714, .b = 0x462225cd34cf}, //16
    {.a = 0x044ce1872bc3, .b = 0x8c90c70cff4a}, //17
    {.a = 0xbc2d1791dec1, .b = 0xca96a487de0b}, //18
    {.a = 0x8791b2ccb5c4, .b = 0xc956c3b80da3}, //19
    {.a = 0x8e26e45e7d65, .b = 0x8e65b3af7d22}, //20
    {.a = 0x0f318130ed18, .b = 0x0c420a20e056}, //21
    {.a = 0x045ceca15535, .b = 0x31bec3d9e510}, //22
    {.a = 0x9d993c5d4ef4, .b = 0x86120e488abf}, //23
    {.a = 0xc65d4eaa645b, .b = 0xb69d40d1a439}, //24
    {.a = 0x3a8a139c20b4, .b = 0x8818a9c5d406}, //25
    {.a = 0xbaff3053b496, .b = 0x4b7cb25354d3}, //26
    {.a = 0x7413b599c4ea, .b = 0xb0a2AAF3A1BA}, //27
    {.a = 0x7413b599c4ea, .b = 0xb0a2aaf3a1ba}, //28
    {.a = 0x0ce7cd2cc72b, .b = 0xfa1fbb3f0f1f}, //29
    {.a = 0x0eb23cc8110b, .b = 0x04dc35277635}, //30
    {.a = 0xbc4580b7f20b, .b = 0xd0a4131fb290}, //31

};

static const MfClassicKeyPair plantain_4k_keys[] = {
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //0
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //1
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //2
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //3
    {.a = 0xe56ac127dd45, .b = 0x19fc84a3784b}, //4
    {.a = 0x77dabc9825e1, .b = 0x9764fec3154a}, //5
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //6
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //7
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7}, //8
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3}, //9
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb}, //10
    {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56}, //11
    {.a = 0xacffffffffff, .b = 0x71f3a315ad26}, //12
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //13
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //14
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //15
    {.a = 0x72f96bdd3714, .b = 0x462225cd34cf}, //16
    {.a = 0x044ce1872bc3, .b = 0x8c90c70cff4a}, //17
    {.a = 0xbc2d1791dec1, .b = 0xca96a487de0b}, //18
    {.a = 0x8791b2ccb5c4, .b = 0xc956c3b80da3}, //19
    {.a = 0x8e26e45e7d65, .b = 0x8e65b3af7d22}, //20
    {.a = 0x0f318130ed18, .b = 0x0c420a20e056}, //21
    {.a = 0x045ceca15535, .b = 0x31bec3d9e510}, //22
    {.a = 0x9d993c5d4ef4, .b = 0x86120e488abf}, //23
    {.a = 0xc65d4eaa645b, .b = 0xb69d40d1a439}, //24
    {.a = 0x3a8a139c20b4, .b = 0x8818a9c5d406}, //25
    {.a = 0xbaff3053b496, .b = 0x4b7cb25354d3}, //26
    {.a = 0x7413b599c4ea, .b = 0xb0a2AAF3A1BA}, //27
    {.a = 0x0ce7cd2cc72b, .b = 0xfa1fbb3f0f1f}, //28
    {.a = 0x0be5fac8b06a, .b = 0x6f95887a4fd3}, //29
    {.a = 0x0eb23cc8110b, .b = 0x04dc35277635}, //30
    {.a = 0xbc4580b7f20b, .b = 0xd0a4131fb290}, //31
    {.a = 0x7a396f0d633d, .b = 0xad2bdc097023}, //32
    {.a = 0xa3faa6daff67, .b = 0x7600e889adf9}, //33
    {.a = 0xfd8705e721b0, .b = 0x296fc317a513}, //34
    {.a = 0x22052b480d11, .b = 0xe19504c39461}, //35
    {.a = 0xa7141147d430, .b = 0xff16014fefc7}, //36
    {.a = 0x8a8d88151a00, .b = 0x038b5f9b5a2a}, //37
    {.a = 0xb27addfb64b0, .b = 0x152fd0c420a7}, //38
    {.a = 0x7259fa0197c6, .b = 0x5583698df085}, //39
};

static const MfClassicKeyPair plantain_4k_keys_legacy[] = {
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //0
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //1
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //2
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //3
    {.a = 0xe56ac127dd45, .b = 0x19fc84a3784b}, //4
    {.a = 0x77dabc9825e1, .b = 0x9764fec3154a}, //5
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //6
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //7
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7}, //8
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3}, //9
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb}, //10
    {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56}, //11
    {.a = 0xacffffffffff, .b = 0x71f3a315ad26}, //12
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //13
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //14
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, //15
    {.a = 0x72f96bdd3714, .b = 0x462225cd34cf}, //16
    {.a = 0x044ce1872bc3, .b = 0x8c90c70cff4a}, //17
    {.a = 0xbc2d1791dec1, .b = 0xca96a487de0b}, //18
    {.a = 0x8791b2ccb5c4, .b = 0xc956c3b80da3}, //19
    {.a = 0x8e26e45e7d65, .b = 0x8e65b3af7d22}, //20
    {.a = 0x0f318130ed18, .b = 0x0c420a20e056}, //21
    {.a = 0x045ceca15535, .b = 0x31bec3d9e510}, //22
    {.a = 0x9d993c5d4ef4, .b = 0x86120e488abf}, //23
    {.a = 0xc65d4eaa645b, .b = 0xb69d40d1a439}, //24
    {.a = 0x46d78e850a7e, .b = 0xa470f8130991}, //25
    {.a = 0x42e9b54e51ab, .b = 0x0231b86df52e}, //26
    {.a = 0x0f01ceff2742, .b = 0x6fec74559ca7}, //27
    {.a = 0xb81f2b0c2f66, .b = 0xa7e2d95f0003}, //28
    {.a = 0x9ea3387a63c1, .b = 0x437e59f57561}, //29
    {.a = 0x0eb23cc8110b, .b = 0x04dc35277635}, //30
    {.a = 0xbc4580b7f20b, .b = 0xd0a4131fb290}, //31
    {.a = 0x7a396f0d633d, .b = 0xad2bdc097023}, //32
    {.a = 0xa3faa6daff67, .b = 0x7600e889adf9}, //33
    {.a = 0xfd8705e721b0, .b = 0x296fc317a513}, //34
    {.a = 0x22052b480d11, .b = 0xe19504c39461}, //35
    {.a = 0xa7141147d430, .b = 0xff16014fefc7}, //36
    {.a = 0x8a8d88151a00, .b = 0x038b5f9b5a2a}, //37
    {.a = 0xb27addfb64b0, .b = 0x152fd0c420a7}, //38
    {.a = 0x7259fa0197c6, .b = 0x5583698df085}, //39
};

static bool plantain_get_card_config(PlantainCardConfig* config, MfClassicType type) {
    bool success = true;

    if(type == MfClassicType1k) {
        config->data_sector = 8;
        config->keys = plantain_1k_keys;
    } else if(type == MfClassicType4k) {
        config->data_sector = 8;
        config->keys = plantain_4k_keys;
    } else {
        success = false;
    }

    return success;
}

static bool plantain_verify_type(Nfc* nfc, MfClassicType type) {
    bool verified = false;

    do {
        PlantainCardConfig cfg = {};
        if(!plantain_get_card_config(&cfg, type)) break;

        const uint8_t block_num = mf_classic_get_first_block_num_of_sector(cfg.data_sector);
        FURI_LOG_D(TAG, "Verifying sector %lu", cfg.data_sector);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(cfg.keys[cfg.data_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Failed to read block %u: %d", block_num, error);
            break;
        }

        verified = true;
    } while(false);

    return verified;
}

static bool plantain_verify(Nfc* nfc) {
    return plantain_verify_type(nfc, MfClassicType1k) ||
           plantain_verify_type(nfc, MfClassicType4k);
}

static bool plantain_read(Nfc* nfc, NfcDevice* device) {
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
        PlantainCardConfig cfg = {};
        if(!plantain_get_card_config(&cfg, data->type)) break;

        const uint8_t legacy_check_sec_num = 26;
        const uint8_t legacy_check_block_num =
            mf_classic_get_first_block_num_of_sector(legacy_check_sec_num);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(
            plantain_4k_keys_legacy[legacy_check_sec_num].a, COUNT_OF(key.data), key.data);

        error = mf_classic_poller_sync_auth(
            nfc, legacy_check_block_num, &key, MfClassicKeyTypeA, NULL);
        if(error == MfClassicErrorNone) {
            FURI_LOG_D(TAG, "Legacy keys detected");
            cfg.keys = plantain_4k_keys_legacy;
        }

        MfClassicDeviceKeys keys = {};
        if(data->type == MfClassicType1k) {
            for(size_t i = 0; i < 32; i++) {
                bit_lib_num_to_bytes_be(cfg.keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
                FURI_BIT_SET(keys.key_a_mask, i);
                bit_lib_num_to_bytes_be(cfg.keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
                FURI_BIT_SET(keys.key_b_mask, i);
            }
        } else {
            for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
                bit_lib_num_to_bytes_be(cfg.keys[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
                FURI_BIT_SET(keys.key_a_mask, i);
                bit_lib_num_to_bytes_be(cfg.keys[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
                FURI_BIT_SET(keys.key_b_mask, i);
            }
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

static bool plantain_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    size_t uid_len = 0;
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    const uint8_t* uid = mf_classic_get_uid(data, &uid_len);

    bool parsed = false;

    do {
        // Verify card type
        PlantainCardConfig cfg = {};
        if(!plantain_get_card_config(&cfg, data->type)) break;

        // Verify key
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, cfg.data_sector);

        const uint64_t key =
            bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        if(key != cfg.keys[cfg.data_sector].a) break;

        furi_string_printf(parsed_data, "\e#Plantain card\n");

        const uint8_t* temp_ptr = &uid[0];

        // UID is read from last to first byte
        uint8_t card_number_tmp[uid_len];

        if(uid_len == 4) {
            for(size_t i = 0; i < 4; i++) {
                card_number_tmp[i] = temp_ptr[3 - i];
            }
        } else if(uid_len == 7) {
            for(size_t i = 0; i < 7; i++) {
                card_number_tmp[i] = temp_ptr[6 - i];
            }
        } else {
            break;
        }
        //UID is converted to a card number
        uint64_t card_number = 0;
        for(size_t i = 0; i < uid_len; i++) {
            card_number = (card_number << 8) | card_number_tmp[i];
        }

        // Print card number with 4-digit groups. "3" in "3078" denotes a ticket type "3 - full ticket", will differ on discounted cards.
        furi_string_cat_printf(parsed_data, "Number: ");
        FuriString* card_number_s = furi_string_alloc();
        furi_string_cat_printf(card_number_s, "%lld", card_number);
        FuriString* tmp_s = furi_string_alloc_set_str("9643 3078 ");
        for(uint8_t i = 0; i < 24; i += 4) {
            for(uint8_t j = 0; j < 4; j++) {
                furi_string_push_back(tmp_s, furi_string_get_char(card_number_s, i + j));
            }
            furi_string_push_back(tmp_s, ' ');
        }
        furi_string_cat_printf(parsed_data, "%s\n", furi_string_get_cstr(tmp_s));
        // this works for 2K Plantain
        if(data->type == MfClassicType1k) {
            //balance
            uint32_t balance = 0;
            for(uint8_t i = 0; i < 4; i++) {
                balance = (balance << 8) | data->block[16].data[3 - i];
            }
            furi_string_cat_printf(parsed_data, "Balance: %ld rub\n", balance / 100);

            //trips
            uint8_t trips_metro = data->block[21].data[0];
            uint8_t trips_ground = data->block[21].data[1];
            furi_string_cat_printf(parsed_data, "Trips: %d\n", trips_metro + trips_ground);
            //trip time
            uint32_t last_trip_timestamp = 0;
            for(uint8_t i = 0; i < 3; i++) {
                last_trip_timestamp = (last_trip_timestamp << 8) | data->block[21].data[4 - i];
            }
            DateTime last_trip = {0};
            from_minutes_to_datetime(last_trip_timestamp + 24 * 60, &last_trip, 2010);
            furi_string_cat_printf(
                parsed_data,
                "Trip start: %02d.%02d.%04d %02d:%02d\n",
                last_trip.day,
                last_trip.month,
                last_trip.year,
                last_trip.hour,
                last_trip.minute);
            //validator
            uint16_t validator = (data->block[20].data[5] << 8) | data->block[20].data[4];
            furi_string_cat_printf(parsed_data, "Validator: %d\n", validator);
            //tariff
            uint16_t fare = (data->block[20].data[7] << 8) | data->block[20].data[6];
            furi_string_cat_printf(parsed_data, "Tariff: %d rub\n", fare / 100);
            //trips in metro
            furi_string_cat_printf(parsed_data, "Trips (Metro): %d\n", trips_metro);
            //trips on ground
            furi_string_cat_printf(parsed_data, "Trips (Ground): %d\n", trips_ground);
            //last payment
            uint32_t last_payment_timestamp = 0;
            for(uint8_t i = 0; i < 3; i++) {
                last_payment_timestamp = (last_payment_timestamp << 8) |
                                         data->block[18].data[4 - i];
            }
            DateTime last_payment_date = {0};
            from_minutes_to_datetime(last_payment_timestamp + 24 * 60, &last_payment_date, 2010);
            furi_string_cat_printf(
                parsed_data,
                "Last pay: %02d.%02d.%04d %02d:%02d\n",
                last_payment_date.day,
                last_payment_date.month,
                last_payment_date.year,
                last_payment_date.hour,
                last_payment_date.minute);
            //Last payment amount.
            uint16_t last_payment = ((data->block[18].data[10] << 16) |
                                     (data->block[18].data[9] << 8) | (data->block[18].data[8])) /
                                    100;
            furi_string_cat_printf(parsed_data, "Amount: %d rub\n", last_payment);
            furi_string_free(card_number_s);
            furi_string_free(tmp_s);
            //This is for 4K Plantains.
        } else if(data->type == MfClassicType4k) {
            //balance
            uint32_t balance = 0;
            for(uint8_t i = 0; i < 4; i++) {
                balance = (balance << 8) | data->block[16].data[3 - i];
            }
            furi_string_cat_printf(parsed_data, "Balance: %ld rub\n", balance / 100);

            //trips
            uint8_t trips_metro = data->block[21].data[0];
            uint8_t trips_ground = data->block[21].data[1];
            furi_string_cat_printf(parsed_data, "Trips: %d\n", trips_metro + trips_ground);
            //trip time
            uint32_t last_trip_timestamp = 0;
            for(uint8_t i = 0; i < 3; i++) {
                last_trip_timestamp = (last_trip_timestamp << 8) | data->block[21].data[4 - i];
            }
            DateTime last_trip = {0};
            from_minutes_to_datetime(last_trip_timestamp + 24 * 60, &last_trip, 2010);
            furi_string_cat_printf(
                parsed_data,
                "Trip start: %02d.%02d.%04d %02d:%02d\n",
                last_trip.day,
                last_trip.month,
                last_trip.year,
                last_trip.hour,
                last_trip.minute);
            //validator
            uint16_t validator = (data->block[20].data[5] << 8) | data->block[20].data[4];
            furi_string_cat_printf(parsed_data, "Validator: %d\n", validator);
            //tariff
            uint16_t fare = (data->block[20].data[7] << 8) | data->block[20].data[6];
            furi_string_cat_printf(parsed_data, "Tariff: %d rub\n", fare / 100);
            //trips in metro
            furi_string_cat_printf(parsed_data, "Trips (Metro): %d\n", trips_metro);
            //trips on ground
            furi_string_cat_printf(parsed_data, "Trips (Ground): %d\n", trips_ground);
            //last payment
            uint32_t last_payment_timestamp = 0;
            for(uint8_t i = 0; i < 3; i++) {
                last_payment_timestamp = (last_payment_timestamp << 8) |
                                         data->block[18].data[4 - i];
            }
            DateTime last_payment_date = {0};
            from_minutes_to_datetime(last_payment_timestamp + 24 * 60, &last_payment_date, 2010);
            furi_string_cat_printf(
                parsed_data,
                "Last pay: %02d.%02d.%04d %02d:%02d\n",
                last_payment_date.day,
                last_payment_date.month,
                last_payment_date.year,
                last_payment_date.hour,
                last_payment_date.minute);
            //Last payment amount
            uint16_t last_payment = ((data->block[18].data[10] << 16) |
                                     (data->block[18].data[9] << 8) | (data->block[18].data[8])) /
                                    100;
            furi_string_cat_printf(parsed_data, "Amount: %d rub\n", last_payment);
            furi_string_free(card_number_s);
            furi_string_free(tmp_s);
        }

        //////////////////////////////// train ticket data handling below ///////////////////////////////////////////

        uint8_t first_ticket_marker = data->block[101].data[0];
        uint8_t second_ticket_marker = 0;
        uint8_t current_status = data->block[105].data[8];
        uint8_t transaction_counter = (data->block[105].data[10]);
        uint32_t tap_data = 0;
        for(uint8_t i = 0; i < 3; i++) {
            tap_data = (tap_data << 8) | data->block[105].data[2 - i];
        }
        uint16_t departure_uic = (data->block[101].data[6] << 8) | (data->block[101].data[5]);
        uint16_t destination_uic = (data->block[101].data[9] << 8) | (data->block[101].data[8]);
        uint16_t direction = (data->block[101].data[14]);
        uint16_t valid_from_date = (data->block[101].data[2] << 8) | (data->block[101].data[1]);
        uint16_t valid_till_date = (data->block[101].data[4] << 8) | (data->block[101].data[3]);
        DateTime v_from = {0};
        DateTime v_till = {0};
        DateTime tap_time = {0};
        uint16_t value_data = (data->block[104].data[0]);
        temp_ptr = data->block[110].data;
        uint8_t sys_n_arr[6] = {0};
        for(size_t i = 0; i < 6; i++) {
            sys_n_arr[i] = temp_ptr[7 - i];
        }
        Storage* storage = furi_record_open(RECORD_STORAGE);
        FuriString* departure_name = furi_string_alloc();
        FuriString* destination_name = furi_string_alloc();
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if(first_ticket_marker != 0 || second_ticket_marker != 0) {
            if(first_ticket_marker != 0) {
                parse_ticket_data(
                    parsed_data,
                    storage,
                    departure_uic,
                    destination_uic,
                    departure_name,
                    destination_name,
                    value_data,
                    current_status,
                    valid_from_date,
                    valid_till_date,
                    tap_data,
                    v_from,
                    v_till,
                    tap_time,
                    second_ticket_marker,
                    sys_n_arr,
                    direction);
            }

            second_ticket_marker = data->block[102].data[0];

            if(second_ticket_marker != 0) {
                //somehow this is not called

                //Variables for the second tickets's data

                departure_uic = (data->block[102].data[6] << 8) | (data->block[102].data[5]);
                destination_uic = (data->block[102].data[9] << 8) | (data->block[102].data[8]);
                direction = (data->block[102].data[14]);
                valid_from_date = (data->block[102].data[2] << 8) | (data->block[102].data[1]);
                valid_till_date = (data->block[102].data[4] << 8) | (data->block[102].data[3]);
                transaction_counter = (data->block[105].data[10]);
                current_status = data->block[105].data[8];

                parse_ticket_data(
                    parsed_data,
                    storage,
                    departure_uic,
                    destination_uic,
                    departure_name,
                    destination_name,
                    value_data,
                    current_status,
                    valid_from_date,
                    valid_till_date,
                    tap_data,
                    v_from,
                    v_till,
                    tap_time,
                    second_ticket_marker,
                    sys_n_arr,
                    direction);
            }
            furi_string_cat_printf(
                parsed_data, "PPK Validation CNT:> %03d\n", transaction_counter);
        }
        furi_record_close(RECORD_STORAGE);
        parsed = true;
        furi_string_free(destination_name);
        furi_string_free(departure_name);
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin plantain_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = plantain_verify,
    .read = plantain_read,
    .parse = plantain_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor plantain_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &plantain_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* plantain_plugin_ep(void) {
    return &plantain_plugin_descriptor;
}
