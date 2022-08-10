#include "mifare_classic.h"
#include "nfca.h"
#include "nfc_util.h"
#include <furi_hal_rtc.h>

// Algorithm from https://github.com/RfidResearchGroup/proxmark3.git

#define TAG "MfClassic"

#define MF_CLASSIC_AUTH_KEY_A_CMD (0x60U)
#define MF_CLASSIC_AUTH_KEY_B_CMD (0x61U)
#define MF_CLASSIC_READ_SECT_CMD (0x30)

typedef enum {
    MfClassicActionDataRead,
    MfClassicActionDataWrite,
    MfClassicActionDataInc,
    MfClassicActionDataDec,

    MfClassicActionKeyARead,
    MfClassicActionKeyAWrite,
    MfClassicActionKeyBRead,
    MfClassicActionKeyBWrite,
    MfClassicActionACRead,
    MfClassicActionACWrite,
} MfClassicAction;

const char* mf_classic_get_type_str(MfClassicType type) {
    if(type == MfClassicType1k) {
        return "MIFARE Classic 1K";
    } else if(type == MfClassicType4k) {
        return "MIFARE Classic 4K";
    } else {
        return "Unknown";
    }
}

static uint8_t mf_classic_get_first_block_num_of_sector(uint8_t sector) {
    furi_assert(sector < 40);
    if(sector < 32) {
        return sector * 4;
    } else {
        return 32 * 4 + (sector - 32) * 16;
    }
}

uint8_t mf_classic_get_sector_trailer_block_num_by_sector(uint8_t sector) {
    furi_assert(sector < 40);
    if(sector < 32) {
        return sector * 4 + 3;
    } else {
        return 32 * 4 + (sector - 32) * 16 + 15;
    }
}

uint8_t mf_classic_get_sector_by_block(uint8_t block) {
    if(block < 128) {
        return (block | 0x03) / 4;
    } else {
        return 32 + ((block | 0xf) - 32 * 4) / 16;
    }
}

static uint8_t mf_classic_get_blocks_num_in_sector(uint8_t sector) {
    furi_assert(sector < 40);
    return sector < 32 ? 4 : 16;
}

uint8_t mf_classic_get_sector_trailer_num_by_block(uint8_t block) {
    if(block < 128) {
        return block | 0x03;
    } else {
        return block | 0x0f;
    }
}

bool mf_classic_is_sector_trailer(uint8_t block) {
    return block == mf_classic_get_sector_trailer_num_by_block(block);
}

MfClassicSectorTrailer*
    mf_classic_get_sector_trailer_by_sector(MfClassicData* data, uint8_t sector) {
    furi_assert(data);
    uint8_t sec_tr_block_num = mf_classic_get_sector_trailer_block_num_by_sector(sector);
    return (MfClassicSectorTrailer*)data->block[sec_tr_block_num].value;
}

uint8_t mf_classic_get_total_sectors_num(MfClassicType type) {
    if(type == MfClassicType1k) {
        return MF_CLASSIC_1K_TOTAL_SECTORS_NUM;
    } else if(type == MfClassicType4k) {
        return MF_CLASSIC_4K_TOTAL_SECTORS_NUM;
    } else {
        return 0;
    }
}

static uint16_t mf_classic_get_total_block_num(MfClassicType type) {
    if(type == MfClassicType1k) {
        return 64;
    } else if(type == MfClassicType4k) {
        return 256;
    } else {
        return 0;
    }
}

bool mf_classic_is_block_read(MfClassicData* data, uint8_t block_num) {
    furi_assert(data);

    return (FURI_BIT(data->block_read_mask[block_num / 32], block_num % 32) == 1);
}

void mf_classic_set_block_read(MfClassicData* data, uint8_t block_num, MfClassicBlock* block_data) {
    furi_assert(data);

    if(mf_classic_is_sector_trailer(block_num)) {
        memcpy(&data->block[block_num].value[6], &block_data->value[6], 4);
    } else {
        memcpy(data->block[block_num].value, block_data->value, MF_CLASSIC_BLOCK_SIZE);
    }
    FURI_BIT_SET(data->block_read_mask[block_num / 32], block_num % 32);
}

bool mf_classic_is_key_found(MfClassicData* data, uint8_t sector_num, MfClassicKey key_type) {
    furi_assert(data);

    bool key_found = false;
    if(key_type == MfClassicKeyA) {
        key_found = (FURI_BIT(data->key_a_mask, sector_num) == 1);
    } else if(key_type == MfClassicKeyB) {
        key_found = (FURI_BIT(data->key_b_mask, sector_num) == 1);
    }

    return key_found;
}

