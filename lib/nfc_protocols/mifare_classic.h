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
    uint8_t key_a[6];
    uint8_t access_bits[4];
    uint8_t key_b[6];
} MfClassicSectorTrailer;

typedef struct {
    uint8_t total_blocks;
    MfClassicBlock block[MF_CLASSIC_BLOCKS_IN_SECTOR_MAX];
} MfClassicSector;

typedef struct {
    MfClassicType type;
    MfClassicBlock block[MF_CLASSIC_TOTAL_BLOCKS_MAX];
} MfClassicData;

typedef struct {
    uint32_t cuid;
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
    uint32_t cuid;
    uint8_t sectors_to_read;
    Crypto1 crypto;
    MfClassicSectorReader sector_reader[MF_CLASSIC_SECTORS_MAX];
} MfClassicReader;

bool mf_classic_check_card_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK);

bool mf_classic_get_type(
    uint8_t* uid,
    uint8_t uid_len,
    uint8_t ATQA0,
    uint8_t ATQA1,
    uint8_t SAK,
    MfClassicReader* reader);

uint8_t mf_classic_get_total_sectors_num(MfClassicReader* reader);

void mf_classic_auth_init_context(MfClassicAuthContext* auth_ctx, uint32_t cuid, uint8_t sector);

bool mf_classic_auth_attempt(
    FuriHalNfcTxRxContext* tx_rx,
    MfClassicAuthContext* auth_ctx,
    uint64_t key);

void mf_classic_reader_add_sector(
    MfClassicReader* reader,
    uint8_t sector,
    uint64_t key_a,
    uint64_t key_b);

bool mf_classic_read_sector(
    FuriHalNfcTxRxContext* tx_rx,
    Crypto1* crypto,
    MfClassicSectorReader* sector_reader,
    MfClassicSector* sector);

uint8_t mf_classic_read_card(
    FuriHalNfcTxRxContext* tx_rx,
    MfClassicReader* reader,
    MfClassicData* data);
