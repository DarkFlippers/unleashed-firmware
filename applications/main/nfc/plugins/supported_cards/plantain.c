//Based on parsers written by Leptoptilos and Assasinfil. Also, thanks to WillyJL (<me@willyjl.dev>) for help!

#include "nfc_supported_card_plugin.h"
#include <flipper_application/flipper_application.h>
#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <datetime.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_format/flipper_format.h>
#define PLANTAIN_EPOCH_START      1262304000 //2010-01-01
#define PPK_WHOLE_EPOCH_START     946684800 //2000-01-01
#define PPK_CURRENT_EPOCH_START   1388534400 //2014-01-01
#define SECONDS_IN_A_DAY          86400
#define SECONDS_IN_A_MINUTE       60
#define FIRST_PPK_TICKET_OFFSET   101
#define FIRST_TICKET_VALUE_BLOCK  104
#define SECOND_PPK_TICKET_OFFSET  102
#define SECOND_TICKET_VALUE_BLOCK 108

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
    {.a = 0x7413b599c4ea, .b = 0xb0a2aaf3a1ba}, //27
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

typedef struct {
    uint16_t departure_uic;
    uint16_t destination_uic;
    uint16_t trip_start_uic;
    uint16_t trip_end_uic;
    FuriString* departure_name;
    FuriString* destination_name;
    FuriString* trip_start_sta_name;
    FuriString* trip_end_sta_name;
    uint8_t value_data;
    uint8_t direction;
    uint8_t current_status;
    uint16_t valid_from_data;
    uint16_t valid_till_data;
    DateTime valid_from;
    DateTime valid_till;
    uint32_t tap_data;
    DateTime tap_time;
    uint8_t first_ticket_marker;
    uint8_t second_ticket_marker;
    uint64_t sys_n;
    uint8_t ppk_cnt;
} PPKData;

