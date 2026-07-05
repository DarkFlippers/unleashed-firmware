#pragma once

#include "mf_plus_poller.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller_i.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MF_PLUS_FWT_FC (60000)

typedef enum {
    MfPlusCardStateDetected,
    MfPlusCardStateLost,
} MfPlusCardState;

typedef enum {
    MfPlusPollerStateIdle,
    MfPlusPollerStateReadVersion,
    MfPlusPollerStateParseVersion,
    MfPlusPollerStateParseIso4,
    // SL3 read flow (entered only in read mode on an SL3 card)
    MfPlusPollerStateRequestMode,
    MfPlusPollerStateReadSignature,
    MfPlusPollerStateRequestKey,
    MfPlusPollerStateAuthSector,
    MfPlusPollerStateReadSectorBlocks,
    MfPlusPollerStateReadFailed,
    MfPlusPollerStateReadSuccess,

    MfPlusPollerStateNum,
} MfPlusPollerState;

// SL3 secure-messaging session established by a successful authentication.
typedef struct {
    uint8_t k_enc[MF_PLUS_KEY_SIZE];
    uint8_t k_mac[MF_PLUS_KEY_SIZE];
    uint8_t ti[4];
    uint16_t r_ctr;
    uint16_t w_ctr;
} MfPlusPollerSession;

struct MfPlusPoller {
    Iso14443_4aPoller* iso14443_4a_poller;

    MfPlusData* data;
    MfPlusPollerState state;
    MfPlusPollerSession session;

    // SL3 read-flow progress.
    MfPlusPollerMode mode;
    MfPlusKey current_key;
    uint8_t current_sector;
    MfPlusKeyType current_key_type;
    uint16_t current_block;
    bool sector_blocks_read;
    uint8_t sectors_read;
    uint8_t keys_found;

    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    BitBuffer* input_buffer;
    BitBuffer* result_buffer;

    MfPlusError error;
    NfcGenericEvent general_event;
    MfPlusPollerEvent mfp_event;
    MfPlusPollerEventData mfp_event_data;
    NfcGenericCallback callback;
    void* context;
};

// Result of the active SAK-0x20 disambiguation probe. AN10833 gives no passive way to separate a
// Plus in SL0 from one in SL3, nor a Plus from a DESFire (both answer SAK 0x20 + ATS), so the
// poller sends a command and classifies the reply.
typedef enum {
    MfPlusProbeResultSl0, // answered 0x09: still in personalization (SL0)
    MfPlusProbeResultSl3, // answered as a Plus but not SL0
    MfPlusProbeResultNotPlus, // DESFire / unsupported: let the poller chain continue
} MfPlusProbeResult;

MfPlusError mf_plus_process_error(Iso14443_4aError error);

// Sends WritePerso to an intentionally invalid block and classifies the reply (mirrors PM3
// `hf mfp info`). Returns MfPlusErrorNone with *result set on a clean exchange, an error otherwise.
MfPlusError mf_plus_poller_probe_security_level(MfPlusPoller* instance, MfPlusProbeResult* result);

MfPlusPoller* mf_plus_poller_alloc(Iso14443_4aPoller* iso14443_4a_poller);

void mf_plus_poller_free(MfPlusPoller* instance);

// --- SL3 secure-messaging primitives (call from inside the poller Ready handler) ---

// First AES authentication (0x70 then 0x72) to `sector` using `key` as KeyA/KeyB. On success
// fills `session` (new TI, counters reset) and derives the session keys.
MfPlusError mf_plus_poller_authenticate(
    MfPlusPoller* instance,
    uint8_t sector,
    MfPlusKeyType key_type,
    const MfPlusKey* key,
    MfPlusPollerSession* session);

// Encrypted + MAC'd read (0x31) of one block; verifies the response MAC and decrypts into `out`.
MfPlusError mf_plus_poller_read_encrypted_block(
    MfPlusPoller* instance,
    uint8_t block_num,
    MfPlusPollerSession* session,
    MfPlusBlock* out);

// Plaintext originality signature (Read_Sig, 0x3C). Sets *present = true and fills `signature`
// (MF_PLUS_SIGNATURE_SIZE bytes) when the card returns one; *present = false otherwise.
MfPlusError
    mf_plus_poller_read_signature(MfPlusPoller* instance, uint8_t* signature, bool* present);

#ifdef __cplusplus
}
#endif
