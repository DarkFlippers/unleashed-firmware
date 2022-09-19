#include "nfc_supported_card.h"
#include "plantain_parser.h" // For luhn and string_push_uint64

#include <gui/modules/widget.h>
#include <nfc_worker_i.h>

#include "furi_hal.h"

static const MfClassicAuthContext plantain_keys_4k[] = {
    {.sector = 0, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 1, .key_a = 0xffffffffffff, .key_b = 0xffffffffffff},
    {.sector = 2, .key_a = 0xffffffffffff, .key_b = 0xffffffffffff},
    {.sector = 3, .key_a = 0xffffffffffff, .key_b = 0xffffffffffff},
    {.sector = 4, .key_a = 0xe56ac127dd45, .key_b = 0x19fc84a3784b},
    {.sector = 5, .key_a = 0x77dabc9825e1, .key_b = 0x9764fec3154a},
    {.sector = 6, .key_a = 0xffffffffffff, .key_b = 0xffffffffffff},
    {.sector = 7, .key_a = 0xffffffffffff, .key_b = 0xffffffffffff},
    {.sector = 8, .key_a = 0x26973ea74321, .key_b = 0xd27058c6e2c7},
    {.sector = 9, .key_a = 0xeb0a8ff88ade, .key_b = 0x578a9ada41e3},
    {.sector = 10, .key_a = 0xea0fd73cb149, .key_b = 0x29c35fa068fb},
    {.sector = 11, .key_a = 0xc76bf71a2509, .key_b = 0x9ba241db3f56},
    {.sector = 12, .key_a = 0xacffffffffff, .key_b = 0x71f3a315ad26},
    {.sector = 13, .key_a = 0xffffffffffff, .key_b = 0xffffffffffff},
    {.sector = 14, .key_a = 0xffffffffffff, .key_b = 0xffffffffffff},
    {.sector = 15, .key_a = 0xffffffffffff, .key_b = 0xffffffffffff},
    {.sector = 16, .key_a = 0x72f96bdd3714, .key_b = 0x462225cd34cf},
    {.sector = 17, .key_a = 0x044ce1872bc3, .key_b = 0x8c90c70cff4a},
    {.sector = 18, .key_a = 0xbc2d1791dec1, .key_b = 0xca96a487de0b},
    {.sector = 19, .key_a = 0x8791b2ccb5c4, .key_b = 0xc956c3b80da3},
    {.sector = 20, .key_a = 0x8e26e45e7d65, .key_b = 0x8e65b3af7d22},
    {.sector = 21, .key_a = 0x0f318130ed18, .key_b = 0x0c420a20e056},
    {.sector = 22, .key_a = 0x045ceca15535, .key_b = 0x31bec3d9e510},
    {.sector = 23, .key_a = 0x9d993c5d4ef4, .key_b = 0x86120e488abf},
    {.sector = 24, .key_a = 0xc65d4eaa645b, .key_b = 0xb69d40d1a439},
    {.sector = 25, .key_a = 0x3a8a139c20b4, .key_b = 0x8818a9c5d406},
    {.sector = 26, .key_a = 0xbaff3053b496, .key_b = 0x4b7cb25354d3},
    {.sector = 27, .key_a = 0x7413b599c4ea, .key_b = 0xb0a2AAF3A1BA},
    {.sector = 28, .key_a = 0x0ce7cd2cc72b, .key_b = 0xfa1fbb3f0f1f},
    {.sector = 29, .key_a = 0x0be5fac8b06a, .key_b = 0x6f95887a4fd3},
    {.sector = 30, .key_a = 0x0eb23cc8110b, .key_b = 0x04dc35277635},
    {.sector = 31, .key_a = 0xbc4580b7f20b, .key_b = 0xd0a4131fb290},
    {.sector = 32, .key_a = 0x7a396f0d633d, .key_b = 0xad2bdc097023},
    {.sector = 33, .key_a = 0xa3faa6daff67, .key_b = 0x7600e889adf9},
    {.sector = 34, .key_a = 0xfd8705e721b0, .key_b = 0x296fc317a513},
    {.sector = 35, .key_a = 0x22052b480d11, .key_b = 0xe19504c39461},
    {.sector = 36, .key_a = 0xa7141147d430, .key_b = 0xff16014fefc7},
    {.sector = 37, .key_a = 0x8a8d88151a00, .key_b = 0x038b5f9b5a2a},
    {.sector = 38, .key_a = 0xb27addfb64b0, .key_b = 0x152fd0c420a7},
    {.sector = 39, .key_a = 0x7259fa0197c6, .key_b = 0x5583698df085},
};

