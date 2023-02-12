#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>

#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>

#include <bit_lib.h>

#define TAG "Andalucia"

#include <gui/modules/widget.h>
#include <nfc_worker_i.h>

#include <furi_hal.h>

static const MfClassicAuthContext andalucia_keys[] = {
    {.sector = 0, .key_a = 0xA0A1A2A3A4A5, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 1, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 2, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 3, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 4, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 5, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 6, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 7, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 8, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 9, .key_a = 0x99100225D83B, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 10, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 11, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 12, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 13, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 14, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
    {.sector = 15, .key_a = 0xFFFFFFFFFFFF, .key_b = 0xFFFFFFFFFFFF},
};

bool andalucia_parser_verify(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);
    UNUSED(nfc_worker);

    if(nfc_worker->dev_data->mf_classic_data.type != MfClassicType1k) {
        return false;
    }

    uint8_t sector = 9;
    uint8_t block = mf_classic_get_sector_trailer_block_num_by_sector(sector);
    FURI_LOG_D("Andalucia", "Verifying sector %d", sector);
    if(mf_classic_authenticate(tx_rx, block, 0x99100225D83B, MfClassicKeyA)) {
        FURI_LOG_D("Andalucia", "Sector %d verified", sector);
        return true;
    }
    return false;
}

bool andalucia_parser_read(NfcWorker* nfc_worker, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(nfc_worker);

    MfClassicReader reader = {};
    FuriHalNfcDevData* nfc_data = &nfc_worker->dev_data->nfc_data;
    reader.type = mf_classic_get_classic_type(nfc_data->atqa[0], nfc_data->atqa[1], nfc_data->sak);

    for(size_t i = 0; i < COUNT_OF(andalucia_keys); i++) {
        mf_classic_reader_add_sector(
            &reader, andalucia_keys[i].sector, andalucia_keys[i].key_a, andalucia_keys[i].key_b);
        FURI_LOG_T("Andalucia", "Added sector %d", andalucia_keys[i].sector);
    }

    return mf_classic_read_card(tx_rx, &reader, &nfc_worker->dev_data->mf_classic_data) == 16;
}

bool andalucia_parser_parse(NfcDeviceData* dev_data) {
    MfClassicData* data = &dev_data->mf_classic_data;

    // Verify key
    MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 9);
    uint64_t key = nfc_util_bytes2num(sec_tr->key_a, 6);
    if(key != andalucia_keys[9].key_a) return false;

    // Verify card type
    if(data->type != MfClassicType1k) return false;

    // Point to block 1 of sector 9, value 0
    uint8_t* temp_ptr = &data->block[9 * 4 + 1].value[0];
    // Read first 4 bytes of block 0 of sector 4 from last to first and convert them to uint32_t
    // 38 18 00 00 becomes 00 00 18 38, and equals to 6200 decimal
    uint32_t balance =
        ((temp_ptr[3] << 24) | (temp_ptr[2] << 16) | (temp_ptr[1] << 8) | temp_ptr[0]);

    double eur = (balance / 2) / 100.0f;

    furi_string_printf(
        dev_data->parsed_data, "\e#Andalucia\nBalance: %.2f euros.", eur);

    return true;
}
