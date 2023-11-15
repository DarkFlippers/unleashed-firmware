#pragma once

#include "mf_classic_poller.h"
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller_i.h>
#include <lib/nfc/helpers/nfc_util.h>
#include "crypto1.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MF_CLASSIC_FWT_FC (60000)

typedef enum {
    MfClassicAuthStateIdle,
    MfClassicAuthStatePassed,
} MfClassicAuthState;

typedef enum {
    MfClassicCardStateDetected,
    MfClassicCardStateLost,
} MfClassicCardState;

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
