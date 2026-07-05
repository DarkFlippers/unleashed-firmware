#include "mf_plus_poller_i.h"
#include "mf_plus_i.h"

#include <nfc/protocols/nfc_poller_base.h>

#include <furi.h>

#define TAG "MfPlusPoller"

#define MF_PLUS_BUF_SIZE        (64U)
#define MF_PLUS_RESULT_BUF_SIZE (512U)

typedef NfcCommand (*MfPlusPollerReadHandler)(MfPlusPoller* instance);

const MfPlusData* mf_plus_poller_get_data(MfPlusPoller* instance) {
    furi_assert(instance);

    return instance->data;
}

MfPlusPoller* mf_plus_poller_alloc(Iso14443_4aPoller* iso14443_4a_poller) {
    furi_assert(iso14443_4a_poller);

    MfPlusPoller* instance = malloc(sizeof(MfPlusPoller));

    instance->iso14443_4a_poller = iso14443_4a_poller;

    instance->data = mf_plus_alloc();

    instance->tx_buffer = bit_buffer_alloc(MF_PLUS_BUF_SIZE);
    instance->rx_buffer = bit_buffer_alloc(MF_PLUS_BUF_SIZE);
    instance->input_buffer = bit_buffer_alloc(MF_PLUS_BUF_SIZE);
    instance->result_buffer = bit_buffer_alloc(MF_PLUS_RESULT_BUF_SIZE);

    instance->general_event.protocol = NfcProtocolMfPlus;
    instance->general_event.event_data = &instance->mfp_event;
    instance->general_event.instance = instance;

    instance->mfp_event.data = &instance->mfp_event_data;

    return instance;
}

// Resolve the SL for a SAK-0x20 card via the active probe (the only case SL0/SL3/DESFire can't be
// told apart passively — see MfPlusProbeResult). Returns Unknown for any other SAK, a probe error,
// or a non-Plus reply, so callers keep their DESFire-safe fallback.
static MfPlusSecurityLevel mf_plus_poller_resolve_sak20_security_level(MfPlusPoller* instance) {
    const Iso14443_4aData* iso14443_4a_data =
        iso14443_4a_poller_get_data(instance->iso14443_4a_poller);
    if(iso14443_4a_data->iso14443_3a_data->sak != 0x20) {
        return MfPlusSecurityLevelUnknown;
    }

    MfPlusProbeResult probe = MfPlusProbeResultNotPlus;
    if(mf_plus_poller_probe_security_level(instance, &probe) != MfPlusErrorNone) {
        return MfPlusSecurityLevelUnknown;
    }

    switch(probe) {
    case MfPlusProbeResultSl0:
        return MfPlusSecurityLevel0;
    case MfPlusProbeResultSl3:
        return MfPlusSecurityLevel3;
    default: // MfPlusProbeResultNotPlus
        return MfPlusSecurityLevelUnknown;
    }
}

