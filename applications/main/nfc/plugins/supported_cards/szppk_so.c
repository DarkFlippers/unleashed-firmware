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

typedef struct {
    uint16_t departure_uic;
    uint16_t destination_uic;
    uint8_t value_data;
    uint8_t current_status;
    uint16_t valid_from_date;
    uint16_t valid_till_date;
    uint32_t tap_data;
} TicketData;

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

static inline bool sz_uic_to_sta(
    Storage* storage,
    const char* file_name,
    FuriString* key,
    FuriString* data) {
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool parsed = false;

    if(flipper_format_file_open_existing(file, file_name)) {
        uint32_t version = 0;
        FuriString* temp_str = furi_string_alloc();
        
        if(flipper_format_read_header(file, temp_str, &version) &&
           !furi_string_cmp_str(temp_str, nfc_resources_header) &&
           (version == nfc_resources_file_version)) {
            flipper_format_read_string(file, furi_string_get_cstr(key), data);
            parsed = true;
        }
        
        furi_string_free(temp_str);
    }

    flipper_format_free(file);
    return parsed;
}

static inline bool sz_uic_search(Storage* storage, uint16_t uic, FuriString* name) {
    FuriString* key = furi_string_alloc_printf("%04X", uic);
    sz_uic_to_sta(storage, EXT_PATH("nfc/assets/szppk_id.nfc"), key, name);
    furi_string_free(key);
    return true;
}

static inline void resolve_station_name(
    Storage* storage,
    uint16_t uic,
    FuriString* name) {
    sz_uic_search(storage, uic, name);
    if(furi_string_utf8_length(name) <= 2) {
        furi_string_printf(name, "1E%04X", uic);
    }
}

static inline void convert_timestamps(
    uint16_t valid_from_date,
    uint16_t valid_till_date,
    uint32_t tap_data,
    DateTime* v_from,
    DateTime* v_till,
    DateTime* tap_time) {
    const uint32_t valid_from_timestamp = 946684800 + valid_from_date * 86400;
    const uint32_t valid_till_timestamp = 946684800 + valid_till_date * 86400;
    const uint32_t tap_timestamp = 1388530800 + tap_data * 60;
    
    datetime_timestamp_to_datetime(valid_from_timestamp, v_from);
    datetime_timestamp_to_datetime(valid_till_timestamp, v_till);
    datetime_timestamp_to_datetime(tap_timestamp, tap_time);
}

static inline void extract_ticket_data(
    const MfClassicData* data,
    uint8_t block_offset,
    TicketData* ticket) {
    ticket->departure_uic = (data->block[block_offset].data[6] << 8) | 
                           (data->block[block_offset].data[5]);
    ticket->destination_uic = (data->block[block_offset].data[9] << 8) | 
                             (data->block[block_offset].data[8]);
    ticket->value_data = data->block[block_offset + 1].data[0];
    ticket->current_status = data->block[block_offset + 2].data[8];
    ticket->valid_from_date = (data->block[block_offset].data[2] << 8) | 
                              (data->block[block_offset].data[1]);
    ticket->valid_till_date = (data->block[block_offset].data[4] << 8) | 
                              (data->block[block_offset].data[3]);
    
    ticket->tap_data = ((uint32_t)data->block[block_offset + 2].data[2] << 16) |
                       ((uint32_t)data->block[block_offset + 2].data[1] << 8) |
                       data->block[block_offset + 2].data[0];
}

static inline bool is_accompany_card(uint16_t departure_uic, uint16_t destination_uic) {
    return departure_uic == destination_uic;
}

static void format_accompany_card(
    FuriString* parsed_data,
    const DateTime* v_from,
    const FuriString* departure_name,
    uint8_t current_status,
    const DateTime* tap_time) {
    furi_string_cat_printf(
        parsed_data,
        "\e#SZPPK Accompany Card\nValid on: %02d-%02d-%04d\nStation: > %s\n",
        v_from->day,
        v_from->month,
        v_from->year,
        furi_string_get_cstr(departure_name));

    switch(current_status) {
    case 0x00:
        furi_string_cat_printf(parsed_data, "Status:> NOT USED\n");
        break;
    case 0x80:
        furi_string_cat_printf(
            parsed_data,
            "Status:> ENTERED STATION\nChecked in at:> %02d:%02d\n",
            tap_time->hour,
            tap_time->minute);
        break;
    case 0x1E:
        furi_string_cat_printf(
            parsed_data,
            "Status:> EXITED STATION\nChecked out at:> %02d:%02d\n",
            tap_time->hour,
            tap_time->minute);
        break;
    default:
        furi_string_cat_printf(parsed_data, "Status:> UNKNOWN");
        break;
    }
}

