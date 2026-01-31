#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <datetime.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_format/flipper_format.h>

#define TAG "SKPPK_TK"

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

static const MfClassicKeyPair t_card_4k[] = {
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B4B2B1B3B6}, //0
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B5B2B4B9B0}, //1
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B8B4B6B1B3}, //2
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B0B3B8B5B7}, //3
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B0B6B4B2B8}, //4
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B4B5B3B8}, //5
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B0B8B1B7B8}, //6
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B5B1B8B1}, //7
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B0B6B4B8B7}, //8
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B9B2B9B2B4}, //9
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B1B8B0B6}, //10
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B7B8B9B5B5}, //11
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B0B7B1B0B0}, //12
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B9B2B8B3B2}, //13
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B4B1B0B4B6}, //14
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B4B2B5B4B8}, //15
    {.a = 0x684CE8377AB8, .b = 0x91888D728D9E}, //16
    {.a = 0xF462ED255B44, .b = 0x0D40620AC610}, //17
    {.a = 0xF1E8FD5B5C9F, .b = 0xABB5763550B0}, //18
    {.a = 0xE9E3265B45B1, .b = 0x11D848B26034}, //19
    {.a = 0xAEA3755DFA82, .b = 0x36D1BAC1395E}, //20
    {.a = 0x83F58B205854, .b = 0x967DAFCF2674}, //21
    {.a = 0x5CDF18E68A75, .b = 0x1689C175B14E}, //22
    {.a = 0x126DC25C5D53, .b = 0x346B03AF1FF3}, //23
    {.a = 0xF1B013C4495C, .b = 0x74CE14DBC71F}, //24
    {.a = 0xA5FE4FAD0269, .b = 0x0025FEA845E5}, //25
    {.a = 0x43080428049C, .b = 0x2E91BB6F511E}, //26
    {.a = 0x4F44A08C51BC, .b = 0xE44CC58DF833}, //27
    {.a = 0x3FEC92F652BE, .b = 0x942039006B83}, //28
    {.a = 0x3A3BE5B635FA, .b = 0xFC564425A9BA}, //29
    {.a = 0x78C9E1C688BB, .b = 0xA29E362B22F3}, //30
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //31
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //32
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //33
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //34
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //35
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //36
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //37
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //38
    {.a = 0xFFFFFFFFFFFF, .b = 0xB0B1B2B3B4B5}, //39
};

static const char* nfc_resources_header = "Flipper NFC resources";
static const uint32_t nfc_resources_file_version = 1;

static inline bool
    sk_uic_to_sta(Storage* storage, const char* file_name, FuriString* key, FuriString* data) {
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

static inline bool sk_uic_search(Storage* storage, uint16_t uic, FuriString* name) {
    FuriString* key = furi_string_alloc_printf("%04X", uic);
    sk_uic_to_sta(storage, EXT_PATH("nfc/assets/skppk_id.nfc"), key, name);
    furi_string_free(key);
    return true;
}

static inline void resolve_station_name(Storage* storage, uint16_t uic, FuriString* name) {
    sk_uic_search(storage, uic, name);
    if(furi_string_utf8_length(name) <= 2) {
        furi_string_printf(name, "1F%04X", uic);
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

static inline void
    extract_ticket_data(const MfClassicData* data, uint8_t block_offset, TicketData* ticket) {
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
        furi_string_cat_printf(parsed_data, "\e#SKPPK Transport Card\n");
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
    case 0x1F:
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
        furi_string_cat_printf(parsed_data, "Status:> UNKNOWN (%02X)\n", ticket->current_status);
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
            "\e#Unknown SKPPK Card\n   NO TICKET DATA FOUND \n\nTHE TICKET IS NOT ISSUED\nOR LAYOUT IS UNKNOWN\n");
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

    format_transport_card(
        parsed_data,
        &v_from,
        &v_till,
        departure_name,
        destination_name,
        ticket,
        &tap_time,
        is_second_ticket);

    furi_string_free(departure_name);
    furi_string_free(destination_name);
}

bool sk_tk_verify(Nfc* nfc) {
    const uint8_t verify_sector = 19;
    const uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);

    MfClassicKey key = {};
    bit_lib_num_to_bytes_be(t_card_4k[verify_sector].a, COUNT_OF(key.data), key.data);

    MfClassicAuthContext auth_ctx = {};
    MfClassicError error =
        mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);

    return error == MfClassicErrorNone;
}

static bool sk_tk_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    bool is_read = false;
    MfClassicType type = MfClassicType4k;
    MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);

    if(error == MfClassicErrorNone) {
        data->type = type;
        MfClassicDeviceKeys keys = {};

        for(size_t i = 0; i < mf_classic_get_total_sectors_num(data->type); i++) {
            bit_lib_num_to_bytes_be(t_card_4k[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(t_card_4k[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
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

static bool sk_tk_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;

    do {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 19);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != t_card_4k[19].a) break;

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

static const NfcSupportedCardsPlugin sk_tk_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = sk_tk_verify,
    .read = sk_tk_read,
    .parse = sk_tk_parse,
};

static const FlipperAppPluginDescriptor sk_tk_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &sk_tk_plugin,
};

const FlipperAppPluginDescriptor* sk_tk_plugin_ep(void) {
    return &sk_tk_plugin_descriptor;
}