typedef struct {
    uint64_t card_number;
    FuriString* card_number_str;
    uint32_t balance;
    uint8_t trips_metro;
    uint8_t trips_ground;
    uint32_t last_trip_data;
    DateTime last_trip_time;
    uint16_t validator;
    uint16_t fare;
    uint32_t last_payment_date_data;
    DateTime last_payment_date;
    uint16_t last_payment_amount;
    uint8_t keyset;

} PlantainData;
// Function to map UIC codes to station names
static inline void sz_uic_to_sta(Storage* storage, const char* file_name, PPKData* ticket) {
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* departure_uic = furi_string_alloc_printf("%04X", ticket->departure_uic);
    FuriString* destination_uic = furi_string_alloc_printf("%04X", ticket->destination_uic);
    FuriString* trip_start_uic = furi_string_alloc_printf("%04X", ticket->trip_start_uic);
    FuriString* trip_end_uic = furi_string_alloc_printf("%04X", ticket->trip_end_uic);

    if(flipper_format_file_open_existing(file, file_name)) {
        flipper_format_read_string(
            file, furi_string_get_cstr(departure_uic), ticket->departure_name);
        flipper_format_rewind(file);
        flipper_format_read_string(
            file, furi_string_get_cstr(destination_uic), ticket->destination_name);
        flipper_format_rewind(file);
        flipper_format_read_string(
            file, furi_string_get_cstr(trip_start_uic), ticket->trip_start_sta_name);
        flipper_format_rewind(file);
        flipper_format_read_string(
            file, furi_string_get_cstr(trip_end_uic), ticket->trip_end_sta_name);
    }

    flipper_format_free(file);
    furi_string_free(departure_uic);
    furi_string_free(destination_uic);
    furi_string_free(trip_start_uic);
    furi_string_free(trip_end_uic);
}
// Function to resolve station names for a ticket, and if not found, set to "1E" + UIC code
static void resolve_station_name(Storage* storage, PPKData* ticket) {
    sz_uic_to_sta(storage, EXT_PATH("nfc/assets/sz_id.nfc"), ticket);
    if(furi_string_utf8_length(ticket->departure_name) <= 2) {
        furi_string_printf(ticket->departure_name, "1E%04X", ticket->departure_uic);
    }
    if(furi_string_utf8_length(ticket->destination_name) <= 2) {
        furi_string_printf(ticket->destination_name, "1E%04X", ticket->destination_uic);
    }
    if(furi_string_utf8_length(ticket->trip_start_sta_name) <= 2) {
        furi_string_printf(ticket->trip_start_sta_name, "1E%04X", ticket->trip_start_uic);
    }
    if(furi_string_utf8_length(ticket->trip_end_sta_name) <= 2) {
        furi_string_printf(ticket->trip_end_sta_name, "1E%04X", ticket->trip_end_uic);
    }
}
// Function to extract plantain purse data from the card data
static inline void extract_purse_data(
    const MfClassicData* data,
    PlantainData* purse,
    PPKData* ticket,
    uint8_t first_ppk_ticket_offset,
    uint8_t second_ppk_ticket_offset) {
    uint64_t card_number = 0;
    size_t uid_len = 0;
    const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
    purse->card_number_str = furi_string_alloc();
    const uint8_t* temp_ptr = &uid[0];
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
        return;
    }

    for(size_t i = 0; i < uid_len; i++) {
        card_number = (card_number << 8) | card_number_tmp[i];
    }
    FuriString* card_number_s = furi_string_alloc();
    furi_string_cat_printf(card_number_s, "%lld", card_number);
    FuriString* tmp_s = furi_string_alloc_set_str("9643 3078 ");
    for(uint8_t i = 0; i < 24; i += 4) {
        for(uint8_t j = 0; j < 4; j++) {
            furi_string_push_back(tmp_s, furi_string_get_char(card_number_s, i + j));
        }
        furi_string_push_back(tmp_s, ' ');
    }

    furi_string_set(purse->card_number_str, furi_string_get_cstr(tmp_s));
    furi_string_free(card_number_s);
    furi_string_free(tmp_s);

    if(data->type == MfClassicType1k) {
        uint32_t balance = 0;
        for(uint8_t i = 0; i < 4; i++)
            balance = (balance << 8) | data->block[16].data[3 - i];
        balance /= 100;
        purse->balance = balance;
        purse->trips_metro = data->block[21].data[0];
        purse->trips_ground = data->block[21].data[1];
        uint32_t last_trip_timestamp = 0;
        for(uint8_t i = 0; i < 3; i++) {
            last_trip_timestamp = (last_trip_timestamp << 8) | data->block[21].data[4 - i];
        }

        for(uint8_t i = 0; i < 3; i++) {
            purse->last_trip_data = (purse->last_trip_data << 8) | data->block[21].data[4 - i];
        }
        purse->validator = (data->block[20].data[5] << 8) | data->block[20].data[4];
        uint16_t fare = ((data->block[20].data[7] << 8) | data->block[20].data[6]) / 100;
        purse->fare = fare;

        for(uint8_t i = 0; i < 3; i++) {
            purse->last_payment_date_data = (purse->last_payment_date_data << 8) |
                                            data->block[18].data[4 - i];
        }
        purse->last_payment_amount = ((data->block[18].data[10] << 16) |
                                      (data->block[18].data[9] << 8) | (data->block[18].data[8])) /
                                     100;
        last_trip_timestamp = PLANTAIN_EPOCH_START + purse->last_trip_data * SECONDS_IN_A_MINUTE;
        const uint32_t last_payment_timestamp =
            PLANTAIN_EPOCH_START + purse->last_payment_date_data * SECONDS_IN_A_MINUTE;
        datetime_timestamp_to_datetime(last_trip_timestamp, &purse->last_trip_time);
        datetime_timestamp_to_datetime(last_payment_timestamp, &purse->last_payment_date);

    } else if(data->type == MfClassicType4k) {
        uint32_t balance = 0;
        for(uint8_t i = 0; i < 4; i++)
            balance = (balance << 8) | data->block[16].data[3 - i];
        balance /= 100;
        purse->balance = balance;
        purse->trips_metro = data->block[21].data[0];
        purse->trips_ground = data->block[21].data[1];

        for(uint8_t i = 0; i < 3; i++) {
            purse->last_trip_data = (purse->last_trip_data << 8) | data->block[21].data[4 - i];
        }
        purse->validator = (data->block[20].data[5] << 8) | data->block[20].data[4];
        uint16_t fare = ((data->block[20].data[7] << 8) | data->block[20].data[6]) / 100;
        purse->fare = fare;

        for(uint8_t i = 0; i < 3; i++) {
            purse->last_payment_date_data = (purse->last_payment_date_data << 8) |
                                            data->block[18].data[4 - i];
        }
        purse->last_payment_amount = ((data->block[18].data[10] << 16) |
                                      (data->block[18].data[9] << 8) | (data->block[18].data[8])) /
                                     100;
        uint32_t last_trip_timestamp =
            PLANTAIN_EPOCH_START + purse->last_trip_data * SECONDS_IN_A_MINUTE;
        const uint32_t last_payment_timestamp =
            PLANTAIN_EPOCH_START + purse->last_payment_date_data * SECONDS_IN_A_MINUTE;
        datetime_timestamp_to_datetime(last_trip_timestamp, &purse->last_trip_time);
        datetime_timestamp_to_datetime(last_payment_timestamp, &purse->last_payment_date);
    }

    ticket->first_ticket_marker = data->block[first_ppk_ticket_offset].data[0];
    ticket->second_ticket_marker = data->block[second_ppk_ticket_offset].data[0];
}
// Function to extract PPK ticket data from the card data
static inline void extract_ppk_data(
    Storage* storage,
    const MfClassicData* data,
    PPKData* ticket,
    bool second_ticket) {
    const uint8_t* temp_ptr = &data->block[SECOND_TICKET_VALUE_BLOCK + 2].data[0];
    uint8_t sys_n_arr[6] = {0};

    if(second_ticket == 0) {
        for(size_t i = 0; i < 6; i++) {
            sys_n_arr[i] = temp_ptr[7 - i];
        }
        ticket->departure_uic = (data->block[FIRST_PPK_TICKET_OFFSET].data[6] << 8) |
                                (data->block[FIRST_PPK_TICKET_OFFSET].data[5]);
        ticket->destination_uic = (data->block[FIRST_PPK_TICKET_OFFSET].data[9] << 8) |
                                  (data->block[FIRST_PPK_TICKET_OFFSET].data[8]);
        ticket->value_data = data->block[FIRST_TICKET_VALUE_BLOCK].data[0];
        ticket->current_status = data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[8];
        ticket->valid_from_data = (data->block[FIRST_PPK_TICKET_OFFSET].data[2] << 8) |
                                  (data->block[FIRST_PPK_TICKET_OFFSET].data[1]);
        ticket->valid_till_data = (data->block[FIRST_PPK_TICKET_OFFSET].data[4] << 8) |
                                  (data->block[FIRST_PPK_TICKET_OFFSET].data[3]);

        ticket->direction = data->block[FIRST_PPK_TICKET_OFFSET].data[14];
        ticket->tap_data = (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[2] << 16) |
                           (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[1] << 8) |
                           data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[0];
        ticket->ppk_cnt = data->block[105].data[10];
        ticket->trip_start_uic = (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[4] << 8) |
                                 (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[3]);
        ticket->trip_end_uic = (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[7] << 8) |
                               (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[6]);

    } else if(second_ticket == 1) {
        for(size_t i = 0; i < 6; i++) {
            sys_n_arr[i] = temp_ptr[13 - i];
        }
        ticket->departure_uic = (data->block[SECOND_PPK_TICKET_OFFSET].data[6] << 8) |
                                (data->block[SECOND_PPK_TICKET_OFFSET].data[5]);
        ticket->destination_uic = (data->block[SECOND_PPK_TICKET_OFFSET].data[9] << 8) |
                                  (data->block[SECOND_PPK_TICKET_OFFSET].data[8]);
        ticket->value_data = data->block[SECOND_TICKET_VALUE_BLOCK].data[0];
        ticket->current_status = data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[8];
        ticket->valid_from_data = (data->block[SECOND_PPK_TICKET_OFFSET].data[2] << 8) |
                                  (data->block[SECOND_PPK_TICKET_OFFSET].data[1]);
        ticket->valid_till_data = (data->block[SECOND_PPK_TICKET_OFFSET].data[4] << 8) |
                                  (data->block[SECOND_PPK_TICKET_OFFSET].data[3]);

        ticket->direction = data->block[SECOND_PPK_TICKET_OFFSET].data[14];
        ticket->tap_data = (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[2] << 16) |
                           (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[1] << 8) |
                           data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[0];
        ticket->ppk_cnt = data->block[109].data[10];
        ticket->trip_start_uic = (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[4] << 8) |
                                 (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[3]);
        ticket->trip_end_uic = (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[7] << 8) |
                               (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[6]);
    }
    ticket->first_ticket_marker = data->block[FIRST_PPK_TICKET_OFFSET].data[0];
    ticket->second_ticket_marker = data->block[SECOND_PPK_TICKET_OFFSET].data[0];
    uint64_t sys_n = 0;
    for(size_t i = 0; i < 6; i++) {
        sys_n = (sys_n << 8) | sys_n_arr[i];
    }
    ticket->sys_n = sys_n;

    const uint32_t valid_from_timestamp =
        PPK_WHOLE_EPOCH_START + ticket->valid_from_data * SECONDS_IN_A_DAY;
    const uint32_t valid_till_timestamp =
        PPK_WHOLE_EPOCH_START + ticket->valid_till_data * SECONDS_IN_A_DAY;
    const uint32_t tap_timestamp =
        PPK_CURRENT_EPOCH_START + ticket->tap_data * SECONDS_IN_A_MINUTE;
    datetime_timestamp_to_datetime(valid_from_timestamp, &ticket->valid_from);
    datetime_timestamp_to_datetime(valid_till_timestamp, &ticket->valid_till);
    datetime_timestamp_to_datetime(tap_timestamp, &ticket->tap_time);
    resolve_station_name(storage, ticket);
}
// Function to format and print plantain purse data
static void printf_plantain_data(FuriString* parsed_data, PlantainData* purse) {
    furi_string_printf(parsed_data, "\e#Plantain Card");
    furi_string_cat_printf(
        parsed_data,
        "\nNumber: %s\nBalance: %ld RUB\nMetro Trips: %d\nGround Trips: %d\nLast Trip: %02d.%02d.%04d %02d:%02d",
        furi_string_get_cstr(purse->card_number_str),
        purse->balance,
        purse->trips_metro,
        purse->trips_ground,
        purse->last_trip_time.day,
        purse->last_trip_time.month,
        purse->last_trip_time.year,
        purse->last_trip_time.hour,
        purse->last_trip_time.minute);

    furi_string_cat_printf(
        parsed_data,
        "\nValidator: %d\nFare: %d RUB\nRefilled on: %02d.%02d.%04d %02d:%02d\nAmount: %d RUB",
        purse->validator,
        purse->fare,
        purse->last_payment_date.day,
        purse->last_payment_date.month,
        purse->last_payment_date.year,
        purse->last_payment_date.hour,
        purse->last_payment_date.minute,
        purse->last_payment_amount);

    if(purse->keyset == 1)
        furi_string_cat_printf(parsed_data, "\nPPK keys installed:> YES");
    else
        furi_string_cat_printf(parsed_data, "\nPPK keys installed:> NO");

    furi_string_free(purse->card_number_str);
}

