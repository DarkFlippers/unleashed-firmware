#pragma once

#include <furi_hal_nfc.h>

#include "crypto1.h"

#define MF_CLASSIC_BLOCK_SIZE (16)
#define MF_CLASSIC_TOTAL_BLOCKS_MAX (256)
#define MF_CLASSIC_1K_TOTAL_SECTORS_NUM (16)
#define MF_CLASSIC_4K_TOTAL_SECTORS_NUM (40)

#define MF_CLASSIC_SECTORS_MAX (40)
#define MF_CLASSIC_BLOCKS_IN_SECTOR_MAX (16)

#define MF_CLASSIC_NO_KEY (0xFFFFFFFFFFFFFFFF)
#define MF_CLASSIC_MAX_DATA_SIZE (16)
#define MF_CLASSIC_KEY_SIZE (6)
#define MF_CLASSIC_ACCESS_BYTES_SIZE (4)

typedef enum {
    MfClassicType1k,
    MfClassicType4k,
} MfClassicType;

typedef enum {
    MfClassicKeyA,
    MfClassicKeyB,
} MfClassicKey;

typedef struct {
    uint8_t value[MF_CLASSIC_BLOCK_SIZE];
} MfClassicBlock;

typedef struct {
    uint8_t key_a[MF_CLASSIC_KEY_SIZE];
    uint8_t access_bits[MF_CLASSIC_ACCESS_BYTES_SIZE];
    uint8_t key_b[MF_CLASSIC_KEY_SIZE];
} MfClassicSectorTrailer;

typedef struct {
    uint8_t total_blocks;
    MfClassicBlock block[MF_CLASSIC_BLOCKS_IN_SECTOR_MAX];
} MfClassicSector;

typedef struct {
    MfClassicType type;
    uint32_t block_read_mask[MF_CLASSIC_TOTAL_BLOCKS_MAX / 32];
    uint64_t key_a_mask;
    uint64_t key_b_mask;
    MfClassicBlock block[MF_CLASSIC_TOTAL_BLOCKS_MAX];
} MfClassicData;

typedef struct {
    uint8_t sector;
    uint64_t key_a;
    uint64_t key_b;
} MfClassicAuthContext;

typedef struct {
    uint8_t sector_num;
    uint64_t key_a;
    uint64_t key_b;
} MfClassicSectorReader;

typedef struct {
    MfClassicType type;
    Crypto1 crypto;
    uint8_t sectors_to_read;
    MfClassicSectorReader sector_reader[MF_CLASSIC_SECTORS_MAX];
} MfClassicReader;

typedef struct {
    uint32_t cuid;
    Crypto1 crypto;
    MfClassicData data;
    bool data_changed;
} MfClassicEmulator;

const char* mf_classic_get_type_str(MfClassicType type);

bool mf_classic_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK);

MfClassicType mf_classic_get_classic_type(int8_t ATQA0, uint8_t ATQA1, uint8_t SAK);

uint8_t mf_classic_get_total_sectors_num(MfClassicType type);

uint8_t mf_classic_get_sector_trailer_block_num_by_sector(uint8_t sector);

bool mf_classic_is_sector_trailer(uint8_t block);

uint8_t mf_classic_get_sector_by_block(uint8_t block);

bool mf_classic_is_key_found(MfClassicData* data, uint8_t sector_num, MfClassicKey key_type);

void mf_classic_set_key_found(
    MfClassicData* data,
    uint8_t sector_num,
    MfClassicKey key_type,
    uint64_t key);

bool mf_classic_is_block_read(MfClassicData* data, uint8_t block_num);

void mf_classic_set_block_read(MfClassicData* data, uint8_t block_num, MfClassicBlock* block_data);

bool mf_classic_is_sector_read(MfClassicData* data, uint8_t sector_num);

void mf_classic_get_read_sectors_and_keys(
    MfClassicData* data,
    uint8_t* sectors_read,
    uint8_t* keys_found);

MfClassicSectorTrailer*
    mf_classic_get_sector_trailer_by_sector(MfClassicData* data, uint8_t sector);

void mf_classic_auth_init_context(MfClassicAuthContext* auth_ctx, uint8_t sector);

bool mf_classic_authenticate(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t block_num,
    uint64_t key,
    MfClassicKey key_type);

bool mf_classic_auth_attempt(
    FuriHalNfcTxRxContext* tx_rx,
    MfClassicAuthContext* auth_ctx,
    uint64_t key);

void mf_classic_reader_add_sector(
    MfClassicReader* reader,
    uint8_t sector,
    uint64_t key_a,
    uint64_t key_b);

void mf_classic_read_sector(FuriHalNfcTxRxContext* tx_rx, MfClassicData* data, uint8_t sec_num);

uint8_t mf_classic_read_card(
    FuriHalNfcTxRxContext* tx_rx,
    MfClassicReader* reader,
    MfClassicData* data);

uint8_t mf_classic_update_card(FuriHalNfcTxRxContext* tx_rx, MfClassicData* data);

bool mf_classic_emulator(MfClassicEmulator* emulator, FuriHalNfcTxRxContext* tx_rx);
