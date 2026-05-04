//Based on parsers written by Leptoptilos and Assasinfil. Also, thanks to WillyJL (<me@willyjl.dev>) for help!

#include "nfc_supported_card_plugin.h"
#include <flipper_application/flipper_application.h>
#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <datetime.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_format/flipper_format.h>
#define PPK_WHOLE_EPOCH_START     946684800 //2000-01-01
#define PPK_CURRENT_EPOCH_START   1388534400 //1388530800 //2014-01-01
#define SECONDS_IN_A_DAY          86400
#define SECONDS_IN_A_MINUTE       60
#define FIRST_PPK_TICKET_OFFSET   76
#define FIRST_TICKET_VALUE_BLOCK  77
#define SECOND_PPK_TICKET_OFFSET  88
#define SECOND_TICKET_VALUE_BLOCK 89

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

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
    uint8_t current_status;
    uint16_t valid_from_data;
    uint16_t valid_till_data;
    DateTime valid_from;
    DateTime valid_till;
    uint32_t tap_data;
    DateTime tap_time;
    uint8_t first_ticket_marker;
    uint8_t second_ticket_marker;
    uint8_t ppk_cnt;
} TicketData;

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

static inline void sz_uic_to_sta(Storage* storage, const char* file_name, TicketData* ticket) {
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
static void resolve_station_name(Storage* storage, TicketData* ticket) {
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

static inline void extract_ppk_data(
    Storage* storage,
    const MfClassicData* data,
    TicketData* ticket,
    bool ticket_number) {
    if(ticket_number == 0) {
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
        ticket->tap_data = (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[2] << 16) |
                           (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[1] << 8) |
                           data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[0];
        ticket->ppk_cnt = data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[10];
        ticket->trip_start_uic = (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[4] << 8) |
                                 (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[3]);
        ticket->trip_end_uic = (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[7] << 8) |
                               (data->block[FIRST_TICKET_VALUE_BLOCK + 1].data[6]);

    } else {
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
        ticket->tap_data = (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[2] << 16) |
                           (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[1] << 8) |
                           data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[0];
        ticket->ppk_cnt = data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[10];
        ticket->trip_start_uic = (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[4] << 8) |
                                 (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[3]);
        ticket->trip_end_uic = (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[7] << 8) |
                               (data->block[SECOND_TICKET_VALUE_BLOCK + 1].data[6]);
    }
    ticket->first_ticket_marker = data->block[FIRST_PPK_TICKET_OFFSET].data[0];
    ticket->second_ticket_marker = data->block[SECOND_PPK_TICKET_OFFSET].data[0];
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

static inline bool is_accompany_card(uint16_t departure_uic, uint16_t destination_uic) {
    return departure_uic == destination_uic;
}

static void printf_accompany_card(TicketData* ticket, FuriString* parsed_data) {
    if(ticket->departure_uic == 0x0000) {
        furi_string_cat_printf(
            parsed_data,
            "\e#Unknown SZPPK Card\n   NO TICKET DATA FOUND \n\nTHE TICKET IS NOT ISSUED\nOR LAYOUT IS UNKNOWN\n");
        return;
    } else {
        furi_string_cat_printf(
            parsed_data,
            "\e#SZPPK Accompany Card\nValid on: %02d-%02d-%04d\nStation: > %s\n",
            ticket->valid_from.day,
            ticket->valid_from.month,
            ticket->valid_from.year,
            furi_string_get_cstr(ticket->departure_name));

        switch(ticket->current_status) {
        case 0x00:
            furi_string_cat_printf(
                parsed_data, "Status:> NOT USED\nPPK CNT: %03d\n", ticket->ppk_cnt);
            break;
        case 0x80:
            furi_string_cat_printf(
                parsed_data,
                "Status:> ENTERED STATION\nChecked in at:> %02d:%02d\nPPK CNT: %03d\n",
                ticket->tap_time.hour,
                ticket->tap_time.minute,
                ticket->ppk_cnt);
            break;
        case 0x1E:
            furi_string_cat_printf(
                parsed_data,
                "Status:> EXITED STATION\nChecked out at:> %02d:%02d\nPPK CNT: %03d\n",
                ticket->tap_time.hour,
                ticket->tap_time.minute,
                ticket->ppk_cnt);
            break;
        default:
            furi_string_cat_printf(
                parsed_data, "Status:> UNKNOWN\nPPK CNT: %03d\n", ticket->ppk_cnt);
            break;
        }
    }
}

static void
    printf_transport_card(FuriString* parsed_data, TicketData* ticket, bool ticket_number) {
    if(ticket->departure_uic == 0x0000) {
        furi_string_cat_printf(
            parsed_data,
            "\e#Unknown SZPPK Card\n   NO TICKET DATA FOUND \n\nTHE TICKET IS NOT ISSUED\nOR LAYOUT IS UNKNOWN\n");
        return;
    } else {
        if(ticket_number == 0) {
            furi_string_cat_printf(parsed_data, "\e#SZPPK Transport Card\n");
            switch(ticket->first_ticket_marker) {
            case 0x02:
                furi_string_cat_printf(parsed_data, "Type:> 5 days unlim.");
                break;
            case 0x06:
                furi_string_cat_printf(parsed_data, "Type:> 10 rides");
                break;
            case 0x18:
                furi_string_cat_printf(parsed_data, "Type:> 30 days unlim.");
                break;
            case 0x1B:
                furi_string_cat_printf(parsed_data, "Type:> 20 rides.");
                break;
            default:
                furi_string_cat_printf(
                    parsed_data, "Type: Unknown, 0x%02X\n", ticket->first_ticket_marker);
            }
        } else {
            furi_string_cat_printf(parsed_data, "\e#Second Ticket:\n");
            switch(ticket->second_ticket_marker) {
            case 0x02:
                furi_string_cat_printf(parsed_data, "Type:> 5 days unlim.");
                break;
            case 0x06:
                furi_string_cat_printf(parsed_data, "Type:> 10 rides");
                break;
            case 0x18:
                furi_string_cat_printf(parsed_data, "Type:> 30 days unlim.");
                break;
            case 0x1B:
                furi_string_cat_printf(parsed_data, "Type:> 20 rides.");
                break;
            default:
                furi_string_cat_printf(
                    parsed_data, "Type: Unknown, 0x%02X", ticket->second_ticket_marker);
            }
        }

        furi_string_cat_printf(
            parsed_data,
            "\nFrom:>%s\nTo:>%s\nValid From: %02d-%02d-%04d\nValid thru:  %02d-%02d-%04d",
            furi_string_get_cstr(ticket->departure_name),
            furi_string_get_cstr(ticket->destination_name),
            ticket->valid_from.day,
            ticket->valid_from.month,
            ticket->valid_from.year,
            ticket->valid_till.day,
            ticket->valid_till.month,
            ticket->valid_till.year);

        if(ticket->value_data > 0) {
            furi_string_cat_printf(parsed_data, "Rides remain: %02d\n", ticket->value_data);
        }

        switch(ticket->current_status) {
        case 0x00:
            furi_string_cat_printf(
                parsed_data, "Status:> NOT USED\nPPK CNT: %03d\n", ticket->ppk_cnt);
            break;
        case 0x80:
            furi_string_cat_printf(
                parsed_data,
                "\nStatus:> ENTERED STATION\nSta name:> %s\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\nPPK CNT: %03d\n",
                furi_string_get_cstr(ticket->trip_start_sta_name),
                ticket->tap_time.day,
                ticket->tap_time.month,
                ticket->tap_time.year,
                ticket->tap_time.hour,
                ticket->tap_time.minute,
                ticket->ppk_cnt);
            break;
        case 0x1E:
            furi_string_cat_printf(
                parsed_data,
                "\nStatus:> EXITED STATION\nSta name:> %s\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\nPPK CNT: %03d\n",
                furi_string_get_cstr(ticket->trip_end_sta_name),
                ticket->tap_time.day,
                ticket->tap_time.month,
                ticket->tap_time.year,
                ticket->tap_time.hour,
                ticket->tap_time.minute,
                ticket->ppk_cnt);
            break;
        default:
            furi_string_cat_printf(
                parsed_data,
                "Status:> UNKNOWN (%02X)\nPPK CNT: %03d",
                ticket->current_status,
                ticket->ppk_cnt);
            break;
        }
    }
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
        primary_ticket.departure_name = furi_string_alloc();
        primary_ticket.destination_name = furi_string_alloc();
        primary_ticket.trip_start_sta_name = furi_string_alloc();
        primary_ticket.trip_end_sta_name = furi_string_alloc();
        extract_ppk_data(storage, data, &primary_ticket, 0);
        if(is_accompany_card(primary_ticket.departure_uic, primary_ticket.destination_uic)) {
            printf_accompany_card(&primary_ticket, parsed_data);
        } else {
            printf_transport_card(parsed_data, &primary_ticket, false);
        }
        furi_string_free(primary_ticket.departure_name);
        furi_string_free(primary_ticket.destination_name);
        furi_string_free(primary_ticket.trip_start_sta_name);
        furi_string_free(primary_ticket.trip_end_sta_name);

        if(primary_ticket.second_ticket_marker != 0) {
            TicketData secondary_ticket = {0};
            secondary_ticket.departure_name = furi_string_alloc();
            secondary_ticket.destination_name = furi_string_alloc();
            secondary_ticket.trip_start_sta_name = furi_string_alloc();
            secondary_ticket.trip_end_sta_name = furi_string_alloc();

            extract_ppk_data(storage, data, &secondary_ticket, 1);
            printf_transport_card(parsed_data, &secondary_ticket, true);
            furi_string_free(primary_ticket.departure_name);
            furi_string_free(primary_ticket.destination_name);
            furi_string_free(primary_ticket.trip_start_sta_name);
            furi_string_free(primary_ticket.trip_end_sta_name);
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
