#pragma once

#include "mf_classic_poller.h"
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller_i.h>
#include <bit_lib/bit_lib.h>
#include "nfc/helpers/iso14443_crc.h"
#include <nfc/helpers/crypto1.h>
#include <stream/stream.h>
#include <stream/buffered_file_stream.h>
#include "keys_dict.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MF_CLASSIC_FWT_FC                       (60000)
#define NFC_FOLDER                              EXT_PATH("nfc")
#define NFC_ASSETS_FOLDER                       EXT_PATH("nfc/assets")
#define MF_CLASSIC_NESTED_LOGS_FILE_NAME        ".nested.log"
#define MF_CLASSIC_NESTED_SYSTEM_DICT_FILE_NAME "mf_classic_dict_nested.nfc"
#define MF_CLASSIC_NESTED_USER_DICT_FILE_NAME   "mf_classic_dict_user_nested.nfc"
#define MF_CLASSIC_NESTED_LOGS_FILE_PATH        (NFC_FOLDER "/" MF_CLASSIC_NESTED_LOGS_FILE_NAME)
#define MF_CLASSIC_NESTED_SYSTEM_DICT_PATH \
    (NFC_ASSETS_FOLDER "/" MF_CLASSIC_NESTED_SYSTEM_DICT_FILE_NAME)
#define MF_CLASSIC_NESTED_USER_DICT_PATH \
    (NFC_ASSETS_FOLDER "/" MF_CLASSIC_NESTED_USER_DICT_FILE_NAME)

typedef enum {
    MfClassicAuthStateIdle,
    MfClassicAuthStatePassed,
} MfClassicAuthState;

typedef enum {
    MfClassicCardStateDetected,
    MfClassicCardStateLost,
} MfClassicCardState;

typedef enum {
    MfClassicNestedStateNone,
    MfClassicNestedStateFailed,
    MfClassicNestedStatePassed,
} MfClassicNestedState;

typedef enum {
    MfClassicPrngTypeUnknown, // Tag not yet tested
    MfClassicPrngTypeNoTag, // No tag detected during test
    MfClassicPrngTypeWeak, // Weak PRNG, standard Nested
    MfClassicPrngTypeHard, // Hard PRNG, Hardnested
} MfClassicPrngType;

typedef enum {
    MfClassicBackdoorUnknown, // Tag not yet tested
    MfClassicBackdoorNone, // No observed backdoor
    MfClassicBackdoorFM11RF08S, // Tag responds to Fudan FM11RF08S backdoor (static encrypted nonce tags)
} MfClassicBackdoor;

typedef struct {
    uint32_t cuid; // Card UID
    uint8_t key_idx; // Key index
    uint32_t nt; // Nonce
    uint32_t nt_enc; // Encrypted nonce
    uint8_t par; // Parity
    uint16_t dist; // Distance
} MfClassicNestedNonce;

typedef struct {
    MfClassicKey* key_candidates;
    size_t count;
} MfClassicNestedKeyCandidateArray;

typedef struct {
    MfClassicNestedNonce* nonces;
    size_t count;
} MfClassicNestedNonceArray;

typedef enum {
    MfClassicPollerStateDetectType,
    MfClassicPollerStateStart,

    // Write states
    MfClassicPollerStateRequestSectorTrailer,
    MfClassicPollerStateCheckWriteConditions,
    MfClassicPollerStateReadBlock,
    MfClassicPollerStateWriteBlock,
    MfClassicPollerStateWriteValueBlock,

    // Read states
    MfClassicPollerStateRequestReadSector,
    MfClassicPollerStateReadSectorBlocks,

    // Dict attack states
    MfClassicPollerStateNextSector,
    MfClassicPollerStateRequestKey,
    MfClassicPollerStateReadSector,
    MfClassicPollerStateAuthKeyA,
    MfClassicPollerStateAuthKeyB,
    MfClassicPollerStateKeyReuseStart,
    MfClassicPollerStateKeyReuseAuthKeyA,
    MfClassicPollerStateKeyReuseAuthKeyB,
    MfClassicPollerStateKeyReuseReadSector,
    MfClassicPollerStateSuccess,
    MfClassicPollerStateFail,

    // Enhanced dictionary attack states
    MfClassicPollerStateNestedAnalyzePRNG,
    MfClassicPollerStateNestedAnalyzeBackdoor,
    MfClassicPollerStateNestedCalibrate,
    MfClassicPollerStateNestedCollectNt,
    MfClassicPollerStateNestedController,
    MfClassicPollerStateNestedCollectNtEnc,
    MfClassicPollerStateNestedDictAttack,
    MfClassicPollerStateNestedLog,

    MfClassicPollerStateNum,
} MfClassicPollerState;

