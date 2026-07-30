#pragma once

#include "mf_plus_listener.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_listener_i.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MfPlusListenerStateIdle, /**< No authentication in progress, no session. */
    MfPlusListenerStateAuthFirstDone, /**< AUTH_FIRST (0x70) processed, awaiting AUTH_CONTINUE. */
    MfPlusListenerStateAuthenticated, /**< A secure-messaging session is established. */
} MfPlusListenerState;

// Card-side secure-messaging session. Same shape as the poller's MfPlusPollerSession, kept private
// to the listener (the mf_plus_crypto primitives take raw key bytes, not a session, so no shared
// type is needed). Ephemeral: never persisted into MfPlusData.
typedef struct {
    uint8_t k_enc[MF_PLUS_KEY_SIZE];
    uint8_t k_mac[MF_PLUS_KEY_SIZE];
    uint8_t ti[4];
    uint16_t r_ctr;
    uint16_t w_ctr;
} MfPlusListenerSession;

struct MfPlusListener {
    Iso14443_4aListener* iso14443_4a_listener; // parent transport
    MfPlusData* data; // mutated in place -> emulate-exit .shd writeback

    MfPlusListenerState state;
    MfPlusKey auth_key; // key AUTH_FIRST resolved (consumed by AUTH_CONTINUE)
    uint8_t rnd_b[MF_PLUS_KEY_SIZE]; // card RndB generated in AUTH_FIRST
    MfPlusListenerSession session;
    uint8_t get_version_stage; // GetVersion chaining: 0 idle, 1 HW sent, 2 SW sent

    BitBuffer* tx_buffer;

    NfcGenericEvent generic_event;
    MfPlusListenerEvent mfp_event;
    MfPlusListenerEventData mfp_event_data;
    NfcGenericCallback callback;
    void* context;
};

// Command handlers (mf_plus_listener_i.c). Each parses `rx` (the decoded I-block APDU), performs
// its AES step against instance->data / instance->session, transmits the response, and returns the
// NfcCommand for the listener loop.
NfcCommand mf_plus_listener_auth_first_handler(MfPlusListener* instance, const BitBuffer* rx);

NfcCommand mf_plus_listener_auth_continue_handler(MfPlusListener* instance, const BitBuffer* rx);

// Handles both encrypted (0x31) and plaintext (0x33) block reads; `plain` selects the mode.
NfcCommand
    mf_plus_listener_read_handler(MfPlusListener* instance, const BitBuffer* rx, bool plain);

// Handles both encrypted (0xA1, keys) and plaintext (0xA3, data) writes; `encrypted` selects the
// mode. Mutates MfPlusData in place so the emulate-exit diff persists the change to the .shd.
NfcCommand
    mf_plus_listener_write_handler(MfPlusListener* instance, const BitBuffer* rx, bool encrypted);

NfcCommand mf_plus_listener_read_signature_handler(MfPlusListener* instance, const BitBuffer* rx);

// Answers the SL0-only WritePerso (0xA8) with the SL3 "command not available" status, so a reader's
// info probe (e.g. PM3 `hf mfp info`) identifies the emulated card as MIFARE Plus in SL3.
NfcCommand mf_plus_listener_write_perso_handler(MfPlusListener* instance, const BitBuffer* rx);

// NAKs any unimplemented command with 0x0B, mirroring a real card (which answers rather than going
// silent) so a reader's info scan completes instead of timing out on each unknown command.
NfcCommand mf_plus_listener_unsupported_handler(MfPlusListener* instance, const BitBuffer* rx);

// Emulate GetVersion by replaying the stored MfPlusVersion as the chained 0x60/0xAF response, native
// (status leads the frame) or ISO7816-wrapped (data leads, 91<status> trailer). Only EV1/EV2 answer;
// an EV0 part (S/X/SE) has an all-zero version and NAKs 0x0B like the real silicon. The _handler
// serves the first (HW) frame; _continue serves the SW and final (UID/batch/date) frames on 0xAF.
NfcCommand mf_plus_listener_get_version_handler(MfPlusListener* instance, bool wrapped);

NfcCommand mf_plus_listener_get_version_continue_handler(MfPlusListener* instance, bool wrapped);

// Reset the in-flight auth and any established session (on field-off / halt / a fresh AUTH_FIRST).
void mf_plus_listener_reset_session(MfPlusListener* instance);

#ifdef __cplusplus
}
#endif