static void format_transport_card(
    FuriString* parsed_data,
    const DateTime* v_from,
    const DateTime* v_till,
    const FuriString* departure_name,
    const FuriString* destination_name,
    const TicketData* ticket,
    const DateTime* tap_time,
    bool is_second_ticket) {
    if(!is_second_ticket) {
        furi_string_cat_printf(parsed_data, "\e#SZPPK Transport Card\n");
    } else {
        furi_string_cat_printf(parsed_data, "\e#Second Ticket:\n");
    }

    furi_string_cat_printf(
        parsed_data,
        "Valid from: %02d-%02d-%04d\nValid thru:  %02d-%02d-%04d\nFrom:> %s\nTo: %s\n",
        v_from->day,
        v_from->month,
        v_from->year,
        v_till->day,
        v_till->month,
        v_till->year,
        furi_string_get_cstr(departure_name),
        furi_string_get_cstr(destination_name));

    if(ticket->value_data > 0) {
        furi_string_cat_printf(parsed_data, "Rides remain: %02d\n", ticket->value_data);
    }

    switch(ticket->current_status) {
    case 0x00:
        furi_string_cat_printf(parsed_data, "Status:> NOT USED\n");
        break;
    case 0x80:
        furi_string_cat_printf(
            parsed_data,
            "Status:> ENTERED STATION\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\n\n",
            tap_time->day,
            tap_time->month,
            tap_time->year,
            tap_time->hour,
            tap_time->minute);
        break;
    case 0x1E:
        furi_string_cat_printf(
            parsed_data,
            "Status:> EXITED STATION\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\n\n",
            tap_time->day,
            tap_time->month,
            tap_time->year,
            tap_time->hour,
            tap_time->minute);
        break;
    default:
        furi_string_cat_printf(parsed_data, "Status:> UNKNOWN (%04X)\n", ticket->current_status);
        break;
    }
}

static void parse_ticket_data(
    FuriString* parsed_data,
    Storage* storage,
    const TicketData* ticket,
    bool is_second_ticket) {
    if(ticket->departure_uic == 0x0000) {
        furi_string_cat_printf(
            parsed_data,
            "\e#Unknown SZPPK Card\n   NO TICKET DATA FOUND \n\nTHE TICKET IS NOT ISSUED\nOR LAYOUT IS UNKNOWN\n");
        return;
    }

    DateTime v_from = {0}, v_till = {0}, tap_time = {0};
    convert_timestamps(
        ticket->valid_from_date,
        ticket->valid_till_date,
        ticket->tap_data,
        &v_from,
        &v_till,
        &tap_time);

    FuriString* departure_name = furi_string_alloc();
    FuriString* destination_name = furi_string_alloc();
    
    resolve_station_name(storage, ticket->departure_uic, departure_name);
    resolve_station_name(storage, ticket->destination_uic, destination_name);

    if(is_accompany_card(ticket->departure_uic, ticket->destination_uic)) {
        format_accompany_card(
            parsed_data,
            &v_from,
            departure_name,
            ticket->current_status,
            &tap_time);
    } else {
        format_transport_card(
            parsed_data,
            &v_from,
            &v_till,
            departure_name,
            destination_name,
            ticket,
            &tap_time,
            is_second_ticket);
    }

    furi_string_free(departure_name);
    furi_string_free(destination_name);
}

bool szppk_so_verify(Nfc* nfc) {
    const uint8_t verify_sector = 19;
    const uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);

    MfClassicKey key = {};
    bit_lib_num_to_bytes_be(so_card_2k[verify_sector].a, COUNT_OF(key.data), key.data);

    MfClassicAuthContext auth_ctx = {};
    MfClassicError error =
        mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);
    
    return error == MfClassicErrorNone;
}

static bool szppk_so_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);
    
    bool is_read = false;
    MfClassicType type = MfClassicType1k;
    MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
    
    if(error == MfClassicErrorNone) {
        data->type = type;
        MfClassicDeviceKeys keys = {};
        
        for(size_t i = 0; i < 32; i++) {
            bit_lib_num_to_bytes_be(so_card_2k[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(so_card_2k[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
            FURI_BIT_SET(keys.key_b_mask, i);
        }
        
        error = mf_classic_poller_sync_read(nfc, &keys, data);
        if(error != MfClassicErrorNotPresent) {
            nfc_device_set_data(device, NfcProtocolMfClassic, data);
            is_read = (error == MfClassicErrorNone);
        }
    }

    mf_classic_free(data);
    return is_read;
}

static bool szppk_so_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;

    do {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 19);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != so_card_2k[19].a) break;

        Storage* storage = furi_record_open(RECORD_STORAGE);
        
        TicketData primary_ticket = {0};
        extract_ticket_data(data, 76, &primary_ticket);
        parse_ticket_data(parsed_data, storage, &primary_ticket, false);

        const uint8_t second_ticket_marker = data->block[88].data[7];
        if(second_ticket_marker != 0) {
            TicketData secondary_ticket = {0};
            extract_ticket_data(data, 88, &secondary_ticket);
            parse_ticket_data(parsed_data, storage, &secondary_ticket, true);
        }
        
        furi_record_close(RECORD_STORAGE);
        parsed = true;

    } while(false);

    return parsed;
}

static const NfcSupportedCardsPlugin szppk_so_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = szppk_so_verify,
    .read = szppk_so_read,
    .parse = szppk_so_parse,
};

static const FlipperAppPluginDescriptor szppk_so_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &szppk_so_plugin,
};

const FlipperAppPluginDescriptor* szppk_so_plugin_ep(void) {
    return &szppk_so_plugin_descriptor;
}