typedef struct {
    uint8_t current_sector;
    MfClassicSectorTrailer sec_tr;
    uint16_t current_block;
    bool is_value_block;
    MfClassicKeyType key_type_read;
    MfClassicKeyType key_type_write;
    bool need_halt_before_write;
    MfClassicBlock tag_block;
} MfClassicPollerWriteContext;

// TODO: Investigate reducing the number of members of this struct by moving into a separate struct dedicated to nested dict attack
typedef struct {
    uint8_t current_sector;
    MfClassicKey current_key;
    MfClassicKeyType current_key_type;
    bool auth_passed;
    uint16_t current_block;
    uint8_t reuse_key_sector;
    // Enhanced dictionary attack and nested nonce collection
    MfClassicPrngType prng_type;
    MfClassicBackdoor backdoor;
    uint32_t nt_prev;
    uint32_t nt_next;
    uint8_t nt_count;
    uint8_t hard_nt_count;
    uint8_t nested_dict_target_key;
    uint8_t nested_target_key;
    MfClassicNestedNonceArray nested_nonce;
    bool static_encrypted;
    bool calibrated;
    uint16_t d_min;
    uint16_t d_max;
    uint8_t attempt_count;
    MfClassicNestedState nested_state;
    KeysDict* mf_classic_system_dict;
    KeysDict* mf_classic_user_dict;
} MfClassicPollerDictAttackContext;

typedef struct {
    uint8_t current_sector;
    uint16_t current_block;
    MfClassicKeyType key_type;
    MfClassicKey key;
    bool auth_passed;
} MfClassicPollerReadContext;

typedef union {
    MfClassicPollerWriteContext write_ctx;
    MfClassicPollerDictAttackContext dict_attack_ctx;
    MfClassicPollerReadContext read_ctx;

} MfClassicPollerModeContext;

struct MfClassicPoller {
    Iso14443_3aPoller* iso14443_3a_poller;

    MfClassicPollerState state;
    MfClassicAuthState auth_state;
    MfClassicCardState card_state;

    MfClassicType current_type_check;
    uint8_t sectors_total;
    MfClassicPollerModeContext mode_ctx;

    Crypto1* crypto;
    BitBuffer* tx_plain_buffer;
    BitBuffer* tx_encrypted_buffer;
    BitBuffer* rx_plain_buffer;
    BitBuffer* rx_encrypted_buffer;
    MfClassicData* data;

    NfcGenericEvent general_event;
    MfClassicPollerEvent mfc_event;
    MfClassicPollerEventData mfc_event_data;
    NfcGenericCallback callback;
    void* context;
};

typedef struct {
    uint8_t block;
    MfClassicKeyType key_type;
    MfClassicNt nt;
} MfClassicCollectNtContext;

typedef struct {
    uint8_t block_num;
    MfClassicKey key;
    MfClassicKeyType key_type;
    MfClassicBlock block;
} MfClassicReadBlockContext;

typedef struct {
    uint8_t block_num;
    MfClassicKey key;
    MfClassicKeyType key_type;
    MfClassicBlock block;
} MfClassicWriteBlockContext;

typedef struct {
    uint8_t block_num;
    MfClassicKey key;
    MfClassicKeyType key_type;
    int32_t value;
} MfClassicReadValueContext;

typedef struct {
    uint8_t block_num;
    MfClassicKey key;
    MfClassicKeyType key_type;
    MfClassicValueCommand value_cmd;
    int32_t data;
    int32_t new_value;
} MfClassicChangeValueContext;

typedef struct {
    MfClassicDeviceKeys keys;
    uint8_t current_sector;
} MfClassicReadContext;

typedef union {
    MfClassicCollectNtContext collect_nt_context;
    MfClassicAuthContext auth_context;
    MfClassicReadBlockContext read_block_context;
    MfClassicWriteBlockContext write_block_context;
    MfClassicReadValueContext read_value_context;
    MfClassicChangeValueContext change_value_context;
    MfClassicReadContext read_context;
} MfClassicPollerContextData;

MfClassicError mf_classic_process_error(Iso14443_3aError error);

MfClassicPoller* mf_classic_poller_alloc(Iso14443_3aPoller* iso14443_3a_poller);

void mf_classic_poller_free(MfClassicPoller* instance);

#ifdef __cplusplus
}
#endif