void mf_classic_set_key_found(
    MfClassicData* data,
    uint8_t sector_num,
    MfClassicKey key_type,
    uint64_t key) {
    furi_assert(data);

    uint8_t key_arr[6] = {};
    MfClassicSectorTrailer* sec_trailer =
        mf_classic_get_sector_trailer_by_sector(data, sector_num);
    nfc_util_num2bytes(key, 6, key_arr);
    if(key_type == MfClassicKeyA) {
        memcpy(sec_trailer->key_a, key_arr, sizeof(sec_trailer->key_a));
        FURI_BIT_SET(data->key_a_mask, sector_num);
    } else if(key_type == MfClassicKeyB) {
        memcpy(sec_trailer->key_b, key_arr, sizeof(sec_trailer->key_b));
        FURI_BIT_SET(data->key_b_mask, sector_num);
    }
}

bool mf_classic_is_sector_read(MfClassicData* data, uint8_t sector_num) {
    furi_assert(data);

    bool sector_read = false;
    do {
        if(!mf_classic_is_key_found(data, sector_num, MfClassicKeyA)) break;
        if(!mf_classic_is_key_found(data, sector_num, MfClassicKeyB)) break;
        uint8_t start_block = mf_classic_get_first_block_num_of_sector(sector_num);
        uint8_t total_blocks = mf_classic_get_blocks_num_in_sector(sector_num);
        uint8_t block_read = true;
        for(size_t i = start_block; i < start_block + total_blocks; i++) {
            block_read = mf_classic_is_block_read(data, i);
            if(!block_read) break;
        }
        sector_read = block_read;
    } while(false);

    return sector_read;
}

void mf_classic_get_read_sectors_and_keys(
    MfClassicData* data,
    uint8_t* sectors_read,
    uint8_t* keys_found) {
    furi_assert(data);
    *sectors_read = 0;
    *keys_found = 0;
    uint8_t sectors_total = mf_classic_get_total_sectors_num(data->type);
    for(size_t i = 0; i < sectors_total; i++) {
        if(mf_classic_is_key_found(data, i, MfClassicKeyA)) {
            *keys_found += 1;
        }
        if(mf_classic_is_key_found(data, i, MfClassicKeyB)) {
            *keys_found += 1;
        }
        uint8_t first_block = mf_classic_get_first_block_num_of_sector(i);
        uint8_t total_blocks_in_sec = mf_classic_get_blocks_num_in_sector(i);
        bool blocks_read = true;
        for(size_t i = first_block; i < first_block + total_blocks_in_sec; i++) {
            blocks_read = mf_classic_is_block_read(data, i);
            if(!blocks_read) break;
        }
        if(blocks_read) {
            *sectors_read += 1;
        }
    }
}

static bool mf_classic_is_allowed_access_sector_trailer(
    MfClassicEmulator* emulator,
    uint8_t block_num,
    MfClassicKey key,
    MfClassicAction action) {
    uint8_t* sector_trailer = emulator->data.block[block_num].value;
    uint8_t AC = ((sector_trailer[7] >> 5) & 0x04) | ((sector_trailer[8] >> 2) & 0x02) |
                 ((sector_trailer[8] >> 7) & 0x01);
    switch(action) {
    case MfClassicActionKeyARead: {
        return false;
    }
    case MfClassicActionKeyAWrite: {
        return (
            (key == MfClassicKeyA && (AC == 0x00 || AC == 0x01)) ||
            (key == MfClassicKeyB && (AC == 0x04 || AC == 0x03)));
    }
    case MfClassicActionKeyBRead: {
        return (key == MfClassicKeyA && (AC == 0x00 || AC == 0x02 || AC == 0x01));
    }
    case MfClassicActionKeyBWrite: {
        return (
            (key == MfClassicKeyA && (AC == 0x00 || AC == 0x01)) ||
            (key == MfClassicKeyB && (AC == 0x04 || AC == 0x03)));
    }
    case MfClassicActionACRead: {
        return (
            (key == MfClassicKeyA) ||
            (key == MfClassicKeyB && !(AC == 0x00 || AC == 0x02 || AC == 0x01)));
    }
    case MfClassicActionACWrite: {
        return (
            (key == MfClassicKeyA && (AC == 0x01)) ||
            (key == MfClassicKeyB && (AC == 0x03 || AC == 0x05)));
    }
    default:
        return false;
    }
    return true;
}

