#pragma once

#include <lib/nfc/protocols/nfc_util.h>
#include <lib/nfc/protocols/mifare_classic.h>
#include <lib/nfc/protocols/crypto1.h>

#include <storage/storage.h>
#include <stream/stream.h>
#include <stream/buffered_file_stream.h>

typedef enum {
    MifareNestedNonceNoTag,
    MifareNestedNonceWeak,
    MifareNestedNonceStatic,
    MifareNestedNonceHard,
} MifareNestedNonceType;

MifareNestedNonceType nested_check_nonce_type(FuriHalNfcTxRxContext* tx_rx, uint8_t blockNo);

struct nonce_info_static {
    uint32_t cuid;
    uint32_t target_nt[2];
    uint32_t target_ks[2];
    bool full;
};

struct nonce_info_hard {
    uint32_t cuid;
    bool static_encrypted;
    bool full;
};

struct nonce_info {
    uint32_t cuid;
    uint32_t target_nt[2];
    uint32_t target_ks[2];
    uint8_t parity[2][4];
    bool full;
};

struct distance_info {
    uint32_t min_prng;
    uint32_t max_prng;
    uint32_t mid_prng;
};

struct nonce_info_static nested_static_nonce_attack(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint8_t targetBlockNo,
    uint8_t targetKeyType,
    uint64_t ui64Key);

struct nonce_info nested_attack(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint8_t targetBlockNo,
    uint8_t targetKeyType,
    uint64_t ui64Key,
    uint32_t distance,
    uint32_t delay);

struct nonce_info_hard nested_hard_nonce_attack(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint8_t targetBlockNo,
    uint8_t targetKeyType,
    uint64_t ui64Key,
    uint32_t* found,
    uint32_t* first_byte_sum,
    Stream* file_stream);

uint32_t nested_calibrate_distance(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint64_t ui64Key,
    uint32_t delay,
    bool full);

struct distance_info nested_calibrate_distance_info(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint64_t ui64Key);

typedef enum {
    NestedCheckKeyNoTag,
    NestedCheckKeyValid,
    NestedCheckKeyInvalid,
} NestedCheckKeyResult;

NestedCheckKeyResult nested_check_key(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint64_t ui64Key);

bool nested_check_block(FuriHalNfcTxRxContext* tx_rx, uint8_t blockNo, uint8_t keyType);

void nested_get_data();

bool mifare_classic_authex(
    Crypto1* crypto,
    FuriHalNfcTxRxContext* tx_rx,
    uint32_t uid,
    uint32_t blockNo,
    uint32_t keyType,
    uint64_t ui64Key,
    bool isNested,
    uint32_t* ntptr);

void nfc_activate();

void nfc_deactivate();
