#include "nfc_supported_card.h"

#include <gui/modules/widget.h>
#include <nfc_worker_i.h>

static const MfClassicAuthContext troyka_keys[] = {
    {.sector = 0, .key_a = 0xa0a1a2a3a4a5, .key_b = 0xfbf225dc5d58},
    {.sector = 1, .key_a = 0xa82607b01c0d, .key_b = 0x2910989b6880},
    {.sector = 2, .key_a = 0x2aa05ed1856f, .key_b = 0xeaac88e5dc99},
    {.sector = 3, .key_a = 0x2aa05ed1856f, .key_b = 0xeaac88e5dc99},
    {.sector = 4, .key_a = 0x73068f118c13, .key_b = 0x2b7f3253fac5},
    {.sector = 5, .key_a = 0xfbc2793d540b, .key_b = 0xd3a297dc2698},
    {.sector = 6, .key_a = 0x2aa05ed1856f, .key_b = 0xeaac88e5dc99},
    {.sector = 7, .key_a = 0xae3d65a3dad4, .key_b = 0x0f1c63013dba},
    {.sector = 8, .key_a = 0xa73f5dc1d333, .key_b = 0xe35173494a81},
    {.sector = 9, .key_a = 0x69a32f1c2f19, .key_b = 0x6b8bd9860763},
    {.sector = 10, .key_a = 0x9becdf3d9273, .key_b = 0xf8493407799d},
    {.sector = 11, .key_a = 0x08b386463229, .key_b = 0x5efbaecef46b},
    {.sector = 12, .key_a = 0xcd4c61c26e3d, .key_b = 0x31c7610de3b0},
    {.sector = 13, .key_a = 0xa82607b01c0d, .key_b = 0x2910989b6880},
    {.sector = 14, .key_a = 0x0e8f64340ba4, .key_b = 0x4acec1205d75},
    {.sector = 15, .key_a = 0x2aa05ed1856f, .key_b = 0xeaac88e5dc99},
};

bool troyka_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);
    UNUSED(nfc_worker);

    MfClassicAuthContext auth_ctx = {
        .key_a = MF_CLASSIC_NO_KEY,
        .key_b = MF_CLASSIC_NO_KEY,
        .sector = 8,
    };
    return mf_classic_auth_attempt(tx_rx, &auth_ctx, 0xa73f5dc1d333);
}

bool troyka_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);

    MfClassicReader reader = {};
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    reader.type = mf_classic_get_classic_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak);

    for(size_t i = 0; i < COUNT_OF(troyka_keys); i++) {
        mf_classic_reader_add_sector(
            &reader, troyka_keys[i].sector, troyka_keys[i].key_a, troyka_keys[i].key_b);
    }

    return mf_classic_read_card(tx_rx, &reader, &nfc_worker->dev_data->mf_classic_data) == 16;
}

bool troyka_parser_parse(NfcDeviceData* dev_data) {
    MfClassicData* data = &dev_data->mf_classic_data;
    bool troyka_parsed = false;

    do {
        // Verify key
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 8);
        uint64_t key = nfc_util_bytes2num(sec_tr->key_a, 6);
        if(key != troyka_keys[8].key_a) break;

        // Parse data
        uint8_t* temp_ptr = &data->block[8 * 4 + 1].value[5];
        uint16_t balance = ((temp_ptr[0] << 8) | temp_ptr[1]) / 25;
        temp_ptr = &data->block[8 * 4].value[3];
        uint32_t number = 0;
        for(size_t i = 0; i < 4; i++) {
            number <<= 8;
            number |= temp_ptr[i];
        }
        number >>= 4;

        string_printf(
            dev_data->parsed_data, "\e#Troyka\nNum: %ld\nBalance: %d rur.", number, balance);
        troyka_parsed = true;
    } while(false);

    return troyka_parsed;
}
