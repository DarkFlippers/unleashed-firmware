#include "nfc_supported_card.h"

#include <gui/modules/widget.h>
#include <nfc_worker_i.h>

static const MfClassicAuthContext troika_4k_keys[] = {
    {.sector = 0, .key_a = 0xa0a1a2a3a4a5, .key_b = 0xfbf225dc5d58},
    {.sector = 1, .key_a = 0xa82607b01c0d, .key_b = 0x2910989b6880},
    {.sector = 2, .key_a = 0x2aa05ed1856f, .key_b = 0xeaac88e5dc99},
    {.sector = 3, .key_a = 0x2aa05ed1856f, .key_b = 0xeaac88e5dc99},
    {.sector = 4, .key_a = 0x73068f118c13, .key_b = 0x2b7f3253fac5},
    {.sector = 5, .key_a = 0xFBC2793D540B, .key_b = 0xd3a297dc2698},
    {.sector = 6, .key_a = 0x2aa05ed1856f, .key_b = 0xeaac88e5dc99},
    {.sector = 7, .key_a = 0xae3d65a3dad4, .key_b = 0x0f1c63013dbb},
    {.sector = 8, .key_a = 0xa73f5dc1d333, .key_b = 0xe35173494a81},
    {.sector = 9, .key_a = 0x69a32f1c2f19, .key_b = 0x6b8bd9860763},
    {.sector = 10, .key_a = 0x9becdf3d9273, .key_b = 0xf8493407799d},
    {.sector = 11, .key_a = 0x08b386463229, .key_b = 0x5efbaecef46b},
    {.sector = 12, .key_a = 0xcd4c61c26e3d, .key_b = 0x31c7610de3b0},
    {.sector = 13, .key_a = 0xa82607b01c0d, .key_b = 0x2910989b6880},
    {.sector = 14, .key_a = 0x0e8f64340ba4, .key_b = 0x4acec1205d75},
    {.sector = 15, .key_a = 0x2aa05ed1856f, .key_b = 0xeaac88e5dc99},
    {.sector = 16, .key_a = 0x6b02733bb6ec, .key_b = 0x7038cd25c408},
    {.sector = 17, .key_a = 0x403d706ba880, .key_b = 0xb39d19a280df},
    {.sector = 18, .key_a = 0xc11f4597efb5, .key_b = 0x70d901648cb9},
    {.sector = 19, .key_a = 0x0db520c78c1c, .key_b = 0x73e5b9d9d3a4},
    {.sector = 20, .key_a = 0x3ebce0925b2f, .key_b = 0x372cc880f216},
    {.sector = 21, .key_a = 0x16a27af45407, .key_b = 0x9868925175ba},
    {.sector = 22, .key_a = 0xaba208516740, .key_b = 0xce26ecb95252},
    {.sector = 23, .key_a = 0xCD64E567ABCD, .key_b = 0x8f79c4fd8a01},
    {.sector = 24, .key_a = 0x764cd061f1e6, .key_b = 0xa74332f74994},
    {.sector = 25, .key_a = 0x1cc219e9fec1, .key_b = 0xb90de525ceb6},
    {.sector = 26, .key_a = 0x2fe3cb83ea43, .key_b = 0xfba88f109b32},
    {.sector = 27, .key_a = 0x07894ffec1d6, .key_b = 0xefcb0e689db3},
    {.sector = 28, .key_a = 0x04c297b91308, .key_b = 0xc8454c154cb5},
    {.sector = 29, .key_a = 0x7a38e3511a38, .key_b = 0xab16584c972a},
    {.sector = 30, .key_a = 0x7545df809202, .key_b = 0xecf751084a80},
    {.sector = 31, .key_a = 0x5125974cd391, .key_b = 0xd3eafb5df46d},
    {.sector = 32, .key_a = 0x7a86aa203788, .key_b = 0xe41242278ca2},
    {.sector = 33, .key_a = 0xafcef64c9913, .key_b = 0x9db96dca4324},
    {.sector = 34, .key_a = 0x04eaa462f70b, .key_b = 0xac17b93e2fae},
    {.sector = 35, .key_a = 0xe734c210f27e, .key_b = 0x29ba8c3e9fda},
    {.sector = 36, .key_a = 0xd5524f591eed, .key_b = 0x5daf42861b4d},
    {.sector = 37, .key_a = 0xe4821a377b75, .key_b = 0xe8709e486465},
    {.sector = 38, .key_a = 0x518dc6eea089, .key_b = 0x97c64ac98ca4},
    {.sector = 39, .key_a = 0xbb52f8cce07f, .key_b = 0x6b6119752c70},
};

bool troika_4k_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);

    if(nfc_worker->dev_data->mf_classic_data.type != MfClassicType4k) {
        return false;
    }

    uint8_t sector = 11;
    uint8_t block = mf_classic_get_sector_trailer_block_num_by_sector(sector);
    FURI_LOG_D("Troika", "Verifying sector %d", sector);
    if(mf_classic_authenticate(tx_rx, block, 0x08b386463229, MfClassicKeyA)) {
        FURI_LOG_D("Troika", "Sector %d verified", sector);
        return true;
    }
    return false;
}

bool troika_4k_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);

    MfClassicReader reader = {};
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    reader.type = mf_classic_get_classic_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak);
    for(size_t i = 0; i < COUNT_OF(troika_4k_keys); i++) {
        mf_classic_reader_add_sector(
            &reader, troika_4k_keys[i].sector, troika_4k_keys[i].key_a, troika_4k_keys[i].key_b);
    }

    return mf_classic_read_card(tx_rx, &reader, &nfc_worker->dev_data->mf_classic_data) == 40;
}

bool troika_4k_parser_parse(NfcDeviceData* dev_data) {
    MfClassicData* data = &dev_data->mf_classic_data;

    // Verify key
    MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 4);
    uint64_t key = nfc_util_bytes2num(sec_tr->key_a, 6);
    if(key != troika_4k_keys[4].key_a) return false;

    // Verify card type
    if(data->type != MfClassicType4k) return false;

    uint8_t* temp_ptr = &data->block[8 * 4 + 1].value[5];
    uint16_t balance = ((temp_ptr[0] << 8) | temp_ptr[1]) / 25;
    temp_ptr = &data->block[8 * 4].value[3];
    uint32_t number = 0;
    for(size_t i = 0; i < 4; i++) {
        number <<= 8;
        number |= temp_ptr[i];
    }
    number >>= 4;

    furi_string_printf(
        dev_data->parsed_data, "\e#Troika\nNum: %lu\nBalance: %u rur.", number, balance);

    return true;
}
