
#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <datetime.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "SKPPK_TK"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    uint16_t station_id;
    const char* station_name;
} StationMap;

const StationMap station_map[] = {
    {0X8377, "OP 1725 KM"},
    {0X8350, "STEKOL'NYI"},
    {0X834F, "SELHOZTECHNIKA"},
    {0X834D, "KANGLY"},
    {0X834B, "OP 1861 KM"},
    {0X834A, "OP 1856 KM"},
    {0X8349, "OP 1842 KM"},
    {0X8348, "OP 1832 KM"},
    {0X8347, "OP 1806 KM"},
    {0X8344, "OP 44 KM"},
    {0X8343, "OP 129 KM"},
    {0X8288, "SUVOROVSKAYA"},
    {0X8188, "NESKUCHNYI"},
    {0X8183, "OP 1741 KM"},
    {0X8182, "KOCHUBEEVSKII"},
    {0X817E, "SURKUL"},
    {0X817D, "OP 1823 KM"},
    {0X8177, "TERSKII"},
    {0X8176, "KRASNAYA"},
    {0X8175, "TEPLAYA RECHKA"},
    {0X8165, "DVORTSOVYI"},
    {0X8164, "PIONER"},
    {0X8161, "ORBEL'YANOVO"},
    {0X815F, "DZHEMUHA"},
    {0X815D, "ETOKA"},
    {0X8158, "OBIL'NYI"},
    {0X8157, "NINY"},
    {0X813C, "STAVROPOL"},
    {0X8104, "KARMALINOVSKII"},
    {0X8084, "KRASN.DEREVNIA"},
    {0X807E, "MIHAILOVSKAIA"},
    {0X7FF8, "ZMEIKA"},
    {0X7FF6, "MASHUK"},
    {0X7FF5, "LERMONTOVSKII"},
    {0X7FF4, "NOVOPIATIGORSK"},
    {0X7FF3, "SKACHKI"},
    {0X7FF2, "ZOLOTUSHKA"},
    {0X7FF1, "BELYI UGOL'"},
    {0X7FF0, "PODKUMOK"},
    {0X7FEF, "MINUTKA"},
    {0X7FBA, "PLAKSEIKA"},
    {0X7FB9, "MASLOV KUT"},
    {0X7FB8, "ZELENOKUMSK"},
    {0X7FB7, "KUMA"},
    {0X7FB6, "GEORGIEVSK"},
    {0X7FA6, "VIAZNIKI"},
    {0X7FA4, "YAGODKA"},
    {0X7FA1, "MAIAK"},
    {0X7F9B, "BEDENNOVSK"},
    {0X7F6A, "PALAGIADA"},
    {0X7F69, "RYZDVIANAIA"},
    {0X7F68, "PEREDOVAIA"},
    {0X7F65, "GRIGOROPOLISSK."},
    {0X7F5C, "NEVINNOMYSSK."},
    {0X7F2B, "INOZEMTSEVO"},
    {0X7F2A, "MIN.VODY"},
    {0X7F1B, "ZHELEZNOVODSK"},
    {0X7F17, "BESHTAU"},
    {0X7EFD, "RASSHEVATKA"},
    {0X7EBC, "PIATIGORSK"},
    {0X7EB2, "KISLOVODSK"},
    {0X7EA5, "ZOLSKII"},
    {0X7EA4, "VINOGRADNAIA"},
    {0X7EA2, "KUMAGORSK"},
    {0X7E9F, "NAGUTSKAIA"},
    {0X7E9D, "KURSAVKA"},
    {0X7E9B, "KIAN"},
    {0X7E9A, "ZELENCHUK"},
    {0X7E96, "BOGLOVSKAIA"},
    {0X7E95, "OVECHKA"},
    {0X8162, "KURSHAVA"},
    {0X7F97, "CHUKOTSKII"},
    {0X83AE, "DEBRI"},
    {0X8306, "DEGTIAREVSKI"},
    {0X8185, "NIVA"},
    {0X8160, "OP 1814 KM"},
    {0X815B, "CHISTAIA"},
    {0X7FA5, "BERMEDSKII"},
    {0X7F93, "IZOBILNAIA"},
    {0X7F64, "KRASNOKUBANSK."},
    {0X7ED0, "ESSENTUKI"},
    {0X7EA6, "APOLLONSKAIA"}

};

const size_t num_station_map_entries = sizeof(station_map) / sizeof(station_map[0]);

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

bool sk_tk_verify(Nfc* nfc) {
    bool verified = false;

    do {
        const uint8_t verify_sector = 19;
        uint8_t block_num = mf_classic_get_first_block_num_of_sector(verify_sector);

        MfClassicKey key = {};
        bit_lib_num_to_bytes_be(t_card_4k[verify_sector].a, COUNT_OF(key.data), key.data);

        MfClassicAuthContext auth_ctx = {};
        MfClassicError error =
            mf_classic_poller_sync_auth(nfc, block_num, &key, MfClassicKeyTypeA, &auth_ctx);
        if(error != MfClassicErrorNone) break;

        verified = true;
    } while(false);

    return verified;
}