static bool mf_classic_is_allowed_access_data_block(
    MfClassicEmulator* emulator,
    uint8_t block_num,
    MfClassicKey key,
    MfClassicAction action) {
    uint8_t* sector_trailer =
        emulator->data.block[mf_classic_get_sector_trailer_num_by_block(block_num)].value;

    uint8_t sector_block;
    if(block_num <= 128) {
        sector_block = block_num & 0x03;
    } else {
        sector_block = (block_num & 0x0f) / 5;
    }

    uint8_t AC;
    switch(sector_block) {
    case 0x00: {
        AC = ((sector_trailer[7] >> 2) & 0x04) | ((sector_trailer[8] << 1) & 0x02) |
             ((sector_trailer[8] >> 4) & 0x01);
        break;
    }
    case 0x01: {
        AC = ((sector_trailer[7] >> 3) & 0x04) | ((sector_trailer[8] >> 0) & 0x02) |
             ((sector_trailer[8] >> 5) & 0x01);
        break;
    }
    case 0x02: {
        AC = ((sector_trailer[7] >> 4) & 0x04) | ((sector_trailer[8] >> 1) & 0x02) |
             ((sector_trailer[8] >> 6) & 0x01);
        break;
    }
    default:
        return false;
    }

    switch(action) {
    case MfClassicActionDataRead: {
        return (
            (key == MfClassicKeyA && !(AC == 0x03 || AC == 0x05 || AC == 0x07)) ||
            (key == MfClassicKeyB && !(AC == 0x07)));
    }
    case MfClassicActionDataWrite: {
        return (
            (key == MfClassicKeyA && (AC == 0x00)) ||
            (key == MfClassicKeyB && (AC == 0x00 || AC == 0x04 || AC == 0x06 || AC == 0x03)));
    }
    case MfClassicActionDataInc: {
        return (
            (key == MfClassicKeyA && (AC == 0x00)) ||
            (key == MfClassicKeyB && (AC == 0x00 || AC == 0x06)));
    }
    case MfClassicActionDataDec: {
        return (
            (key == MfClassicKeyA && (AC == 0x00 || AC == 0x06 || AC == 0x01)) ||
            (key == MfClassicKeyB && (AC == 0x00 || AC == 0x06 || AC == 0x01)));
    }
    default:
        return false;
    }

    return false;
}

static bool mf_classic_is_allowed_access(
    MfClassicEmulator* emulator,
    uint8_t block_num,
    MfClassicKey key,
    MfClassicAction action) {
    if(mf_classic_is_sector_trailer(block_num)) {
        return mf_classic_is_allowed_access_sector_trailer(emulator, block_num, key, action);
    } else {
        return mf_classic_is_allowed_access_data_block(emulator, block_num, key, action);
    }
}

bool mf_classic_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK) {
    UNUSED(ATQA1);
    if((ATQA0 == 0x44 || ATQA0 == 0x04) && (SAK == 0x08 || SAK == 0x88 || SAK == 0x09)) {
        return true;
    } else if((ATQA0 == 0x01) && (ATQA1 == 0x0F) && (SAK == 0x01)) {
        //skylanders support
        return true;
    } else if((ATQA0 == 0x42 || ATQA0 == 0x02) && (SAK == 0x18)) {
        return true;
    } else {
        return false;
    }
}

MfClassicType mf_classic_get_classic_type(int8_t ATQA0, uint8_t ATQA1, uint8_t SAK) {
    UNUSED(ATQA1);
    if((ATQA0 == 0x44 || ATQA0 == 0x04) && (SAK == 0x08 || SAK == 0x88 || SAK == 0x09)) {
        return MfClassicType1k;
    } else if((ATQA0 == 0x01) && (ATQA1 == 0x0F) && (SAK == 0x01)) {
        //skylanders support
        return MfClassicType1k;
    } else if((ATQA0 == 0x42 || ATQA0 == 0x02) && (SAK == 0x18)) {
        return MfClassicType4k;
    }
    return MfClassicType1k;
}

void mf_classic_reader_add_sector(
    MfClassicReader* reader,
    uint8_t sector,
    uint64_t key_a,
    uint64_t key_b) {
    furi_assert(reader);
    furi_assert(sector < MF_CLASSIC_SECTORS_MAX);
    furi_assert((key_a != MF_CLASSIC_NO_KEY) || (key_b != MF_CLASSIC_NO_KEY));

    if(reader->sectors_to_read < MF_CLASSIC_SECTORS_MAX) {
        reader->sector_reader[reader->sectors_to_read].key_a = key_a;
        reader->sector_reader[reader->sectors_to_read].key_b = key_b;
        reader->sector_reader[reader->sectors_to_read].sector_num = sector;
        reader->sectors_to_read++;
    }
}

void mf_classic_auth_init_context(MfClassicAuthContext* auth_ctx, uint8_t sector) {
    furi_assert(auth_ctx);
    auth_ctx->sector = sector;
    auth_ctx->key_a = MF_CLASSIC_NO_KEY;
    auth_ctx->key_b = MF_CLASSIC_NO_KEY;
}

