
#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <datetime.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_format/flipper_format.h>

#define TAG "SZPPK_SO"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

static const char* nfc_resources_header = "Flipper NFC resources";
static const uint32_t nfc_resources_file_version = 1;

static const MfClassicKeyPair so_card_2k[] = {
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B7B5B8B4B5}, //0
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B6B2B9B8B7}, //1
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B2B4B4B4B0}, //2
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B3B5B2B2B2}, //3
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B8B4B8B0B9}, //4
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B3B7B9B8B6}, //5
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B6B5B5B5B6}, //6
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B7B0B0B2B0}, //7
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B1B2B8B0}, //8
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B0B1B8B3B2}, //9
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B0B0B3B1B8}, //10
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B2B6B7B5B2}, //11
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B9B2B0B2B0}, //12
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B7B8B8B7B3}, //13
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B3B4B9B2B0}, //14
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B7B2B0B0B9}, //15
    {.a = 0xA94CAB611187, .b = 0x389109BD1D82}, //16
    {.a = 0xBF4280329F11, .b = 0x28D9EDD2096D}, //17
    {.a = 0xDE6BD90BD6B0, .b = 0x94866C16E9A4}, //18
    {.a = 0x2EA9493CAA7C, .b = 0x5068BCE2BC1C}, //19
    {.a = 0x15A41BA53F6C, .b = 0x3BD3CF43571C}, //20
    {.a = 0x1290FFD80DB5, .b = 0xD821B7916B7E}, //21
    {.a = 0x68C1A07E96A9, .b = 0x2B3323E75750}, //22
    {.a = 0xC699831AB307, .b = 0xCD7F7E9111F1}, //23
    {.a = 0x4E5884BF23E9, .b = 0x2287812A6AEE}, //24
    {.a = 0xC55212F716DC, .b = 0x594E368CCEFF}, //25
    {.a = 0x6EF127E674B1, .b = 0xDD21C8D3E0B9}, //26
    {.a = 0xFB79FAF4B55C, .b = 0xFE52B3B2A93B}, //27
    {.a = 0x6CF85CDFF647, .b = 0xCCAD7C41FC8A}, //28
    {.a = 0x591F6C130F91, .b = 0x2D2B734ECF91}, //29
    {.a = 0xEEB83529B79B, .b = 0xCB14E70EBA38}, //30
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //31

};

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

    sz_UIC_to_sta(storage, EXT_PATH("nfc/assets/skppk_id.nfc"), key, name);
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
    bool card_type,
    DateTime v_from,
    DateTime v_till,
    DateTime tap_time,
    uint8_t second_ticket_marker) {
    bool parsed = false;
    uint32_t valid_from_timestamp = 946684800 + valid_from_date * 24 * 60 * 60;
    uint32_t valid_till_timestamp = 946684800 + valid_till_date * 24 * 60 * 60;
    uint32_t tap_timestamp = 1388530800 + tap_data * 60;
    datetime_timestamp_to_datetime(valid_from_timestamp, &v_from);
    datetime_timestamp_to_datetime(valid_till_timestamp, &v_till);
    datetime_timestamp_to_datetime(tap_timestamp, &tap_time);

    sz_uic_search(storage, departure_uic, departure_name);
    if(furi_string_utf8_length(departure_name) <= 2)
        furi_string_printf(departure_name, "1E%04X", departure_uic);
    sz_uic_search(storage, destination_uic, destination_name);
    if(furi_string_utf8_length(destination_name) <= 2)
        furi_string_printf(destination_name, "1E%04X", destination_uic);

    if(departure_uic ==
       0x0000) { //if the ticket is not issued (unissued tickets will have a 0x0000 as a departure station ID)
        furi_string_cat_printf(
            parsed_data,
            "\e#Unknown SZPPK Card\n   NO TICKET DATA FOUND \n\nTHE TICKET IS NOT ISSUED\nOR LAYOUT IS UNKNOWN\n");
    } else { //if the ticket is issued

        if(card_type == 0) { //if the ticket is an Accomp. Card
            furi_string_cat_printf(
                parsed_data,
                "\e#SZPPK Accompany Card\nValid on: %02d-%02d-%04d\nStation: > %s\n",
                v_from.day,
                v_from.month,
                v_from.year,
                furi_string_get_cstr(departure_name));
            if(current_status == 0x00)
                furi_string_cat_printf(parsed_data, "Status:> NOT USED\n");
            else if(current_status == 0x80) //if status == IN
                furi_string_cat_printf(
                    parsed_data,
                    "Status:> ENTERED STATION\nChecked in at:> %02d:%02d\n",
                    tap_time.hour,
                    tap_time.minute);
            else if(current_status == 0x1E) //if status == OUT
                furi_string_cat_printf(
                    parsed_data,
                    "Status:> EXITED STATION\nChecked out at:> %02d:%02d\n",
                    tap_time.hour,
                    tap_time.minute);
            else
                furi_string_cat_printf(parsed_data, "Status:> UNKNOWN");
        }

        else { //If it's a transport card
            if(second_ticket_marker == 0)
                furi_string_cat_printf(parsed_data, "\e#SZPPK Transport Card\n");
            else
                furi_string_cat_printf(parsed_data, "\e#Second Ticket:\n");
            furi_string_cat_printf(
                parsed_data,
                "Valid from: %02d-%02d-%04d\nValid thru:  %02d-%02d-%04d\nFrom:> %s\nTo: %s\n",
                v_from.day,
                v_from.month,
                v_from.year,
                v_till.day,
                v_till.month,
                v_till.year,
                furi_string_get_cstr(departure_name),
                furi_string_get_cstr(destination_name));

            if(value_data > 0)
                furi_string_cat_printf(parsed_data, "Rides remain: %02d\n", value_data);
            if(current_status == 0x00)
                furi_string_cat_printf(parsed_data, "Status:> NOT USED\n");
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
                furi_string_cat_printf(parsed_data, "Status:> UNKNOWN (%04X)\n", current_status);
        }
    }
    parsed = true;

    return parsed;
}