// Function to format and print PPK ticket data
static void printf_ppk_data(FuriString* parsed_data, PPKData* ticket, bool ticket_number) {
    if(ticket_number == 0) {
        furi_string_cat_printf(parsed_data, "\n\n\e#PPK Ticket:\n");
        switch(ticket->first_ticket_marker) {
        case 0x33:
            furi_string_cat_printf(parsed_data, "Type:> 1 ride");
            break;
        case 0x34:
            furi_string_cat_printf(parsed_data, "Type:> 2 rides (Mon.-Fri.)");
            break;
        case 0x35:
            furi_string_cat_printf(parsed_data, "Type:> 2 rides (Fri.-Mon.)");
            break;
        case 0x36:
            furi_string_cat_printf(parsed_data, "Type:> 2 rides (Sat.-Mon.");
            break;
        default:
            furi_string_cat_printf(
                parsed_data, "Type:> Unknown, 0x%02X", ticket->first_ticket_marker);
        }
    } else if(ticket_number == 1) {
        furi_string_cat_printf(parsed_data, "\n\nSecond PPK Ticket:\n");
        switch(ticket->second_ticket_marker) {
        case 0x33:
            furi_string_cat_printf(parsed_data, "Type:> 1 ride");
            break;
        case 0x34:
            furi_string_cat_printf(parsed_data, "Type:> 2 rides (Mon.-Fri.)");
            break;
        case 0x35:
            furi_string_cat_printf(parsed_data, "Type:> 2 rides (Fri.-Mon.)");
            break;
        case 0x36:
            furi_string_cat_printf(parsed_data, "Type:> 2 rides (Sat.-Mon.)");
            break;
        default:
            furi_string_cat_printf(
                parsed_data, "Type:> Unknown, 0x%02X", ticket->second_ticket_marker);
        }
    }

    furi_string_cat_printf(
        parsed_data,
        "\nFrom:> %s\nTo:> %s",
        furi_string_get_cstr(ticket->departure_name),
        furi_string_get_cstr(ticket->destination_name));

    if(ticket->valid_from.day == ticket->valid_till.day) {
        furi_string_cat_printf(
            parsed_data,
            "\nValid On: %02d-%02d-%04d",
            ticket->valid_from.day,
            ticket->valid_from.month,
            ticket->valid_from.year);
    } else {
        furi_string_cat_printf(
            parsed_data,
            "\nValid From: %02d-%02d-%04d\nValid thru:  %02d-%02d-%04d",
            ticket->valid_from.day,
            ticket->valid_from.month,
            ticket->valid_from.year,
            ticket->valid_till.day,
            ticket->valid_till.month,
            ticket->valid_till.year);
    }

    if(ticket->direction == 1) {
        furi_string_cat_printf(parsed_data, "\nDirection: One-way ->>");
    } else if(ticket->direction == 2) {
        furi_string_cat_printf(parsed_data, "\nDirection: Round-trip <<-->>");
    }
    furi_string_cat_printf(parsed_data, "\nRides left:> %02d", ticket->value_data);

    if(ticket->current_status == 0) {
        furi_string_cat_printf(parsed_data, "\nStatus:> TICKET IS READY\n");
    } else if(ticket->current_status == 0x80)
        furi_string_cat_printf(
            parsed_data,
            "\nStatus:> ENTERED STATION\nSta name:> %s\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\n",
            furi_string_get_cstr(ticket->trip_start_sta_name),
            ticket->tap_time.day,
            ticket->tap_time.month,
            ticket->tap_time.year,
            ticket->tap_time.hour,
            ticket->tap_time.minute);
    else if(ticket->current_status == 0x1E)
        furi_string_cat_printf(
            parsed_data,
            "\nStatus:> EXITED STATION\nSta name:> %s\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\n",
            furi_string_get_cstr(ticket->trip_end_sta_name),
            ticket->tap_time.day,
            ticket->tap_time.month,
            ticket->tap_time.year,
            ticket->tap_time.hour,
            ticket->tap_time.minute);
    else
        furi_string_cat_printf(parsed_data, "\nStatus:> UNKNOWN (%02X)\n", ticket->current_status);

    furi_string_cat_printf(
        parsed_data, "SYS N:> %lld\nPPK CNT:> %03d", ticket->sys_n, ticket->ppk_cnt);
}
//Function to select a keyset based on card type
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
        PlantainCardConfig cfg = {0};
        if(!plantain_get_card_config(&cfg, type)) break;

        const uint8_t block_num = mf_classic_get_first_block_num_of_sector(cfg.data_sector);

        MfClassicKey key = {0};
        bit_lib_num_to_bytes_be(cfg.keys[cfg.data_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_context;
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_context);
        if(error != MfClassicErrorNone) {
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
        PlantainCardConfig cfg = {0};
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
            break;
        }

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (error == MfClassicErrorNone);
    } while(false);

    mf_classic_free(data);

    return is_read;
}
//Main parsing function
static bool plantain_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    PPKData ticket = {0};
    ticket.departure_name = furi_string_alloc();
    ticket.destination_name = furi_string_alloc();
    ticket.trip_start_sta_name = furi_string_alloc();
    ticket.trip_end_sta_name = furi_string_alloc();
    PlantainData purse = {0};
    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool parsed = false;

    do {
        // Verify card type
        PlantainCardConfig cfg = {0};
        if(!plantain_get_card_config(&cfg, data->type)) break;

        // Verify key
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(data, cfg.data_sector);

        const uint64_t key =
            bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
        if(key != cfg.keys[cfg.data_sector].a) break;

        if(data->block[107].data[10] == 0x02)
            purse.keyset = 0;
        else
            purse.keyset = 1;

        // Extract plantain purse data and fill PPK tickets markers
        extract_purse_data(
            data, &purse, &ticket, FIRST_PPK_TICKET_OFFSET, SECOND_PPK_TICKET_OFFSET);
        // Print plantain purse data
        printf_plantain_data(parsed_data, &purse);
        if(purse.card_number_str) furi_string_free(purse.card_number_str);
        // Extract and print PPK ticket data if present
        if(ticket.first_ticket_marker != 0) {
            extract_ppk_data(storage, data, &ticket, 0);
            printf_ppk_data(parsed_data, &ticket, false);
        }
        // Extract and print second PPK ticket data if present
        if(ticket.second_ticket_marker != 0) {
            PPKData second_ticket = {0};
            second_ticket.departure_name = furi_string_alloc();
            second_ticket.destination_name = furi_string_alloc();
            second_ticket.trip_start_sta_name = furi_string_alloc();
            second_ticket.trip_end_sta_name = furi_string_alloc();

            extract_ppk_data(storage, data, &second_ticket, 1);

            printf_ppk_data(parsed_data, &second_ticket, true);

            furi_string_free(second_ticket.departure_name);
            furi_string_free(second_ticket.destination_name);
            furi_string_free(second_ticket.trip_start_sta_name);
            furi_string_free(second_ticket.trip_end_sta_name);
        }

        parsed = true;
    } while(false);
    furi_string_free(ticket.departure_name);
    furi_string_free(ticket.destination_name);
    furi_string_free(ticket.trip_start_sta_name);
    furi_string_free(ticket.trip_end_sta_name);
    furi_record_close(RECORD_STORAGE);

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