static bool mf_classic_auth(
    FuriHalNfcTxRxContext* tx_rx,
    uint32_t block,
    uint64_t key,
    MfClassicKey key_type,
    Crypto1* crypto) {
    bool auth_success = false;
    uint32_t cuid = 0;
    memset(tx_rx->tx_data, 0, sizeof(tx_rx->tx_data));
    memset(tx_rx->tx_parity, 0, sizeof(tx_rx->tx_parity));
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;

    do {
        if(!furi_hal_nfc_activate_nfca(200, &cuid)) break;
        if(key_type == MfClassicKeyA) {
            tx_rx->tx_data[0] = MF_CLASSIC_AUTH_KEY_A_CMD;
        } else {
            tx_rx->tx_data[0] = MF_CLASSIC_AUTH_KEY_B_CMD;
        }
        tx_rx->tx_data[1] = block;
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeRxNoCrc;
        tx_rx->tx_bits = 2 * 8;
        if(!furi_hal_nfc_tx_rx(tx_rx, 6)) break;

        uint32_t nt = (uint32_t)nfc_util_bytes2num(tx_rx->rx_data, 4);
        crypto1_init(crypto, key);
        crypto1_word(crypto, nt ^ cuid, 0);
        uint8_t nr[4] = {};
        nfc_util_num2bytes(prng_successor(DWT->CYCCNT, 32), 4, nr);
        for(uint8_t i = 0; i < 4; i++) {
            tx_rx->tx_data[i] = crypto1_byte(crypto, nr[i], 0) ^ nr[i];
            tx_rx->tx_parity[0] |=
                (((crypto1_filter(crypto->odd) ^ nfc_util_odd_parity8(nr[i])) & 0x01) << (7 - i));
        }
        nt = prng_successor(nt, 32);
        for(uint8_t i = 4; i < 8; i++) {
            nt = prng_successor(nt, 8);
            tx_rx->tx_data[i] = crypto1_byte(crypto, 0x00, 0) ^ (nt & 0xff);
            tx_rx->tx_parity[0] |=
                (((crypto1_filter(crypto->odd) ^ nfc_util_odd_parity8(nt & 0xff)) & 0x01)
                 << (7 - i));
        }
        tx_rx->tx_rx_type = FuriHalNfcTxRxTypeRaw;
        tx_rx->tx_bits = 8 * 8;
        if(!furi_hal_nfc_tx_rx(tx_rx, 6)) break;
        if(tx_rx->rx_bits == 32) {
            crypto1_word(crypto, 0, 0);
            auth_success = true;
        }
    } while(false);

    return auth_success;
}

bool mf_classic_authenticate(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t block_num,
    uint64_t key,
    MfClassicKey key_type) {
    furi_assert(tx_rx);

    Crypto1 crypto = {};
    bool key_found = mf_classic_auth(tx_rx, block_num, key, key_type, &crypto);
    furi_hal_nfc_sleep();
    return key_found;
}

bool mf_classic_auth_attempt(
    FuriHalNfcTxRxContext* tx_rx,
    MfClassicAuthContext* auth_ctx,
    uint64_t key) {
    furi_assert(tx_rx);
    furi_assert(auth_ctx);
    bool found_key = false;
    bool need_halt = (auth_ctx->key_a == MF_CLASSIC_NO_KEY) &&
                     (auth_ctx->key_b == MF_CLASSIC_NO_KEY);

    Crypto1 crypto;
    if(auth_ctx->key_a == MF_CLASSIC_NO_KEY) {
        // Try AUTH with key A
        if(mf_classic_auth(
               tx_rx,
               mf_classic_get_first_block_num_of_sector(auth_ctx->sector),
               key,
               MfClassicKeyA,
               &crypto)) {
            auth_ctx->key_a = key;
            found_key = true;
        }
    }

    if(need_halt) {
        furi_hal_nfc_sleep();
    }

    if(auth_ctx->key_b == MF_CLASSIC_NO_KEY) {
        // Try AUTH with key B
        if(mf_classic_auth(
               tx_rx,
               mf_classic_get_first_block_num_of_sector(auth_ctx->sector),
               key,
               MfClassicKeyB,
               &crypto)) {
            auth_ctx->key_b = key;
            found_key = true;
        }
    }

    return found_key;
}

bool mf_classic_read_block(
    FuriHalNfcTxRxContext* tx_rx,
    Crypto1* crypto,
    uint8_t block_num,
    MfClassicBlock* block) {
    furi_assert(tx_rx);
    furi_assert(crypto);
    furi_assert(block);

    bool read_block_success = false;
    uint8_t plain_cmd[4] = {MF_CLASSIC_READ_SECT_CMD, block_num, 0x00, 0x00};
    nfca_append_crc16(plain_cmd, 2);
    memset(tx_rx->tx_data, 0, sizeof(tx_rx->tx_data));
    memset(tx_rx->tx_parity, 0, sizeof(tx_rx->tx_parity));

    for(uint8_t i = 0; i < 4; i++) {
        tx_rx->tx_data[i] = crypto1_byte(crypto, 0x00, 0) ^ plain_cmd[i];
        tx_rx->tx_parity[0] |=
            ((crypto1_filter(crypto->odd) ^ nfc_util_odd_parity8(plain_cmd[i])) & 0x01) << (7 - i);
    }
    tx_rx->tx_bits = 4 * 9;
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeRaw;

    if(furi_hal_nfc_tx_rx(tx_rx, 50)) {
        if(tx_rx->rx_bits == 8 * (MF_CLASSIC_BLOCK_SIZE + 2)) {
            uint8_t block_received[MF_CLASSIC_BLOCK_SIZE + 2];
            for(uint8_t i = 0; i < MF_CLASSIC_BLOCK_SIZE + 2; i++) {
                block_received[i] = crypto1_byte(crypto, 0, 0) ^ tx_rx->rx_data[i];
            }
            uint16_t crc_calc = nfca_get_crc16(block_received, MF_CLASSIC_BLOCK_SIZE);
            uint16_t crc_received = (block_received[MF_CLASSIC_BLOCK_SIZE + 1] << 8) |
                                    block_received[MF_CLASSIC_BLOCK_SIZE];
            if(crc_received != crc_calc) {
                FURI_LOG_E(
                    TAG,
                    "Incorrect CRC while reading block %d. Expected %04X, Received %04X",
                    block_num,
                    crc_received,
                    crc_calc);
            } else {
                memcpy(block->value, block_received, MF_CLASSIC_BLOCK_SIZE);
                read_block_success = true;
            }
        }
    }
    return read_block_success;
}