bool szppk_so_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 19;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(so_card_2k[verify_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);
        if(error != MfClassicErrorNone) break;

        verified = true;
    } while(false);

    return verified;
}

static bool szppk_so_read(Nfc* nfc, NfcDevice* device) {
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
        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < 32; i++) {
            bit_lib_num_to_bytes_be(so_card_2k[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(so_card_2k[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }
        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error == MfClassicErrorNotPresent) break;
        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (error == MfClassicErrorNone);
    } while(false);

    mf_classic_free(data);

    return is_read;
}

static bool szppk_so_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    FuriString* departure_name = furi_string_alloc();
    FuriString* destination_name = furi_string_alloc();

    bool parsed = false;

    do {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 19);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != so_card_2k[19].a) break;

        uint8_t value_data = data->block[77].data[0];
        uint16_t departure_uic = (data->block[76].data[6] << 8) | (data->block[76].data[5]);
        uint16_t destination_uic = (data->block[76].data[9] << 8) | (data->block[76].data[8]);
        uint8_t current_status = (data->block[78].data[8]);
        uint16_t valid_from_date = (data->block[76].data[2] << 8) |
                                   (data->block[76].data[1]); //number of days since Jan 1st 2000
        uint16_t valid_till_date = (data->block[76].data[4] << 8) |
                                   (data->block[76].data[3]); //number of days since Jan 1st 2000
        uint8_t second_ticket_marker = 0;
        uint32_t tap_data = 0;
        for(uint8_t i = 0; i < 3; i++) {
            tap_data = (tap_data << 8) | data->block[78].data[2 - i];
        }
        DateTime v_from = {0};
        DateTime v_till = {0};
        DateTime tap_time = {0};
        bool card_type =
            (departure_uic == destination_uic) ?
                0 : //transport card's departure and destination will differ
                1; //Accomp. cards will have the same station ID for Departure and Destination
        Storage* storage = furi_record_open(RECORD_STORAGE);

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
            card_type,
            v_from,
            v_till,
            tap_time,
            second_ticket_marker);

        second_ticket_marker = data->block[88].data[7];

        if(second_ticket_marker != 0) {
            departure_uic = (data->block[88].data[6] << 8) | (data->block[88].data[5]);
            value_data = data->block[89].data[0];
            destination_uic = (data->block[88].data[9] << 8) | (data->block[88].data[8]);
            current_status = (data->block[90].data[9] << 8) | (data->block[90].data[8]);
            valid_from_date = (data->block[88].data[2] << 8) |
                              (data->block[88].data[1]); //number of days since Jan 1st 2000
            valid_till_date = (data->block[88].data[4] << 8) |
                              (data->block[88].data[3]); //number of days since Jan 1st 2000
            tap_data = 0;
            for(uint8_t i = 0; i < 3; i++) {
                tap_data = (tap_data << 8) | data->block[90].data[2 - i];
            };
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
                card_type,
                v_from,
                v_till,
                tap_time,
                second_ticket_marker);
        }
        furi_record_close(RECORD_STORAGE);
        parsed = true;

    } while(false);
    furi_string_free(destination_name);
    furi_string_free(departure_name);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin szppk_so_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = szppk_so_verify,
    .read = szppk_so_read,
    .parse = szppk_so_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor szppk_so_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &szppk_so_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* szppk_so_plugin_ep(void) {
    return &szppk_so_plugin_descriptor;
}
