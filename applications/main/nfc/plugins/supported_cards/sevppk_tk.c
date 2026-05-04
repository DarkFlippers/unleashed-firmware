//Based on parsers written by Leptoptilos and Assasinfil. Also, thanks to WillyJL (<me@willyjl.dev>) for help!

#include "nfc_supported_card_plugin.h"
#include <flipper_application/flipper_application.h>
#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <datetime.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <flipper_format/flipper_format.h>
#define PPK_WHOLE_EPOCH_START     946684800 //2000-01-01
#define PPK_CURRENT_EPOCH_START   1388534400 //2014-01-01
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

static const MfClassicKeyPair t_card_2k[] = {
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //0
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //1
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //2
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //3
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //4
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //5
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //6
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //7
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //8
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //9
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //10
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //11
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //12
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //13
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //14
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //15

    {.a = 0x061029A4A6A4, .b = 0xCF43E52FC23D}, //16
    {.a = 0xE09F2EC8229C, .b = 0x044D78B70F92}, //17
    {.a = 0x66843B4FF86E, .b = 0x4474EB131577}, //18
    {.a = 0x9AC9A9B5E809, .b = 0x558B8E45E568}, //19
    {.a = 0xC114DE1AD90F, .b = 0xA0A9033E2A09}, //20
    {.a = 0xF696EFFDD838, .b = 0xA38333EC3F72}, //21
    {.a = 0xAB50BDE9CD04, .b = 0x67991D678AED}, //22
    {.a = 0x9041DD7B236C, .b = 0x8A527C8CA237}, //23
    {.a = 0x7D40A5AA63D0, .b = 0x4A61C563DD8A}, //24
    {.a = 0x05D0520EC52B, .b = 0xB40BD14E6CB4}, //25
    {.a = 0x9354F1BFF80F, .b = 0xF99D40CBBA63}, //26
    {.a = 0x84132D9FD1AF, .b = 0x1BAA2563C14D}, //27
    {.a = 0x27A8E6436B01, .b = 0x4DB883715A5C}, //28
    {.a = 0x58B173568D26, .b = 0x2E4ADD66E35E}, //29
    {.a = 0x1BE71601E73C, .b = 0xB855F30C54FB}, //30
    {.a = 0xFFFFFFFFFFFF, .b = 0xFFFFFFFFFFFF}, //31

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

static void resolve_station_name(Storage* storage, TicketData* ticket) {
    sz_uic_to_sta(storage, EXT_PATH("nfc/assets/sev_id.nfc"), ticket);
    if(furi_string_utf8_length(ticket->departure_name) <= 2) {
        furi_string_printf(ticket->departure_name, "1f%04X", ticket->departure_uic);
    }
    if(furi_string_utf8_length(ticket->destination_name) <= 2) {
        furi_string_printf(ticket->destination_name, "1F%04X", ticket->destination_uic);
    }
    if(furi_string_utf8_length(ticket->trip_start_sta_name) <= 2) {
        furi_string_printf(ticket->trip_start_sta_name, "1F%04X", ticket->trip_start_uic);
    }
    if(furi_string_utf8_length(ticket->trip_end_sta_name) <= 2) {
        furi_string_printf(ticket->trip_end_sta_name, "1F%04X", ticket->trip_end_uic);
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

static void
    printf_transport_card(FuriString* parsed_data, TicketData* ticket, bool ticket_number) {
    if(ticket->departure_uic == 0x0000) {
        furi_string_cat_printf(
            parsed_data,
            "\e#Unknown SevPPK Card\n   NO TICKET DATA FOUND \n\nTHE TICKET IS NOT ISSUED\nOR LAYOUT IS UNKNOWN\n");
        return;
    } else {
        if(ticket_number == 0) {
            furi_string_cat_printf(parsed_data, "\e#SevPPK Transport Card\n");
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
                furi_string_cat_printf(parsed_data, "Type:> 20 rides");
                break;
            default:
                furi_string_cat_printf(
                    parsed_data, "Type: Unknown, 0x%02X", ticket->first_ticket_marker);
            }
        } else {
            furi_string_cat_printf(parsed_data, "\e#Second Ticket:");
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
                furi_string_cat_printf(parsed_data, "Type:> 20 rides");
                break;
            default:
                furi_string_cat_printf(
                    parsed_data, "Type: Unknown, 0x%02X", ticket->second_ticket_marker);
            }
        }
    }

    furi_string_cat_printf(
        parsed_data,
        "\nFrom:>%s\nTo:>%s\nValid From: %02d-%02d-%04d\nValid thru:  %02d-%02d-%04d\n",
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
        furi_string_cat_printf(parsed_data, "Status:> NOT USED\n");
        break;
    case 0x80:
        furi_string_cat_printf(
            parsed_data,
            "Status:> ENTERED STATION\nSta name:> %s\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\nPPK CNT: %03d\n",
            furi_string_get_cstr(ticket->trip_start_sta_name),
            ticket->tap_time.day,
            ticket->tap_time.month,
            ticket->tap_time.year,
            ticket->tap_time.hour,
            ticket->tap_time.minute,
            ticket->ppk_cnt);
        break;
    case 0x1F:
        furi_string_cat_printf(
            parsed_data,
            "Status:> EXITED STATION\nSta name:> %s\nLast pass on:> %02d-%02d-%04d\nPass time:> %02d:%02d\nPPK CNT: %03d\n",
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

bool sev_tk_verify(Nfc* nfc) {
    const uint8_t verify_sector = 19;
    const uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);

    MfClassicKey key = {};
    bit_lib_num_to_bytes_be(t_card_2k[verify_sector].a, COUNT_OF(key.data), key.data);

    MfClassicAuthContext auth_ctx = {};
    MfClassicError error =
        mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);

    return error == MfClassicErrorNone;
}

static bool sev_tk_read(Nfc* nfc, NfcDevice* device) {
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
            bit_lib_num_to_bytes_be(t_card_2k[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(t_card_2k[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
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

static bool sev_tk_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;

    do {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 19);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != t_card_2k[19].a) break;

        Storage* storage = furi_record_open(RECORD_STORAGE);
        TicketData primary_ticket = {0};
        primary_ticket.departure_name = furi_string_alloc();
        primary_ticket.destination_name = furi_string_alloc();
        primary_ticket.trip_start_sta_name = furi_string_alloc();
        primary_ticket.trip_end_sta_name = furi_string_alloc();
        extract_ppk_data(storage, data, &primary_ticket, 0);

        printf_transport_card(parsed_data, &primary_ticket, false);

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

static const NfcSupportedCardsPlugin sev_tk_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = sev_tk_verify,
    .read = sev_tk_read,
    .parse = sev_tk_parse,
};

static const FlipperAppPluginDescriptor sev_tk_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &sev_tk_plugin,
};

const FlipperAppPluginDescriptor* sev_tk_plugin_ep(void) {
    return &sev_tk_plugin_descriptor;
}