void mf_classic_read_sector(FuriHalNfcTxRxContext* tx_rx, MfClassicData* data, uint8_t sec_num) {
    furi_assert(tx_rx);
    furi_assert(data);

    furi_hal_nfc_sleep();
    bool key_a_found = mf_classic_is_key_found(data, sec_num, MfClassicKeyA);
    bool key_b_found = mf_classic_is_key_found(data, sec_num, MfClassicKeyB);
    uint8_t start_block = mf_classic_get_first_block_num_of_sector(sec_num);
    uint8_t total_blocks = mf_classic_get_blocks_num_in_sector(sec_num);
    MfClassicBlock block_tmp = {};
    uint64_t key = 0;
    MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, sec_num);
    Crypto1 crypto = {};

    uint8_t blocks_read = 0;
    do {
        if(!key_a_found) break;
        FURI_LOG_D(TAG, "Try to read blocks with key A");
        key = nfc_util_bytes2num(sec_tr->key_a, sizeof(sec_tr->key_a));
        if(!mf_classic_auth(tx_rx, start_block, key, MfClassicKeyA, &crypto)) break;
        for(size_t i = start_block; i < start_block + total_blocks; i++) {
            if(!mf_classic_is_block_read(data, i)) {
                if(mf_classic_read_block(tx_rx, &crypto, i, &block_tmp)) {
                    mf_classic_set_block_read(data, i, &block_tmp);
                    blocks_read++;
                }
            } else {
                blocks_read++;
            }
        }
        FURI_LOG_D(TAG, "Read %d blocks out of %d", blocks_read, total_blocks);
    } while(false);
    do {
        if(blocks_read == total_blocks) break;
        if(!key_b_found) break;
        FURI_LOG_D(TAG, "Try to read blocks with key B");
        key = nfc_util_bytes2num(sec_tr->key_b, sizeof(sec_tr->key_b));
        furi_hal_nfc_sleep();
        if(!mf_classic_auth(tx_rx, start_block, key, MfClassicKeyB, &crypto)) break;
        for(size_t i = start_block; i < start_block + total_blocks; i++) {
            if(!mf_classic_is_block_read(data, i)) {
                if(mf_classic_read_block(tx_rx, &crypto, i, &block_tmp)) {
                    mf_classic_set_block_read(data, i, &block_tmp);
                    blocks_read++;
                }
            } else {
                blocks_read++;
            }
        }
        FURI_LOG_D(TAG, "Read %d blocks out of %d", blocks_read, total_blocks);
    } while(false);
}

static bool mf_classic_read_sector_with_reader(
    FuriHalNfcTxRxContext* tx_rx,
    Crypto1* crypto,
    MfClassicSectorReader* sector_reader,
    MfClassicSector* sector) {
    furi_assert(tx_rx);
    furi_assert(sector_reader);
    furi_assert(sector);

    uint64_t key;
    MfClassicKey key_type;
    uint8_t first_block;
    bool sector_read = false;

    furi_hal_nfc_sleep();
    do {
        // Activate card
        first_block = mf_classic_get_first_block_num_of_sector(sector_reader->sector_num);
        if(sector_reader->key_a != MF_CLASSIC_NO_KEY) {
            key = sector_reader->key_a;
            key_type = MfClassicKeyA;
        } else if(sector_reader->key_b != MF_CLASSIC_NO_KEY) {
            key = sector_reader->key_b;
            key_type = MfClassicKeyB;
        } else {
            break;
        }

        // Auth to first block in sector
        if(!mf_classic_auth(tx_rx, first_block, key, key_type, crypto)) break;
        sector->total_blocks = mf_classic_get_blocks_num_in_sector(sector_reader->sector_num);

        // Read blocks
        for(uint8_t i = 0; i < sector->total_blocks; i++) {
            mf_classic_read_block(tx_rx, crypto, first_block + i, &sector->block[i]);
        }
        // Save sector keys in last block
        if(sector_reader->key_a != MF_CLASSIC_NO_KEY) {
            nfc_util_num2bytes(
                sector_reader->key_a, 6, &sector->block[sector->total_blocks - 1].value[0]);
        }
        if(sector_reader->key_b != MF_CLASSIC_NO_KEY) {
            nfc_util_num2bytes(
                sector_reader->key_b, 6, &sector->block[sector->total_blocks - 1].value[10]);
        }

        sector_read = true;
    } while(false);

    return sector_read;
}

