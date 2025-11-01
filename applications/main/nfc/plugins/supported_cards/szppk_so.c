
#include "nfc_supported_card_plugin.h"

#include <flipper_application/flipper_application.h>

#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <datetime.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#define TAG "SZPPK_SO"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    uint16_t station_id;
    const char* station_name;
} StationMap;

const StationMap station_map[] = {
    {0x9426, "LADOZH.VOKZ."},
    {0x9421, "MOSKOV.VOKZ."},
    {0X9423, "VITEBS.VOKZ"},
    {0x9424, "FINLND.VOKZ"},
    {0X9425, "BALT.VOKZ"},
    {0X948B, "IM.MOROZOVA"},
    {0X94B4, "TOSNO"},
    {0X94B5, "SABLINO"},
    {0X94BE, "GORY"},
    {0X94C0, "IZHORY"},
    {0X94C1, "RYBATSKOE"},
    {0X24C2, "OBUKHOVO"},
    {0X94C3, "SLAVYANKA"},
    {0X94C4, "KOLPINO"},
    {0X94C9, "MSHINSKAYA"},
    {0X94CC, "STROGANOVO"},
    {0X94CD, "SUI'DA"},
    {0X94CE, "GATCHINA BLT."},
    {0X94D4, "VYRITSA"},
    {0X94D5, "PAVLOVSK"},
    {0X94D6, "TSARSKOE SELO"},
    {0X94E2, "KALISHCHE"},
    {0X94E3, "LEBYAZHYE"},
    {0X94E4, "BOLSH.IZHORA"},
    {0X9507, "BELOOSTROV"},
    {0X9508, "ZELENOGORSK"},
    {0X9511, "RUCH'I"},
    {0X9512, "TOKSOVO"},
    {0X9513, "OSEL'KI"},
    {0X9514, "PERI"},
    {0X9515, "GRUZINO"},
    {0X9516, "VASKELOVO"},
    {0X9517, "OREKHOVO"},
    {0X9518, "PETYAJARVI"},
    {0X9519, "LOSEVO"},
    {0X951A, "GROMOVO"},
    {0X951F, "KUZNECHNOE"},
    {0X95C3, "KAPITOLOVO"},
    {0X969A, "MGA"},
    {0X969C, "LUGA"},
    {0X969D, "TOLMACHEVO"},
    {0X969E, "SIVERSKAYA"},
    {0X969F, "GATCHINA VRSH."},
    {0X96A1, "OREDEZH"},
    {0X96A3, "SLANTSY"},
    {0X96A5, "ORANIENBAUM 1"},
    {0X96A8, "VOLOSOVO"},
    {0X96BD, "TIKHVIN"},
    {0X96C0, "VOLKHOVSTROY 1"},
    {0X96C1, "VOLKHOVSTROY 2"},
    {0X96CA, "VYBORG"},
    {0X96CB, "SOSNOVO"},
    {0X96CC, "PRIOZYORSK"},
    {0X970A, "BOLOTISTOE"},
    {0X971D, "STAR.DEREVNYA"},
    {0X9721, "54 KM"},
    {0X9723, "KOLOSKOVO(79KM)"},
    {0X972C, "PARAVOZNY MUZEI"},
    {0X9741, "BOROVAYA"},
    {0X97BB, "DETSKOSELSKAYA"},
    {0X9809, "KUPCHINO"},
    {0X9810, "PETROKREPOST'"},
    {0X9811, "ALEKSANDROVSK.F"},
    {0X9817, "BERNGARDOVKA"},
    {0X981C, "PROSPEKT SLAVY"},
    {0X9826, "SOSNOVAYA POL."},
    {0X9829, "PUDOST'"},
    {0X982B, "LENINSKI' PR."},
    {0X982D, "PAVLOVO-NA-NEVE"},
    {0X9835, "LANSKAYA"},
    {0X9838, "KANNELYARVI"},
    {0X983A, "MELN.RUCHEI"},
    {0X983F, "PISKARYOVKA"},
    {0X9849, "UDEL'NAYA"},
    {0X9852, "BRONKA"},
    {0X9853, "VSEVOLOZHSK."},
    {0X985D, "FARFOROVSKAYA"},
    {0X9867, "RZHEVKA"},
    {0X986B, "AEROPORT"},
    {0X9871, "BRONEVAYA"},
    {0X987B, "SESTRORETSK"},
    {0X9880, "TATIANINO"},
    {0X988A, "KUSHELEVKA"},
    {0X9890, "RAKH'YA"},
    {0X9894, "UL'YANKA"},
    {0X9899, "PESOCHNAYA"},
    {0X989A, "NAVALOCHNAYA"},
    {0X989E, "STREL'NA"},
    {0X98A3, "ST.PETERGOF"},
    {0X98A5, "KOLTUSHI"},
    {0X98A8, "KUZ'MOLOVO"},
    {0X98AD, "KRASN. SELO"},
    {0X98AE, "SHUSHARY"},
    {0X98BC, "GORELOVO"},
    {0X98C1, "ROSHCHINO"},
    {0X98CB, "LISII' NOS"},
    {0X98D5, "DACHNOYE"},
    {0X98D6, "PREDPORTOVAYA"},
    {0X98E4, "NOVAYA OKHTA"},
    {0X98E9, "REPINO"},
    {0X98F3, "LEVASHOVO"},
    {0X98FD, "NOV.DEREVNYA"},
    {0X98FE, "IZHORSK.ZAVOD"},
    {0X9908, "VAGANOVO"},
    {0X990C, "SHUVALOVO"},
    {0X9911, "DUNAI'"},
    {0X9916, "IRINOVKA"},
    {0X991C, "TARKHOVKA"},
    {0X9923, "PUPYSHEVO"},
    {0X9925, "TAI'TSY"},
    {0X9926, "LAVRIKI"},
    {0X992A, "LADOZHSK.OZ."},
    {0X992D, "ROMANOVKA"},
    {0X9936, "RAZLIV"},
    {0X9938, "KIRILLOVSKOE"},
    {0X9939, "OL'GINO"},
    {0X993D, "METALLOSTROY"},
    {0X993E, "KAVGOLOVO"},
    {0X9943, "VOZD.PARK"},
    {0X9944, "POST KOVALEVO"},
    {0X9949, "LAPPELOVO"},
    {0X994D, "KORNEVO"},
    {0X9952, "SOLNECHNOYE"}, 
    {0X9956, "MYAGLOVO"},
    {0X995B, "OZERKI"},
    {0X995C, "KOMAROVO"},
    {0X9969, "PROBA"},
    {0X996A, "KIRPICH.ZAVOD"},
    {0X9970, "KURORT"},
    {0X9975, "67 KM"},
    {0X9976, "USHKOVO"},
    {0X9981, "DUDERGOF"},
    {0X9989, "UNIVERSITETSK."},
    {0X9992, "YAKHTENNAYA"},
    {0X9996, "KOVALEVO"},
    {0X9997, "DIBUNY"},
    {0X99AF, "MANUSHKINO"},
    {0X99CF, "PARGOLOVO"},
    {0X99D4, "LIGOVO"},
    {0X9C09, "DEVYATKINO"},
    {0X9C58, "NEV.DUBROVKA"},
    {0X9C5A, "ALEKSANDROVSK."},
    {0X9C5B, "GORSKAYA"},
    {0X9D1C, "NOV.PETERGOF"},
    {0X9FDC, "BORISOVA GRIVA"},
    {0X98B7, "SERGIEVO"},
    {0X9957, "LAKHTA"}

};

