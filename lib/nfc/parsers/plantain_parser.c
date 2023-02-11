#include "nfc_supported_card.h"

#include <gui/modules/widget.h>
#include <nfc_worker_i.h>

#include <furi_hal.h>

static const MfClassicAuthContext plantain_keys[] = {
    {.sector = 0, .key_a = 0xffffffffffff, .key_b = 0xffffffffffff},
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
};

bool plantain_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);
    UNUSED(nfc_worker);
    if(nfc_worker->dev_data->mf_classic_data.type != MfClassicType1k) {
        return false;
    }

    uint8_t sector = 8;
    uint8_t block = mf_classic_get_sector_trailer_block_num_by_sector(sector);
    FURI_LOG_D("Plant", "Verifying sector %d", sector);
    if(mf_classic_authenticate(tx_rx, block, 0x26973ea74321, MfClassicKeyA)) {
        FURI_LOG_D("Plant", "Sector %d verified", sector);
        return true;
    }
    return false;
}

bool plantain_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);

    MfClassicReader reader = {};
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    reader.type = mf_classic_get_classic_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak);
    for(size_t i = 0; i < COUNT_OF(plantain_keys); i++) {
        mf_classic_reader_add_sector(
            &reader, plantain_keys[i].sector, plantain_keys[i].key_a, plantain_keys[i].key_b);
    }

    return mf_classic_read_card(tx_rx, &reader, &nfc_worker->dev_data->mf_classic_data) == 16;
}

uint8_t plantain_calculate_luhn(uint64_t number) {
    // No.
    UNUSED(number);
    return 0;
}

bool plantain_parser_parse(NfcDeviceData* dev_data) {
    MfClassicData* data = &dev_data->mf_classic_data;

    // Verify key
    MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 8);
    uint64_t key = nfc_util_bytes2num(sec_tr->key_a, 6);
    if(key != plantain_keys[8].key_a) return false;

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
    // 04 31 16 8A 23 5C 80 becomes 80 5C 23 8A 16 31 04, and equals to 36130104729284868 decimal
    uint8_t card_number_arr[7];
    for(size_t i = 0; i < 7; i++) {
        card_number_arr[i] = temp_ptr[6 - i];
    }
    // Copy card number to uint64_t
    uint64_t card_number = 0;
    for(size_t i = 0; i < 7; i++) {
        card_number = (card_number << 8) | card_number_arr[i];
    }

    furi_string_printf(
        dev_data->parsed_data, "\e#Plantain\nN:%llu-\nBalance:%ld\n", card_number, balance);

    return true;
}