uint8_t mf_classic_read_card(
    FuriHalNfcTxRxContext* tx_rx,
    MfClassicReader* reader,
    MfClassicData* data) {
    furi_assert(tx_rx);
    furi_assert(reader);
    furi_assert(data);

    uint8_t sectors_read = 0;
    data->type = reader->type;
    data->key_a_mask = 0;
    data->key_b_mask = 0;
    MfClassicSector temp_sector = {};
    for(uint8_t i = 0; i < reader->sectors_to_read; i++) {
        if(mf_classic_read_sector_with_reader(
               tx_rx, &reader->crypto, &reader->sector_reader[i], &temp_sector)) {
            uint8_t first_block =
                mf_classic_get_first_block_num_of_sector(reader->sector_reader[i].sector_num);
            for(uint8_t j = 0; j < temp_sector.total_blocks; j++) {
                mf_classic_set_block_read(data, first_block + j, &temp_sector.block[j]);
            }
            if(reader->sector_reader[i].key_a != MF_CLASSIC_NO_KEY) {
                mf_classic_set_key_found(
                    data,
                    reader->sector_reader[i].sector_num,
                    MfClassicKeyA,
                    reader->sector_reader[i].key_a);
            }
            if(reader->sector_reader[i].key_b != MF_CLASSIC_NO_KEY) {
                mf_classic_set_key_found(
                    data,
                    reader->sector_reader[i].sector_num,
                    MfClassicKeyB,
                    reader->sector_reader[i].key_b);
            }
            sectors_read++;
        }
    }

    return sectors_read;
}

uint8_t mf_classic_update_card(FuriHalNfcTxRxContext* tx_rx, MfClassicData* data) {
    furi_assert(tx_rx);
    furi_assert(data);

    uint8_t sectors_read = 0;
    Crypto1 crypto = {};
    uint8_t total_sectors = mf_classic_get_total_sectors_num(data->type);
    uint64_t key_a = 0;
    uint64_t key_b = 0;
    MfClassicSectorReader sec_reader = {};
    MfClassicSector temp_sector = {};

    for(size_t i = 0; i < total_sectors; i++) {
        MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, i);
        // Load key A
        if(mf_classic_is_key_found(data, i, MfClassicKeyA)) {
            sec_reader.key_a = nfc_util_bytes2num(sec_tr->key_a, 6);
        } else {
            sec_reader.key_a = MF_CLASSIC_NO_KEY;
        }
        // Load key B
        if(mf_classic_is_key_found(data, i, MfClassicKeyB)) {
            sec_reader.key_b = nfc_util_bytes2num(sec_tr->key_b, 6);
        } else {
            sec_reader.key_b = MF_CLASSIC_NO_KEY;
        }
        if((key_a != MF_CLASSIC_NO_KEY) || (key_b != MF_CLASSIC_NO_KEY)) {
            sec_reader.sector_num = i;
            if(mf_classic_read_sector_with_reader(tx_rx, &crypto, &sec_reader, &temp_sector)) {
                uint8_t first_block = mf_classic_get_first_block_num_of_sector(i);
                for(uint8_t j = 0; j < temp_sector.total_blocks; j++) {
                    mf_classic_set_block_read(data, first_block + j, &temp_sector.block[j]);
                }
                sectors_read++;
            }
        }
    }
    return sectors_read;
}

void mf_crypto1_decrypt(
    Crypto1* crypto,
    uint8_t* encrypted_data,
    uint16_t encrypted_data_bits,
    uint8_t* decrypted_data) {
    if(encrypted_data_bits < 8) {
        uint8_t decrypted_byte = 0;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_data[0], 0)) << 0;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_data[0], 1)) << 1;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_data[0], 2)) << 2;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_data[0], 3)) << 3;
        decrypted_data[0] = decrypted_byte;
    } else {
        for(size_t i = 0; i < encrypted_data_bits / 8; i++) {
            decrypted_data[i] = crypto1_byte(crypto, 0, 0) ^ encrypted_data[i];
        }
    }
}

void mf_crypto1_encrypt(
    Crypto1* crypto,
    uint8_t* keystream,
    uint8_t* plain_data,
    uint16_t plain_data_bits,
    uint8_t* encrypted_data,
    uint8_t* encrypted_parity) {
    if(plain_data_bits < 8) {
        encrypted_data[0] = 0;
        for(size_t i = 0; i < plain_data_bits; i++) {
            encrypted_data[0] |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(plain_data[0], i)) << i;
        }
    } else {
        memset(encrypted_parity, 0, plain_data_bits / 8 + 1);
        for(uint8_t i = 0; i < plain_data_bits / 8; i++) {
            encrypted_data[i] = crypto1_byte(crypto, keystream ? keystream[i] : 0, 0) ^
                                plain_data[i];
            encrypted_parity[i / 8] |=
                (((crypto1_filter(crypto->odd) ^ nfc_util_odd_parity8(plain_data[i])) & 0x01)
                 << (7 - (i & 0x0007)));
        }
    }
}