static NfcCommand mf_plus_poller_handler_idle(MfPlusPoller* instance) {
    furi_assert(instance);

    bit_buffer_reset(instance->input_buffer);
    bit_buffer_reset(instance->result_buffer);
    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    iso14443_4a_copy(
        instance->data->iso14443_4a_data,
        iso14443_4a_poller_get_data(instance->iso14443_4a_poller));

    instance->state = MfPlusPollerStateReadVersion;
    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_read_version(MfPlusPoller* instance) {
    MfPlusError error = mf_plus_poller_read_version(instance, &instance->data->version);
    if(error == MfPlusErrorNone) {
        instance->state = MfPlusPollerStateParseVersion;
    } else {
        instance->state = MfPlusPollerStateParseIso4;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_parse_version(MfPlusPoller* instance) {
    furi_assert(instance);

    const MfPlusSecurityLevel probed_sl = mf_plus_poller_resolve_sak20_security_level(instance);
    MfPlusError error = mf_plus_get_type_from_version(
        iso14443_4a_poller_get_data(instance->iso14443_4a_poller), probed_sl, instance->data);
    if(error == MfPlusErrorNone) {
        instance->state = MfPlusPollerStateRequestMode;
    } else {
        instance->error = error;
        instance->state = MfPlusPollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_parse_iso4(MfPlusPoller* instance) {
    furi_assert(instance);

    const MfPlusSecurityLevel probed_sl = mf_plus_poller_resolve_sak20_security_level(instance);
    MfPlusError error = mf_plus_get_type_from_iso4(
        iso14443_4a_poller_get_data(instance->iso14443_4a_poller), probed_sl, instance->data);
    if(error == MfPlusErrorNone) {
        instance->state = MfPlusPollerStateRequestMode;
    } else {
        instance->error = error;
        instance->state = MfPlusPollerStateReadFailed;
    }

    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_read_failed(MfPlusPoller* instance) {
    furi_assert(instance);

    FURI_LOG_D(TAG, "Read failed");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);

    instance->mfp_event.type = MfPlusPollerEventTypeReadFailed;
    instance->mfp_event.data->error = instance->error;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->state = MfPlusPollerStateIdle;

    return command;
}

static NfcCommand mf_plus_poller_handler_read_success(MfPlusPoller* instance) {
    furi_assert(instance);

    FURI_LOG_D(TAG, "Read success");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);

    instance->mfp_event.type = MfPlusPollerEventTypeReadSuccess;
    NfcCommand command = instance->callback(instance->general_event, instance->context);

    return command;
}

// Advance the read scan to the next (sector, key type): key A -> key B for the same sector, then
// on to the next sector; finish once every sector has been visited.
static void mf_plus_poller_read_advance(MfPlusPoller* instance) {
    if(instance->current_key_type == MfPlusKeyTypeA) {
        instance->current_key_type = MfPlusKeyTypeB;
        instance->state = MfPlusPollerStateRequestKey;
        return;
    }

    instance->current_sector++;
    instance->current_key_type = MfPlusKeyTypeA;
    instance->sector_blocks_read = false;
    if(instance->current_sector >= mf_plus_get_sector_count(instance->data->size)) {
        instance->state = MfPlusPollerStateReadSuccess;
    } else {
        instance->state = MfPlusPollerStateRequestKey;
    }
}

static NfcCommand mf_plus_poller_handler_request_mode(MfPlusPoller* instance) {
    furi_assert(instance);

    instance->mfp_event.data->mode_request.mode = MfPlusPollerModeInfo;
    instance->mfp_event.type = MfPlusPollerEventTypeRequestMode;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->mode = instance->mfp_event.data->mode_request.mode;

    // A full SL3 read only makes sense on an SL3 card in read mode; otherwise just report identity.
    if(instance->mode == MfPlusPollerModeRead &&
       instance->data->security_level == MfPlusSecurityLevel3) {
        instance->state = MfPlusPollerStateReadSignature;
    } else {
        instance->state = MfPlusPollerStateReadSuccess;
    }
    return command;
}

static NfcCommand mf_plus_poller_handler_read_signature(MfPlusPoller* instance) {
    furi_assert(instance);

    // Best-effort: a card without (or rejecting) an originality signature must not fail the read.
    bool present = false;
    MfPlusError error =
        mf_plus_poller_read_signature(instance, instance->data->signature, &present);
    instance->data->signature_present = present;
    if(error != MfPlusErrorNone) {
        FURI_LOG_D(TAG, "Signature read error %d (ignored)", error);
    } else if(!present) {
        FURI_LOG_D(TAG, "Card has no originality signature");
    }

    instance->current_sector = 0;
    instance->current_key_type = MfPlusKeyTypeA;
    instance->sector_blocks_read = false;
    instance->sectors_read = 0;
    instance->keys_found = 0;
    // Guard a (spurious) 0-sector card so RequestKey isn't entered for a nonexistent sector.
    instance->state = (mf_plus_get_sector_count(instance->data->size) == 0) ?
                          MfPlusPollerStateReadSuccess :
                          MfPlusPollerStateRequestKey;
    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_request_key(MfPlusPoller* instance) {
    furi_assert(instance);

    // Key iteration is delegated to the app: it must eventually answer key_provided = false so the
    // scan advances (there is no poller-side attempt cap). Mirrors the mf_classic dict-attack loop.
    instance->mfp_event.data->key_request.sector = instance->current_sector;
    instance->mfp_event.data->key_request.key_type = instance->current_key_type;
    instance->mfp_event.data->key_request.key_provided = false;
    instance->mfp_event.type = MfPlusPollerEventTypeRequestKey;
    NfcCommand command = instance->callback(instance->general_event, instance->context);

    if(instance->mfp_event.data->key_request.key_provided) {
        instance->current_key = instance->mfp_event.data->key_request.key;
        instance->state = MfPlusPollerStateAuthSector;
    } else {
        // No (more) candidate keys for this sector/type.
        mf_plus_poller_read_advance(instance);
    }
    return command;
}

static NfcCommand mf_plus_poller_handler_auth_sector(MfPlusPoller* instance) {
    furi_assert(instance);

    MfPlusError error = mf_plus_poller_authenticate(
        instance,
        instance->current_sector,
        instance->current_key_type,
        &instance->current_key,
        &instance->session);

    if(error == MfPlusErrorNone) {
        mf_plus_set_key_found(
            instance->data,
            instance->current_sector,
            instance->current_key_type,
            &instance->current_key);
        instance->keys_found++;

        if(instance->sector_blocks_read) {
            // Blocks already captured via the other key -> just record this key and move on.
            mf_plus_poller_read_advance(instance);
        } else {
            instance->current_block = mf_plus_sector_get_first_block(instance->current_sector);
            instance->state = MfPlusPollerStateReadSectorBlocks;
        }
    } else if(error == MfPlusErrorAuth) {
        // The card rejected our token: a genuine wrong key -> ask the app for the next candidate.
        instance->state = MfPlusPollerStateRequestKey;
    } else {
        // A comms/RF fault (or lost card) is NOT a wrong key: fail loudly rather than silently
        // dropping a possibly-correct key and leaving the sector unread.
        FURI_LOG_W(TAG, "Auth error %d at sector %u", error, instance->current_sector);
        instance->error = error;
        instance->state = MfPlusPollerStateReadFailed;
    }
    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_read_sector_blocks(MfPlusPoller* instance) {
    furi_assert(instance);

    const uint8_t sector = instance->current_sector;
    const uint16_t end_block =
        mf_plus_sector_get_first_block(sector) + mf_plus_sector_get_block_count(sector);

    MfPlusBlock block;
    MfPlusError error = mf_plus_poller_read_encrypted_block(
        instance, (uint8_t)instance->current_block, &instance->session, &block);

    bool sector_complete;
    if(error == MfPlusErrorNone) {
        mf_plus_set_block_read(instance->data, instance->current_block, &block);
        instance->current_block++;
        sector_complete = instance->current_block >= end_block;
    } else if(error == MfPlusErrorNotPresent || error == MfPlusErrorTimeout) {
        instance->error = error;
        instance->state = MfPlusPollerStateReadFailed;
        return NfcCommandContinue;
    } else if(error == MfPlusErrorAuth) {
        // Response MAC mismatch: r_ctr is desynced, so the rest of this sector can't be read
        // reliably. Abort it (the next sector re-authenticates and resets the counter).
        FURI_LOG_W(
            TAG, "Block %u MAC mismatch; aborting sector %u", instance->current_block, sector);
        sector_complete = true;
    } else {
        // Access-denied / transient on this block: leave it unread and try the next one.
        FURI_LOG_D(TAG, "Skipping block %u (error %d)", instance->current_block, error);
        instance->current_block++;
        sector_complete = instance->current_block >= end_block;
    }

    if(!sector_complete) {
        return NfcCommandContinue;
    }

    instance->sector_blocks_read = true;
    instance->sectors_read++;

    instance->mfp_event.data->data_update.current_sector = sector;
    instance->mfp_event.data->data_update.sectors_read = instance->sectors_read;
    instance->mfp_event.data->data_update.keys_found = instance->keys_found;
    instance->mfp_event.type = MfPlusPollerEventTypeDataUpdate;
    NfcCommand command = instance->callback(instance->general_event, instance->context);

    mf_plus_poller_read_advance(instance);
    return command;
}

static const MfPlusPollerReadHandler mf_plus_poller_read_handler[MfPlusPollerStateNum] = {
    [MfPlusPollerStateIdle] = mf_plus_poller_handler_idle,
    [MfPlusPollerStateReadVersion] = mf_plus_poller_handler_read_version,
    [MfPlusPollerStateParseVersion] = mf_plus_poller_handler_parse_version,
    [MfPlusPollerStateParseIso4] = mf_plus_poller_handler_parse_iso4,
    [MfPlusPollerStateRequestMode] = mf_plus_poller_handler_request_mode,
    [MfPlusPollerStateReadSignature] = mf_plus_poller_handler_read_signature,
    [MfPlusPollerStateRequestKey] = mf_plus_poller_handler_request_key,
    [MfPlusPollerStateAuthSector] = mf_plus_poller_handler_auth_sector,
    [MfPlusPollerStateReadSectorBlocks] = mf_plus_poller_handler_read_sector_blocks,
    [MfPlusPollerStateReadFailed] = mf_plus_poller_handler_read_failed,
    [MfPlusPollerStateReadSuccess] = mf_plus_poller_handler_read_success,
};

static void mf_plus_poller_set_callback(
    MfPlusPoller* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static NfcCommand mf_plus_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolIso14443_4a);
    furi_assert(event.event_data);

    MfPlusPoller* instance = context;
    const Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;

    NfcCommand command = NfcCommandContinue;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        command = mf_plus_poller_read_handler[instance->state](instance);
    } else if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeError) {
        instance->mfp_event.type = MfPlusPollerEventTypeReadFailed;
        command = instance->callback(instance->general_event, instance->context);
    }

    return command;
}

void mf_plus_poller_free(MfPlusPoller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    bit_buffer_free(instance->input_buffer);
    bit_buffer_free(instance->result_buffer);
    mf_plus_free(instance->data);
    free(instance);
}

static bool mf_plus_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolIso14443_4a);
    furi_assert(event.event_data);

    MfPlusPoller* instance = context;
    Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;

    MfPlusError error = MfPlusErrorUnknown;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        error = mf_plus_poller_read_version(instance, &instance->data->version);
        if(error == MfPlusErrorNone) {
            // Version path already excludes DESFire (family nibble) and the SL chosen here is
            // discarded with this throwaway detect poller, so skip the probe round-trip; the read
            // phase resolves the real SL0/SL3.
            error = mf_plus_get_type_from_version(
                iso14443_4a_poller_get_data(instance->iso14443_4a_poller),
                MfPlusSecurityLevelUnknown,
                instance->data);
        } else {
            // iso4 fallback: the probe is what confirms Plus-not-DESFire and lifts an untabled ATS
            // out of Unknown, so it is required for detection coverage here.
            const MfPlusSecurityLevel probed_sl =
                mf_plus_poller_resolve_sak20_security_level(instance);
            error = mf_plus_get_type_from_iso4(
                iso14443_4a_poller_get_data(instance->iso14443_4a_poller),
                probed_sl,
                instance->data);
        }
    }

    return error == MfPlusErrorNone;
}

const NfcPollerBase mf_plus_poller = {
    .alloc = (NfcPollerAlloc)mf_plus_poller_alloc,
    .free = (NfcPollerFree)mf_plus_poller_free,
    .set_callback = (NfcPollerSetCallback)mf_plus_poller_set_callback,
    .run = (NfcPollerRun)mf_plus_poller_run,
    .detect = (NfcPollerDetect)mf_plus_poller_detect,
    .get_data = (NfcPollerGetData)mf_plus_poller_get_data,
};