const size_t num_station_map_entries = sizeof(station_map) / sizeof(station_map[0]);

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

    bool parsed = false;
    do {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 19);
        uint64_t key = bit_lib_bytes_to_num_be(sec_tr->key_a.data, 6);
        if(key != so_card_2k[19].a) break;

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
        bool card_type =
            (departure_station == destination_station) ?
                0 :
                1; //Accomp. cards will have te same station ID for Departure and Destination
        datetime_timestamp_to_datetime(valid_from_timestamp, &v_from);
        datetime_timestamp_to_datetime(valid_till_timestamp, &v_till);
        datetime_timestamp_to_datetime(tap_timestamp, &tap_time);

        if(departure_station ==
           0x0000) //if the ticket is not issued (unissued tickets will have a 0x0000 as a departure station ID)
            furi_string_cat_printf(
                parsed_data,
                "\e#Unknown SZPPK Card\n-NO TICKET DATA FOUND-\nTHE TICKET IS NOT ISSUED\nOR LAYOUT IS UNKNOWN\n");

        else { //if the ticket is issued

            if(card_type == 0) { //if the ticket is an Accomp. Card
                furi_string_cat_printf(
                    parsed_data,
                    "\e#SZPPK Accompany Card\nValid on: %02d-%02d-%04d\n",
                    v_from.day,
                    v_from.month,
                    v_from.year);

                for(size_t i = 0; i < num_station_map_entries; ++i) {
                    if(departure_station == station_map[i].station_id) {
                        furi_string_cat_printf(
                            parsed_data, "Station: > %s\n", station_map[i].station_name);
                        departure_is_known = 1;
                    }
                }

                if(departure_is_known == 0)
                    furi_string_cat_printf(parsed_data, "Station ID: %04x\n", departure_station);

                if(current_status == 0x0000)
                    furi_string_cat_printf(parsed_data, "Status:> NOT USED\n");
                else if(current_status == 0x2180) //if status == IN
                    furi_string_cat_printf(
                        parsed_data,
                        "Status:> ENTERED STATION\nChecked in at:> %02d:%02d\n",
                        tap_time.hour,
                        tap_time.minute);

                else if(current_status == 0x211E) //if status == OUT
                    furi_string_cat_printf(
                        parsed_data,
                        "Status:> EXITED STATION\nChecked out at:> %02d:%02d\n",
                        tap_time.hour,
                        tap_time.minute);
                else
                    furi_string_cat_printf(parsed_data, "Status:> UNKNOWN");
            }

            else {
                furi_string_cat_printf(
                    parsed_data,
                    "\e#SZPPK Transport Card\nValid from: %02d-%02d-%04d\nValid till:      %02d-%02d-%04d\n",
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
                        parsed_data, "Departure st. ID: %04x\n", departure_station);

                for(size_t i = 0; i < num_station_map_entries; ++i) {
                    if(destination_station == station_map[i].station_id) {
                        furi_string_cat_printf(
                            parsed_data, "To:> %s\n", station_map[i].station_name);
                        destination_is_known = 1;
                    }
                }
                if(destination_is_known == 0)
                    furi_string_cat_printf(
                        parsed_data, "Destination st. ID: %04x\n", destination_station);

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
                } else if(current_status == 0x211E) {
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
        }

        parsed = true;
    } while(false);

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