static bool sk_tk_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);
    do {
        MfClassicType type = MfClassicType4k;
        MfClassicError error = mf_classic_poller_sync_detect_type(nfc, &type);
        if(error != MfClassicErrorNone) break;

        data->type = type;
        MfClassicDeviceKeys keys = {};
        for(size_t i = 0; i < 32; i++) {
            bit_lib_num_to_bytes_be(t_card_4k[i].a, sizeof(MfClassicKey), keys.key_a[i].data);
            FURI_BIT_SET(keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(t_card_4k[i].b, sizeof(MfClassicKey), keys.key_b[i].data);
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

static bool sk_tk_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);

    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    bool parsed = false;
    do {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 19);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != t_card_4k[19].a) break;

        uint8_t value_data = data->block[77].data[0];
        uint16_t departure_station = (data->block[76].data[6] << 8) | (data->block[76].data[5]);
        uint16_t destination_station = (data->block[76].data[9] << 8) | (data->block[76].data[8]);
        uint16_t current_status = (data->block[78].data[9] << 8) | (data->block[78].data[8]);
        uint16_t valid_from_date = (data->block[76].data[2] << 8) |
                                   (data->block[76].data[1]); //number of days since Jan 1st 2000
        uint16_t valid_till_date = (data->block[76].data[4] << 8) |
                                   (data->block[76].data[3]); //number of days since Jan 1st 2000
        uint32_t valid_from_timestamp = 946684800 + valid_from_date * 24 * 60 * 60;
        uint32_t valid_till_timestamp = 946684800 + valid_till_date * 24 * 60 * 60;
        uint32_t tap_data = 0;
        for(uint8_t i = 0; i < 3; i++) {
            tap_data = (tap_data << 8) | data->block[78].data[2 - i];
        }
        uint32_t tap_timestamp = 1388530800 + tap_data * 60;

        DateTime v_from = {0};
        DateTime v_till = {0};
        DateTime tap_time = {0};
        bool departure_is_known = 0;
        bool destination_is_known = 0;
        
        datetime_timestamp_to_datetime(valid_from_timestamp, &v_from);
        datetime_timestamp_to_datetime(valid_till_timestamp, &v_till);
        datetime_timestamp_to_datetime(tap_timestamp, &tap_time);

        if(departure_station ==
           0x0000) //if the ticket is not issued (unissued tickets will have a 0x0000 as a departure station ID)
            furi_string_cat_printf(
                parsed_data,
                "\e#Unkown SKPPK Card\n-NO TICKET DATA FOUND-\nTHE TICKET IS NOT ISSUED\nOR LAYOUT IS UNKNOWN\n");

        else { //if the ticket is issued
                furi_string_cat_printf(
                    parsed_data,
                    "\e#SKPPK Transport Card\nValid from: %02d-%02d-%04d\nValid till:      %02d-%02d-%04d\n",
                    v_from.day,
                    v_from.month,
                    v_from.year,
                    v_till.day,
                    v_till.month,
                    v_till.year);

                for(size_t i = 0; i < num_station_map_entries; ++i) {
                    if(departure_station == station_map[i].station_id) {
                        furi_string_cat_printf(
                            parsed_data, "From:> %s\n", station_map[i].station_name);
                        departure_is_known = 1;
                    }
                }
                if(departure_is_known == 0)
                    furi_string_cat_printf(
                        parsed_data, "Departure UIC: 1F%04x\n", departure_station);

                for(size_t i = 0; i < num_station_map_entries; ++i) {
                    if(destination_station == station_map[i].station_id) {
                        furi_string_cat_printf(
                            parsed_data, "To:> %s\n", station_map[i].station_name);
                        destination_is_known = 1;
                    }
                }
                if(destination_is_known == 0)
                    furi_string_cat_printf(
                        parsed_data, "Destination UIC: 1F%04x\n", destination_station);

                if(value_data > 0)
                    furi_string_cat_printf(parsed_data, "Rides remain: %02d\n", value_data);

                if(current_status == 0x0000) {
                    furi_string_cat_printf(parsed_data, "Status:> NOT USED\n");
                } else if(current_status == 0x2180) {
                    furi_string_cat_printf(
                        parsed_data,
                        "Status:> ENTERED STATION\nLast pass:> %02d-%02d-%04d\nPass time:> %02d:%02d\n",
                        tap_time.day,
                        tap_time.month,
                        tap_time.year,
                        tap_time.hour,
                        tap_time.minute);
                } else if(current_status == 0x211F) {
                    furi_string_cat_printf(
                        parsed_data,
                        "Status:> EXITED STATION\nLast pass:> %02d-%02d-%04d\nPass time:> %02d:%02d\n",
                        tap_time.day,
                        tap_time.month,
                        tap_time.year,
                        tap_time.hour,
                        tap_time.minute);
                } else {
                    furi_string_cat_printf(
                        parsed_data, "Status:> UNKNOWN (%04X)\n", current_status);
                }
            }
        

        parsed = true;
    } while(false);

    return parsed;
}

/* Actual implementation of app<>plugin interface */
static const NfcSupportedCardsPlugin sk_tk_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = sk_tk_verify,
    .read = sk_tk_read,
    .parse = sk_tk_parse,
};

/* Plugin descriptor to comply with basic plugin specification */
static const FlipperAppPluginDescriptor sk_tk_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &sk_tk_plugin,
};

/* Plugin entry point - must return a pointer to const descriptor  */
const FlipperAppPluginDescriptor* sk_tk_plugin_ep(void) {
    return &sk_tk_plugin_descriptor;
}