bool mf_classic_emulator(MfClassicEmulator* emulator, FuriHalNfcTxRxContext* tx_rx) {
    furi_assert(emulator);
    furi_assert(tx_rx);
    bool command_processed = false;
    bool is_encrypted = false;
    uint8_t plain_data[MF_CLASSIC_MAX_DATA_SIZE];
    MfClassicKey access_key = MfClassicKeyA;

    // Read command
    while(!command_processed) {
        if(!is_encrypted) {
            crypto1_reset(&emulator->crypto);
            memcpy(plain_data, tx_rx->rx_data, tx_rx->rx_bits / 8);
        } else {
            if(!furi_hal_nfc_tx_rx(tx_rx, 300)) {
                FURI_LOG_D(
                    TAG,
                    "Error in tx rx. Tx :%d bits, Rx: %d bits",
                    tx_rx->tx_bits,
                    tx_rx->rx_bits);
                break;
            }
            mf_crypto1_decrypt(&emulator->crypto, tx_rx->rx_data, tx_rx->rx_bits, plain_data);
        }

        if(plain_data[0] == 0x50 && plain_data[1] == 0x00) {
            FURI_LOG_T(TAG, "Halt received");
            furi_hal_nfc_listen_sleep();
            command_processed = true;
            break;
        } else if(plain_data[0] == 0x60 || plain_data[0] == 0x61) {
            uint8_t block = plain_data[1];
            uint64_t key = 0;
            uint8_t sector_trailer_block = mf_classic_get_sector_trailer_num_by_block(block);
            MfClassicSectorTrailer* sector_trailer =
                (MfClassicSectorTrailer*)emulator->data.block[sector_trailer_block].value;
            if(plain_data[0] == 0x60) {
                key = nfc_util_bytes2num(sector_trailer->key_a, 6);
                access_key = MfClassicKeyA;
            } else {
                key = nfc_util_bytes2num(sector_trailer->key_b, 6);
                access_key = MfClassicKeyB;
            }

            uint32_t nonce = prng_successor(DWT->CYCCNT, 32) ^ 0xAA;
            uint8_t nt[4];
            uint8_t nt_keystream[4];
            nfc_util_num2bytes(nonce, 4, nt);
            nfc_util_num2bytes(nonce ^ emulator->cuid, 4, nt_keystream);
            crypto1_init(&emulator->crypto, key);
            if(!is_encrypted) {
                crypto1_word(&emulator->crypto, emulator->cuid ^ nonce, 0);
                memcpy(tx_rx->tx_data, nt, sizeof(nt));
                tx_rx->tx_parity[0] = 0;
                for(size_t i = 0; i < sizeof(nt); i++) {
                    tx_rx->tx_parity[0] |= nfc_util_odd_parity8(nt[i]) << (7 - i);
                }
                tx_rx->tx_bits = sizeof(nt) * 8;
                tx_rx->tx_rx_type = FuriHalNfcTxRxTransparent;
            } else {
                mf_crypto1_encrypt(
                    &emulator->crypto,
                    nt_keystream,
                    nt,
                    sizeof(nt) * 8,
                    tx_rx->tx_data,
                    tx_rx->tx_parity);
                tx_rx->tx_bits = sizeof(nt) * 8;
                tx_rx->tx_rx_type = FuriHalNfcTxRxTransparent;
            }
            if(!furi_hal_nfc_tx_rx(tx_rx, 500)) {
                FURI_LOG_E(TAG, "Error in NT exchange");
                command_processed = true;
                break;
            }

            if(tx_rx->rx_bits != 64) {
                FURI_LOG_W(TAG, "Incorrect nr + ar");
                command_processed = true;
                break;
            }

            uint32_t nr = nfc_util_bytes2num(tx_rx->rx_data, 4);
            uint32_t ar = nfc_util_bytes2num(&tx_rx->rx_data[4], 4);

            FURI_LOG_D(
                TAG,
                "%08x key%c block %d nt/nr/ar: %08x %08x %08x",
                emulator->cuid,
                access_key == MfClassicKeyA ? 'A' : 'B',
                sector_trailer_block,
                nonce,
                nr,
                ar);

            crypto1_word(&emulator->crypto, nr, 1);
            uint32_t cardRr = ar ^ crypto1_word(&emulator->crypto, 0, 0);
            if(cardRr != prng_successor(nonce, 64)) {
                FURI_LOG_T(TAG, "Wrong AUTH! %08X != %08X", cardRr, prng_successor(nonce, 64));
                // Don't send NACK, as the tag doesn't send it
                command_processed = true;
                break;
            }

            uint32_t ans = prng_successor(nonce, 96);
            uint8_t responce[4] = {};
            nfc_util_num2bytes(ans, 4, responce);
            mf_crypto1_encrypt(
                &emulator->crypto,
                NULL,
                responce,
                sizeof(responce) * 8,
                tx_rx->tx_data,
                tx_rx->tx_parity);
            tx_rx->tx_bits = sizeof(responce) * 8;
            tx_rx->tx_rx_type = FuriHalNfcTxRxTransparent;

            is_encrypted = true;
        } else if(is_encrypted && plain_data[0] == 0x30) {
            uint8_t block = plain_data[1];
            uint8_t block_data[18] = {};
            memcpy(block_data, emulator->data.block[block].value, MF_CLASSIC_BLOCK_SIZE);
            if(mf_classic_is_sector_trailer(block)) {
                if(!mf_classic_is_allowed_access(
                       emulator, block, access_key, MfClassicActionKeyARead)) {
                    memset(block_data, 0, 6);
                }
                if(!mf_classic_is_allowed_access(
                       emulator, block, access_key, MfClassicActionKeyBRead)) {
                    memset(&block_data[10], 0, 6);
                }
                if(!mf_classic_is_allowed_access(
                       emulator, block, access_key, MfClassicActionACRead)) {
                    memset(&block_data[6], 0, 4);
                }
            } else {
                if(!mf_classic_is_allowed_access(
                       emulator, block, access_key, MfClassicActionDataRead)) {
                    // Send NACK
                    uint8_t nack = 0x04;
                    if(is_encrypted) {
                        mf_crypto1_encrypt(
                            &emulator->crypto, NULL, &nack, 4, tx_rx->tx_data, tx_rx->tx_parity);
                    } else {
                        tx_rx->tx_data[0] = nack;
                    }
                    tx_rx->tx_rx_type = FuriHalNfcTxRxTransparent;
                    tx_rx->tx_bits = 4;
                    furi_hal_nfc_tx_rx(tx_rx, 300);
                    break;
                }
            }
            nfca_append_crc16(block_data, 16);

            mf_crypto1_encrypt(
                &emulator->crypto,
                NULL,
                block_data,
                sizeof(block_data) * 8,
                tx_rx->tx_data,
                tx_rx->tx_parity);
            tx_rx->tx_bits = 18 * 8;
            tx_rx->tx_rx_type = FuriHalNfcTxRxTransparent;
        } else if(is_encrypted && plain_data[0] == 0xA0) {
            uint8_t block = plain_data[1];
            if(block > mf_classic_get_total_block_num(emulator->data.type)) {
                break;
            }
            // Send ACK
            uint8_t ack = 0x0A;
            mf_crypto1_encrypt(&emulator->crypto, NULL, &ack, 4, tx_rx->tx_data, tx_rx->tx_parity);
            tx_rx->tx_rx_type = FuriHalNfcTxRxTransparent;
            tx_rx->tx_bits = 4;

            if(!furi_hal_nfc_tx_rx(tx_rx, 300)) break;
            if(tx_rx->rx_bits != 18 * 8) break;

            mf_crypto1_decrypt(&emulator->crypto, tx_rx->rx_data, tx_rx->rx_bits, plain_data);
            uint8_t block_data[16] = {};
            memcpy(block_data, emulator->data.block[block].value, MF_CLASSIC_BLOCK_SIZE);
            if(mf_classic_is_sector_trailer(block)) {
                if(mf_classic_is_allowed_access(
                       emulator, block, access_key, MfClassicActionKeyAWrite)) {
                    memcpy(block_data, plain_data, 6);
                }
                if(mf_classic_is_allowed_access(
                       emulator, block, access_key, MfClassicActionKeyBWrite)) {
                    memcpy(&block_data[10], &plain_data[10], 6);
                }
                if(mf_classic_is_allowed_access(
                       emulator, block, access_key, MfClassicActionACWrite)) {
                    memcpy(&block_data[6], &plain_data[6], 4);
                }
            } else {
                if(mf_classic_is_allowed_access(
                       emulator, block, access_key, MfClassicActionDataWrite)) {
                    memcpy(block_data, plain_data, MF_CLASSIC_BLOCK_SIZE);
                }
            }
            if(memcmp(block_data, emulator->data.block[block].value, MF_CLASSIC_BLOCK_SIZE)) {
                memcpy(emulator->data.block[block].value, block_data, MF_CLASSIC_BLOCK_SIZE);
                emulator->data_changed = true;
            }
            // Send ACK
            ack = 0x0A;
            mf_crypto1_encrypt(&emulator->crypto, NULL, &ack, 4, tx_rx->tx_data, tx_rx->tx_parity);
            tx_rx->tx_rx_type = FuriHalNfcTxRxTransparent;
            tx_rx->tx_bits = 4;
        } else {
            // Unknown command
            break;
        }
    }

    if(!command_processed) {
        // Send NACK
        uint8_t nack = 0x04;
        if(is_encrypted) {
            mf_crypto1_encrypt(
                &emulator->crypto, NULL, &nack, 4, tx_rx->tx_data, tx_rx->tx_parity);
        } else {
            tx_rx->tx_data[0] = nack;
        }
        tx_rx->tx_rx_type = FuriHalNfcTxRxTransparent;
        tx_rx->tx_bits = 4;
        furi_hal_nfc_tx_rx(tx_rx, 300);
    }

    return true;
}
