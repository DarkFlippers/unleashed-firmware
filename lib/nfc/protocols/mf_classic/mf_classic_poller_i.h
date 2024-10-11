#pragma once

#include "mf_classic_poller.h"
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller_i.h>
#include <bit_lib/bit_lib.h>
#include <nfc/helpers/iso14443_crc.h>
#include <nfc/helpers/crypto1.h>
#include <stream/stream.h>
#include <stream/buffered_file_stream.h>
#include <toolbox/keys_dict.h>
#include <helpers/nfc_util.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MF_CLASSIC_FWT_FC                       (60000)
#define NFC_FOLDER                              EXT_PATH("nfc")
#define NFC_ASSETS_FOLDER                       EXT_PATH("nfc/assets")
#define MF_CLASSIC_NESTED_ANALYZE_NT_COUNT      (5)
#define MF_CLASSIC_NESTED_NT_HARD_MINIMUM       (3)
#define MF_CLASSIC_NESTED_RETRY_MAXIMUM         (60)
#define MF_CLASSIC_NESTED_HARD_RETRY_MAXIMUM    (3)
#define MF_CLASSIC_NESTED_CALIBRATION_COUNT     (21)
#define MF_CLASSIC_NESTED_LOGS_FILE_NAME        ".nested.log"
#define MF_CLASSIC_NESTED_SYSTEM_DICT_FILE_NAME "mf_classic_dict_nested.nfc"
#define MF_CLASSIC_NESTED_USER_DICT_FILE_NAME   "mf_classic_dict_user_nested.nfc"
#define MF_CLASSIC_NESTED_LOGS_FILE_PATH        (NFC_FOLDER "/" MF_CLASSIC_NESTED_LOGS_FILE_NAME)
#define MF_CLASSIC_NESTED_SYSTEM_DICT_PATH \
    (NFC_ASSETS_FOLDER "/" MF_CLASSIC_NESTED_SYSTEM_DICT_FILE_NAME)
#define MF_CLASSIC_NESTED_USER_DICT_PATH \
    (NFC_ASSETS_FOLDER "/" MF_CLASSIC_NESTED_USER_DICT_FILE_NAME)
#define SET_PACKED_BIT(arr, bit) ((arr)[(bit) / 8] |= (1 << ((bit) % 8)))
#define GET_PACKED_BIT(arr, bit) ((arr)[(bit) / 8] & (1 << ((bit) % 8)))

extern const MfClassicKey auth1_backdoor_key;
extern const MfClassicKey auth2_backdoor_key;
extern const MfClassicKey auth3_backdoor_key;
extern const uint16_t valid_sums[19];

typedef enum {
    MfClassicAuthStateIdle,
    MfClassicAuthStatePassed,
} MfClassicAuthState;

typedef enum {
    MfClassicCardStateDetected,
    MfClassicCardStateLost,
} MfClassicCardState;

typedef struct {
    MfClassicKey key;
    MfClassicBackdoor type;
} MfClassicBackdoorKeyPair;

extern const MfClassicBackdoorKeyPair mf_classic_backdoor_keys[];
extern const size_t mf_classic_backdoor_keys_count;

typedef struct {
    uint32_t cuid; // Card UID
    uint8_t key_idx; // Key index
    uint32_t nt; // Nonce
    uint32_t nt_enc; // Encrypted nonce
    uint8_t par; // Parity
    uint16_t dist; // Distance
} MfClassicNestedNonce;

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
    MfClassicPollerStateAnalyzeBackdoor,
    MfClassicPollerStateBackdoorReadSector,
    MfClassicPollerStateRequestKey,
    MfClassicPollerStateReadSector,
    MfClassicPollerStateAuthKeyA,
    MfClassicPollerStateAuthKeyB,
    MfClassicPollerStateKeyReuseStart,
    MfClassicPollerStateKeyReuseStartNoOffset,
    MfClassicPollerStateKeyReuseAuthKeyA,
    MfClassicPollerStateKeyReuseAuthKeyB,
    MfClassicPollerStateKeyReuseReadSector,
    MfClassicPollerStateSuccess,
    MfClassicPollerStateFail,

    // Enhanced dictionary attack states
    MfClassicPollerStateNestedAnalyzePRNG,
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

typedef struct {
    uint8_t current_sector;
    MfClassicKey current_key;
    MfClassicKeyType current_key_type;
    bool auth_passed;
    uint16_t current_block;
    uint8_t reuse_key_sector;
    MfClassicBackdoor backdoor;
    // Enhanced dictionary attack and nested nonce collection
    bool enhanced_dict;
    MfClassicNestedPhase nested_phase;
    MfClassicKey nested_known_key;
    MfClassicKeyType nested_known_key_type;
    bool current_key_checked;
    uint8_t nested_known_key_sector;
    uint16_t nested_target_key;
    MfClassicNestedNonceArray nested_nonce;
    MfClassicPrngType prng_type;
    bool static_encrypted;
    uint32_t static_encrypted_nonce;
    bool calibrated;
    uint16_t d_min;
    uint16_t d_max;
    uint8_t attempt_count;
    KeysDict* mf_classic_system_dict;
    KeysDict* mf_classic_user_dict;
    // Hardnested
    uint8_t nt_enc_msb
        [32]; // Bit-packed array to track which unique most significant bytes have been seen (256 bits = 32 bytes)
    uint16_t msb_par_sum; // Sum of parity bits for each unique most significant byte
    uint16_t msb_count; // Number of unique most significant bytes seen
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
