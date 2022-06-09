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

static uint8_t mf_classic_get_first_block_num_of_sector(uint8_t sector) {
    furi_assert(sector < 40);
    if(sector < 32) {
        return sector * 4;
    } else {
        return 32 * 4 + (sector - 32) * 16;
    }
}

static uint8_t mf_classic_get_sector_by_block(uint8_t block) {
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

static uint8_t mf_classic_get_sector_trailer(uint8_t block) {
    if(block < 128) {
        return block | 0x03;
    } else {
        return block | 0x0f;
    }
}

static bool mf_classic_is_sector_trailer(uint8_t block) {
    return block == mf_classic_get_sector_trailer(block);
}

uint8_t mf_classic_get_total_sectors_num(MfClassicReader* reader) {
    furi_assert(reader);
    if(reader->type == MfClassicType1k) {
        return MF_CLASSIC_1K_TOTAL_SECTORS_NUM;
    } else if(reader->type == MfClassicType4k) {
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
    uint8_t* sector_trailer = emulator->data.block[mf_classic_get_sector_trailer(block_num)].value;

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
    if((ATQA0 == 0x44 || ATQA0 == 0x04) && (SAK == 0x08)) {
        return true;
    } else if((ATQA0 == 0x42 || ATQA0 == 0x02) && (SAK == 0x18)) {
        return true;
    } else {
        return false;
    }
}

bool mf_classic_get_type(
    uint8_t* uid,
    uint8_t uid_len,
    uint8_t ATQA0,
    uint8_t ATQA1,
    uint8_t SAK,
    MfClassicReader* reader) {
    UNUSED(ATQA1);
    furi_assert(uid);
    furi_assert(reader);
    memset(reader, 0, sizeof(MfClassicReader));

    if((ATQA0 == 0x44 || ATQA0 == 0x04) && (SAK == 0x08)) {
        reader->type = MfClassicType1k;
    } else if((ATQA0 == 0x42 || ATQA0 == 0x02) && (SAK == 0x18)) {
        reader->type = MfClassicType4k;
    } else {
        return false;
    }

    uint8_t* cuid_start = uid;
    if(uid_len == 7) {
        cuid_start = &uid[3];
    }
    reader->cuid = (cuid_start[0] << 24) | (cuid_start[1] << 16) | (cuid_start[2] << 8) |
                   (cuid_start[3]);

    return true;
}

void mf_classic_reader_add_sector(
    MfClassicReader* reader,
    uint8_t sector,
    uint64_t key_a,
    uint64_t key_b) {
    furi_assert(reader);
    furi_assert(sector < MF_CLASSIC_SECTORS_MAX);
    furi_assert((key_a != MF_CLASSIC_NO_KEY) || (key_b != MF_CLASSIC_NO_KEY));

    if(reader->sectors_to_read < MF_CLASSIC_SECTORS_MAX - 1) {
        reader->sector_reader[reader->sectors_to_read].key_a = key_a;
        reader->sector_reader[reader->sectors_to_read].key_b = key_b;
        reader->sector_reader[reader->sectors_to_read].sector_num = sector;
        reader->sectors_to_read++;
    }
}

void mf_classic_auth_init_context(MfClassicAuthContext* auth_ctx, uint32_t cuid, uint8_t sector) {
    furi_assert(auth_ctx);
    auth_ctx->cuid = cuid;
    auth_ctx->sector = sector;
    auth_ctx->key_a = MF_CLASSIC_NO_KEY;
    auth_ctx->key_b = MF_CLASSIC_NO_KEY;
}

static bool mf_classic_auth(
    FuriHalNfcTxRxContext* tx_rx,
    uint32_t cuid,
    uint32_t block,
    uint64_t key,
    MfClassicKey key_type,
    Crypto1* crypto) {
    bool auth_success = false;
    memset(tx_rx->tx_data, 0, sizeof(tx_rx->tx_data));
    memset(tx_rx->tx_parity, 0, sizeof(tx_rx->tx_parity));
    tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;

    do {
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
               auth_ctx->cuid,
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
        furi_hal_nfc_activate_nfca(300, &auth_ctx->cuid);
    }

    if(auth_ctx->key_b == MF_CLASSIC_NO_KEY) {
        // Try AUTH with key B
        if(mf_classic_auth(
               tx_rx,
               auth_ctx->cuid,
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
        if(tx_rx->rx_bits == 8 * 18) {
            for(uint8_t i = 0; i < 18; i++) {
                block->value[i] = crypto1_byte(crypto, 0, 0) ^ tx_rx->rx_data[i];
            }
            read_block_success = true;
        }
    }
    return read_block_success;
}

bool mf_classic_read_sector(
    FuriHalNfcTxRxContext* tx_rx,
    Crypto1* crypto,
    MfClassicSectorReader* sector_reader,
    MfClassicSector* sector) {
    furi_assert(tx_rx);
    furi_assert(sector_reader);
    furi_assert(sector);

    uint32_t cuid = 0;
    uint64_t key;
    MfClassicKey key_type;
    uint8_t first_block;
    bool sector_read = false;

    furi_hal_nfc_sleep();
    do {
        // Activate card
        if(!furi_hal_nfc_activate_nfca(200, &cuid)) break;
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
        if(!mf_classic_auth(tx_rx, cuid, first_block, key, key_type, crypto)) break;
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
        if(mf_classic_read_sector(
               tx_rx, &reader->crypto, &reader->sector_reader[i], &temp_sector)) {
            uint8_t first_block =
                mf_classic_get_first_block_num_of_sector(reader->sector_reader[i].sector_num);
            for(uint8_t j = 0; j < temp_sector.total_blocks; j++) {
                data->block[first_block + j] = temp_sector.block[j];
            }
            if(reader->sector_reader[i].key_a != MF_CLASSIC_NO_KEY) {
                data->key_a_mask |= 1 << reader->sector_reader[i].sector_num;
            }
            if(reader->sector_reader[i].key_b != MF_CLASSIC_NO_KEY) {
                data->key_b_mask |= 1 << reader->sector_reader[i].sector_num;
            }
            sectors_read++;
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
            // Read first frame
            tx_rx->tx_bits = 0;
            tx_rx->tx_rx_type = FuriHalNfcTxRxTypeDefault;
        }
        if(!furi_hal_nfc_tx_rx(tx_rx, 300)) {
            FURI_LOG_D(
                TAG, "Error in tx rx. Tx :%d bits, Rx: %d bits", tx_rx->tx_bits, tx_rx->rx_bits);
            break;
        }
        if(!is_encrypted) {
            memcpy(plain_data, tx_rx->rx_data, tx_rx->rx_bits / 8);
        } else {
            mf_crypto1_decrypt(&emulator->crypto, tx_rx->rx_data, tx_rx->rx_bits, plain_data);
        }
        // TODO Check crc

        if(plain_data[0] == 0x50 && plain_data[1] == 00) {
            FURI_LOG_T(TAG, "Halt received");
            command_processed = true;
            break;
        } else if(plain_data[0] == 0x60 || plain_data[0] == 0x61) {
            uint8_t block = plain_data[1];
            uint64_t key = 0;
            uint8_t sector_trailer_block = mf_classic_get_sector_trailer(block);
            MfClassicSectorTrailer* sector_trailer =
                (MfClassicSectorTrailer*)emulator->data.block[sector_trailer_block].value;
            if(plain_data[0] == 0x61) {
                key = nfc_util_bytes2num(sector_trailer->key_b, 6);
                access_key = MfClassicKeyA;
            } else {
                key = nfc_util_bytes2num(sector_trailer->key_a, 6);
                access_key = MfClassicKeyB;
            }

            uint32_t nonce = prng_successor(DWT->CYCCNT, 32);
            uint8_t nt[4];
            uint8_t nt_keystream[4];
            nfc_util_num2bytes(nonce, 4, nt);
            nfc_util_num2bytes(nonce ^ emulator->cuid, 4, nt_keystream);
            crypto1_init(&emulator->crypto, key);
            if(!is_encrypted) {
                crypto1_word(&emulator->crypto, emulator->cuid ^ nonce, 0);
                memcpy(tx_rx->tx_data, nt, sizeof(nt));
                tx_rx->tx_bits = sizeof(nt) * 8;
                tx_rx->tx_rx_type = FuriHalNfcTxRxTypeRxRaw;
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

            // Check if we store valid key
            if(access_key == MfClassicKeyA) {
                if(FURI_BIT(emulator->data.key_a_mask, mf_classic_get_sector_by_block(block)) ==
                   0) {
                    FURI_LOG_D(TAG, "Unsupported sector key A for block %d", sector_trailer_block);
                    break;
                }
            } else if(access_key == MfClassicKeyB) {
                if(FURI_BIT(emulator->data.key_b_mask, mf_classic_get_sector_by_block(block)) ==
                   0) {
                    FURI_LOG_D(TAG, "Unsupported sector key B for block %d", sector_trailer_block);
                    break;
                }
            }

            uint32_t nr = nfc_util_bytes2num(tx_rx->rx_data, 4);
            uint32_t ar = nfc_util_bytes2num(&tx_rx->rx_data[4], 4);
            crypto1_word(&emulator->crypto, nr, 1);
            uint32_t cardRr = ar ^ crypto1_word(&emulator->crypto, 0, 0);
            if(cardRr != prng_successor(nonce, 64)) {
                FURI_LOG_T(TAG, "Wrong AUTH! %08X != %08X", cardRr, prng_successor(nonce, 64));
                // Don't send NACK, as tag don't send it
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
                    memset(block_data, 0, 16);
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