bool plantain_4k_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);
    UNUSED(nfc_worker);

    if(nfc_worker->dev_data->mf_classic_data.type != MfClassicType4k) {
        return false;
    }

    uint8_t sector = 8;
    uint8_t block = mf_classic_get_sector_trailer_block_num_by_sector(sector);
    FURI_LOG_D("Plant4K", "Verifying sector %d", sector);
    if(mf_classic_authenticate(tx_rx, block, 0x26973ea74321, MfClassicKeyA)) {
        FURI_LOG_D("Plant4K", "Sector %d verified", sector);
        return true;
    }
    return false;
}

bool plantain_4k_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);

    MfClassicReader reader = {};
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    reader.type = mf_classic_get_classic_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak);
    for(size_t i = 0; i < COUNT_OF(plantain_keys_4k); i++) {
        mf_classic_reader_add_sector(
            &reader,
            plantain_keys_4k[i].sector,
            plantain_keys_4k[i].key_a,
            plantain_keys_4k[i].key_b);
        FURI_LOG_T("plant4k", "Added sector %d", plantain_keys_4k[i].sector);
    }
    for(int i = 0; i < 5; i++) {
        if(mf_classic_read_card(tx_rx, &reader, &nfc_worker->dev_data->mf_classic_data) == 40) {
            return true;
        }
    }
    return false;
}

bool plantain_4k_parser_parse(NfcDeviceData* dev_data) {
    MfClassicData* data = &dev_data->mf_classic_data;

    // Verify key
    MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 8);
    uint64_t key = nfc_util_bytes2num(sec_tr->key_a, 6);
    if(key != plantain_keys_4k[8].key_a) return false;

    // Point to block 0 of sector 4, value 0
    uint8_t* temp_ptr = &data->block[4 * 4].value[0];
    // Read first 4 bytes of block 0 of sector 4 from last to first and convert them to uint32_t
    // 38 18 00 00 becomes 00 00 18 38, and equals to 6200 decimal
    uint32_t balance =
        ((temp_ptr[3] << 24) | (temp_ptr[2] << 16) | (temp_ptr[1] << 8) | temp_ptr[0]) / 100;
    // Read card number
    // Point to block 0 of sector 0, value 0
    temp_ptr = &data->block[0 * 4].value[0];
    // Read first 7 bytes of block 0 of sector 0 from last to first and convert them to uint64_t
    // 80 5C 23 8A 16 31 04 becomes 04 31 16 8A 23 5C 80, and equals to 36130104729284868 decimal
    uint8_t card_number_arr[7];
    for(size_t i = 0; i < 7; i++) {
        card_number_arr[i] = temp_ptr[6 - i];
    }
    // Copy card number to uint64_t
    uint64_t card_number = 0;
    for(size_t i = 0; i < 7; i++) {
        card_number = (card_number << 8) | card_number_arr[i];
    }
    // Convert card number to string
    string_t card_number_str;
    string_init(card_number_str);
    // Should look like "361301047292848684"
    // %llu doesn't work for some reason in sprintf, so we use string_push_uint64 instead
    string_push_uint64(card_number, card_number_str);
    // Add suffix with luhn checksum (1 digit) to the card number string
    string_t card_number_suffix;
    string_init(card_number_suffix);

    // The number to calculate the checksum on doesn't fit into uint64_t, idk
    //uint8_t luhn_checksum = plantain_calculate_luhn(card_number);

    // // Convert luhn checksum to string
    // string_t luhn_checksum_str;
    // string_init(luhn_checksum_str);
    // string_push_uint64(luhn_checksum, luhn_checksum_str);

    string_cat_printf(card_number_suffix, "-");
    // FURI_LOG_D("plant4k", "Card checksum: %d", luhn_checksum);
    string_cat_printf(card_number_str, string_get_cstr(card_number_suffix));
    // Free all not needed strings
    string_clear(card_number_suffix);
    // string_clear(luhn_checksum_str);

    string_printf(
        dev_data->parsed_data,
        "\e#Plantain\nN:%s\nBalance:%d\n",
        string_get_cstr(card_number_str),
        balance);
    string_clear(card_number_str);

    return true;
}
