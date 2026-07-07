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

#define MF_PLUS_CONFIG_BLOCK_HIGH (0xB0) // config blocks live at 0xB000..0xB003

// Advance the read scan to the next (sector, key type): key A -> key B for the same sector, then
// on to the next sector; once every sector has been visited, move on to the admin-key phase.
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
        instance->current_admin = 0;
        instance->state = MfPlusPollerStateRequestAdminKey;
    } else {
        instance->state = MfPlusPollerStateRequestKey;
    }
}

// Advance to the next admin key; finish the read once all have been attempted.
static void mf_plus_poller_admin_advance(MfPlusPoller* instance) {
    instance->current_admin++;
    if(instance->current_admin >= MfPlusAdminKeyNum) {
        instance->state = MfPlusPollerStateReadSuccess;
    } else {
        instance->state = MfPlusPollerStateRequestAdminKey;
    }
}

static NfcCommand mf_plus_poller_handler_request_mode(MfPlusPoller* instance) {
    furi_assert(instance);

    instance->mfp_event.data->mode_request.mode = MfPlusPollerModeInfo;
    instance->mfp_event.type = MfPlusPollerEventTypeRequestMode;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->mode = instance->mfp_event.data->mode_request.mode;

    // Secure-messaging flows only make sense on an SL3 card; otherwise just report identity.
    if(instance->data->security_level == MfPlusSecurityLevel3 &&
       instance->mode == MfPlusPollerModeRead) {
        instance->state = MfPlusPollerStateReadSignature;
    } else if(
        instance->data->security_level == MfPlusSecurityLevel3 &&
        instance->mode == MfPlusPollerModeWrite) {
        instance->current_sector = 0;
        instance->current_block = 0;
        instance->write_plain = false;
        instance->write_mode_locked = false;
        instance->state = MfPlusPollerStateRequestWriteSector;
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
    instance->read_plain = false;
    instance->read_mode_locked = false;
    instance->current_admin = 0;
    instance->admin_keys_found = 0;
    instance->current_config_block = 0;
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
    instance->mfp_event.data->key_request.is_admin = false;
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
    MfPlusError error = mf_plus_poller_read_block(
        instance,
        (uint8_t)instance->current_block,
        0x00,
        instance->read_plain,
        &instance->session,
        &block);

    // Read-mode probe. Some SL3 cards require plaintext (0x33) reads instead of encrypted (0x31).
    // Until a block has been read successfully (read_mode_locked), a status rejection (Rejected, a
    // bare status frame that leaves r_ctr in sync) means "wrong mode": re-authenticate this sector
    // and retry the SAME block in the other mode, in-line. Only a success adopts the other mode, so
    // a block that both modes refuse is treated as unreadable (not a mode signal) and skipped with
    // the mode unchanged -- the next block re-probes. A transient comms fault (Protocol/Timeout) is
    // NOT a Rejected, so it never flips the mode.
    if(error == MfPlusErrorRejected && !instance->read_mode_locked) {
        const bool other_plain = !instance->read_plain;
        FURI_LOG_D(
            TAG,
            "Read refused on block %u; probing %s mode",
            instance->current_block,
            other_plain ? "plain" : "encrypted");
        MfPlusError reauth = mf_plus_poller_authenticate(
            instance,
            sector,
            instance->current_key_type,
            &instance->current_key,
            &instance->session);
        if(reauth != MfPlusErrorNone) {
            error = reauth;
        } else {
            error = mf_plus_poller_read_block(
                instance,
                (uint8_t)instance->current_block,
                0x00,
                other_plain,
                &instance->session,
                &block);
            if(error == MfPlusErrorNone) {
                instance->read_plain = other_plain; // the other mode works -> adopt it card-wide
            }
        }
    }

    bool sector_complete;
    if(error == MfPlusErrorNone) {
        instance->read_mode_locked = true; // a successful read pins the card-wide read mode
        mf_plus_set_block_read(instance->data, instance->current_block, &block);
        instance->current_block++;
        sector_complete = instance->current_block >= end_block;
    } else if(error == MfPlusErrorNotPresent || error == MfPlusErrorTimeout) {
        instance->error = error;
        instance->state = MfPlusPollerStateReadFailed;
        return NfcCommandContinue;
    } else if(error == MfPlusErrorAuth) {
        // A response-MAC mismatch (or a failed probe re-auth) means r_ctr is desynced, so the rest
        // of this sector can't be read reliably. Abort it (the next sector re-authenticates and
        // resets the counter).
        FURI_LOG_W(
            TAG, "Block %u auth failure; aborting sector %u", instance->current_block, sector);
        sector_complete = true;
    } else {
        // Access-denied / transient / refused-in-both-modes: leave this block unread and try the
        // next one. The mode is left unchanged (and unlocked if still probing).
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

// True while any 0xB0xx config block is still uncaptured. A CCK pass can fill blocks the CMK pass
// could not read (config blocks are owned by different keys), so config is tracked by this mask,
// not a one-shot flag.
static bool mf_plus_poller_config_incomplete(const MfPlusPoller* instance) {
    for(uint8_t c = 0; c < MF_PLUS_CONFIG_BLOCK_NUM; c++) {
        if(!mf_plus_is_config_block_read(instance->data, c)) return true;
    }
    return false;
}

static NfcCommand mf_plus_poller_handler_request_admin_key(MfPlusPoller* instance) {
    furi_assert(instance);

    // Same app-driven dictionary loop as sectors, but the target is an admin key (0x90xx). The app
    // must eventually answer key_provided = false so the scan advances to the next admin key.
    instance->mfp_event.data->key_request.is_admin = true;
    instance->mfp_event.data->key_request.admin_type = (MfPlusAdminKeyType)instance->current_admin;
    instance->mfp_event.data->key_request.key_provided = false;
    instance->mfp_event.type = MfPlusPollerEventTypeRequestKey;
    NfcCommand command = instance->callback(instance->general_event, instance->context);

    if(instance->mfp_event.data->key_request.key_provided) {
        instance->current_key = instance->mfp_event.data->key_request.key;
        instance->state = MfPlusPollerStateAuthAdminKey;
    } else {
        mf_plus_poller_admin_advance(instance);
    }
    return command;
}

static NfcCommand mf_plus_poller_handler_auth_admin_key(MfPlusPoller* instance) {
    furi_assert(instance);

    const MfPlusAdminKeyType admin_type = (MfPlusAdminKeyType)instance->current_admin;
    MfPlusError error = mf_plus_poller_authenticate_key_id(
        instance,
        mf_plus_get_admin_key_address(admin_type),
        &instance->current_key,
        &instance->session);

    if(error == MfPlusErrorNone) {
        mf_plus_set_admin_key_found(instance->data, admin_type, &instance->current_key);
        instance->admin_keys_found++;
        // The Card Master / Config key unlocks the 0xB0xx config blocks; read them now while this
        // session is fresh (any later admin auth resets it).
        const bool unlocks_config =
            (admin_type == MfPlusAdminKeyCardMaster || admin_type == MfPlusAdminKeyCardConfig);
        if(unlocks_config && mf_plus_poller_config_incomplete(instance)) {
            instance->current_config_block = 0;
            instance->state = MfPlusPollerStateReadConfig;
        } else {
            mf_plus_poller_admin_advance(instance);
        }
    } else if(error == MfPlusErrorAuth) {
        // Wrong candidate -> ask for the next dictionary candidate for this same admin key.
        instance->state = MfPlusPollerStateRequestAdminKey;
    } else if(error == MfPlusErrorNotPresent || error == MfPlusErrorTimeout) {
        // The card is gone. Every sector was already captured before the admin phase, so finish
        // with what we have rather than downgrading an otherwise-complete read to a failure.
        FURI_LOG_D(TAG, "Card lost during admin phase (error %d)", error);
        instance->state = MfPlusPollerStateReadSuccess;
    } else {
        // The card rejected AUTH_FIRST for this address: the slot is not exposed on this card (a
        // present sector/admin slot always answers AUTH_FIRST regardless of the candidate). Skip
        // the whole key -- re-probing it with every remaining dict candidate is pointless -- and
        // keep the read; admin keys are best-effort bonus that must not fail a complete read.
        FURI_LOG_D(TAG, "Admin key %u unavailable (error %d)", instance->current_admin, error);
        mf_plus_poller_admin_advance(instance);
    }
    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_read_config(MfPlusPoller* instance) {
    furi_assert(instance);

    // Skip blocks already captured (e.g. by the CMK pass before this CCK pass).
    while(instance->current_config_block < MF_PLUS_CONFIG_BLOCK_NUM &&
          mf_plus_is_config_block_read(instance->data, instance->current_config_block)) {
        instance->current_config_block++;
    }
    if(instance->current_config_block >= MF_PLUS_CONFIG_BLOCK_NUM) {
        mf_plus_poller_admin_advance(instance);
        return NfcCommandContinue;
    }

    MfPlusBlock block;
    // Config blocks (0xB0xx) are always read encrypted via the CMK/CCK session.
    MfPlusError error = mf_plus_poller_read_block(
        instance,
        instance->current_config_block,
        MF_PLUS_CONFIG_BLOCK_HIGH,
        false,
        &instance->session,
        &block);

    if(error == MfPlusErrorNone) {
        mf_plus_set_config_block_read(instance->data, instance->current_config_block, &block);
        instance->current_config_block++;
    } else if(error == MfPlusErrorNotPresent || error == MfPlusErrorTimeout) {
        // Card gone during the bonus config read: finish with the (already complete) sector data
        // rather than failing the read.
        FURI_LOG_D(TAG, "Card lost during config read (error %d)", error);
        instance->state = MfPlusPollerStateReadSuccess;
    } else if(error == MfPlusErrorAuth) {
        // MAC mismatch desyncs r_ctr: this session can't read the rest. Abort the pass WITHOUT
        // marking config done, so a subsequent CMK/CCK auth re-authenticates and retries.
        FURI_LOG_W(
            TAG,
            "Config block %u MAC mismatch; aborting config pass",
            instance->current_config_block);
        mf_plus_poller_admin_advance(instance);
    } else {
        // Access-denied (a block this key does not own) or transient: leave it unread and try the
        // next, same as an unreadable data block. Config is best-effort and never fails the read.
        FURI_LOG_D(
            TAG, "Skipping config block %u (error %d)", instance->current_config_block, error);
        instance->current_config_block++;
    }
    return NfcCommandContinue;
}

/* ---- SL3 write flow ("Write to Initial Card") ---- */

// Advance the write scan to the next sector.
static void mf_plus_poller_write_advance(MfPlusPoller* instance) {
    instance->current_sector++;
    instance->state = MfPlusPollerStateRequestWriteSector;
}

static NfcCommand mf_plus_poller_handler_request_write_sector(MfPlusPoller* instance) {
    furi_assert(instance);

    if(instance->current_sector >= mf_plus_get_sector_count(instance->data->size)) {
        instance->state = MfPlusPollerStateWriteSuccess;
        return NfcCommandContinue;
    }

    // Ask the app for the recovered AES key of this sector (from the loaded dump).
    instance->mfp_event.data->write_sector_request.sector = instance->current_sector;
    instance->mfp_event.data->write_sector_request.key_provided = false;
    instance->mfp_event.type = MfPlusPollerEventTypeRequestWriteSector;
    NfcCommand command = instance->callback(instance->general_event, instance->context);

    if(instance->mfp_event.data->write_sector_request.key_provided) {
        instance->current_key = instance->mfp_event.data->write_sector_request.key;
        instance->current_key_type = instance->mfp_event.data->write_sector_request.key_type;
        instance->state = MfPlusPollerStateAuthWriteSector;
    } else {
        // No recovered key for this sector -> it can't be authenticated to write. Skip it.
        mf_plus_poller_write_advance(instance);
    }
    return command;
}

static NfcCommand mf_plus_poller_handler_auth_write_sector(MfPlusPoller* instance) {
    furi_assert(instance);

    MfPlusError error = mf_plus_poller_authenticate(
        instance,
        instance->current_sector,
        instance->current_key_type,
        &instance->current_key,
        &instance->session);

    if(error == MfPlusErrorNone) {
        instance->current_block = mf_plus_sector_get_first_block(instance->current_sector);
        instance->state = MfPlusPollerStateRequestWriteBlock;
    } else if(error == MfPlusErrorAuth) {
        // The supplied key (which the dump recovered) is rejected by the source card: something is
        // wrong (wrong card slipped past the UID check, or a re-keyed sector). Fail the write.
        FURI_LOG_W(TAG, "Write auth failed at sector %u", instance->current_sector);
        instance->error = error;
        instance->state = MfPlusPollerStateWriteFailed;
    } else {
        instance->error = error;
        instance->state = MfPlusPollerStateWriteFailed;
    }
    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_request_write_block(MfPlusPoller* instance) {
    furi_assert(instance);

    const uint16_t end_block = mf_plus_sector_get_first_block(instance->current_sector) +
                               mf_plus_sector_get_block_count(instance->current_sector);
    if(instance->current_block >= end_block) {
        mf_plus_poller_write_advance(instance);
        return NfcCommandContinue;
    }

    instance->mfp_event.data->write_block_request.block_num = instance->current_block;
    instance->mfp_event.data->write_block_request.block_provided = false;
    instance->mfp_event.type = MfPlusPollerEventTypeRequestWriteBlock;
    NfcCommand command = instance->callback(instance->general_event, instance->context);

    if(instance->mfp_event.data->write_block_request.block_provided) {
        instance->current_write_block = instance->mfp_event.data->write_block_request.block;
        instance->state = MfPlusPollerStateWriteBlock;
    } else {
        // Block not captured in the dump -> nothing to write here; skip it.
        instance->current_block++;
    }
    return command;
}

static NfcCommand mf_plus_poller_handler_write_block(MfPlusPoller* instance) {
    furi_assert(instance);

    MfPlusError error = mf_plus_poller_write_block(
        instance,
        (uint8_t)instance->current_block,
        0x00,
        instance->write_plain,
        &instance->current_write_block,
        &instance->session);

    // Write-mode probe, mirroring the read handler. Until a block has been written successfully
    // (write_mode_locked), a card status rejection (Rejected) means "wrong mode": re-authenticate
    // this sector and retry the SAME block in the other mode, in-line. The other mode is adopted
    // only on success, so a block refused in both modes is treated as access-denied (not a mode
    // signal) and skipped with the mode unchanged. A transient comms fault never flips the mode.
    if(error == MfPlusErrorRejected && !instance->write_mode_locked) {
        const bool other_plain = !instance->write_plain;
        FURI_LOG_D(
            TAG,
            "Write refused on block %u; probing %s mode",
            instance->current_block,
            other_plain ? "plain" : "encrypted");
        MfPlusError reauth = mf_plus_poller_authenticate(
            instance,
            instance->current_sector,
            instance->current_key_type,
            &instance->current_key,
            &instance->session);
        if(reauth != MfPlusErrorNone) {
            error = reauth;
        } else {
            error = mf_plus_poller_write_block(
                instance,
                (uint8_t)instance->current_block,
                0x00,
                other_plain,
                &instance->current_write_block,
                &instance->session);
            if(error == MfPlusErrorNone) {
                instance->write_plain = other_plain; // the other mode works -> adopt it card-wide
            }
        }
    }

    if(error == MfPlusErrorNone) {
        instance->write_mode_locked = true; // a successful write pins the card-wide write mode
        instance->current_block++;
        instance->state = MfPlusPollerStateRequestWriteBlock;
    } else if(error == MfPlusErrorNotPresent || error == MfPlusErrorTimeout) {
        instance->error = error;
        instance->state = MfPlusPollerStateWriteFailed;
    } else if(error == MfPlusErrorAuth) {
        // Response-MAC mismatch: the session crypto is desynced, so the rest can't be trusted.
        FURI_LOG_W(TAG, "Write block %u MAC desync; failing", instance->current_block);
        instance->error = error;
        instance->state = MfPlusPollerStateWriteFailed;
    } else {
        // Access-denied / refused-in-both-modes / transient: leave this block and try the next.
        FURI_LOG_D(TAG, "Skipping write of block %u (error %d)", instance->current_block, error);
        instance->current_block++;
        instance->state = MfPlusPollerStateRequestWriteBlock;
    }
    return NfcCommandContinue;
}

static NfcCommand mf_plus_poller_handler_write_failed(MfPlusPoller* instance) {
    furi_assert(instance);

    FURI_LOG_D(TAG, "Write failed");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);

    instance->mfp_event.type = MfPlusPollerEventTypeWriteFailed;
    instance->mfp_event.data->error = instance->error;
    NfcCommand command = instance->callback(instance->general_event, instance->context);
    instance->state = MfPlusPollerStateIdle;

    return command;
}

static NfcCommand mf_plus_poller_handler_write_success(MfPlusPoller* instance) {
    furi_assert(instance);

    FURI_LOG_D(TAG, "Write success");
    iso14443_4a_poller_halt(instance->iso14443_4a_poller);

    instance->mfp_event.type = MfPlusPollerEventTypeWriteSuccess;
    NfcCommand command = instance->callback(instance->general_event, instance->context);

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
    [MfPlusPollerStateRequestAdminKey] = mf_plus_poller_handler_request_admin_key,
    [MfPlusPollerStateAuthAdminKey] = mf_plus_poller_handler_auth_admin_key,
    [MfPlusPollerStateReadConfig] = mf_plus_poller_handler_read_config,
    [MfPlusPollerStateReadFailed] = mf_plus_poller_handler_read_failed,
    [MfPlusPollerStateReadSuccess] = mf_plus_poller_handler_read_success,
    [MfPlusPollerStateRequestWriteSector] = mf_plus_poller_handler_request_write_sector,
    [MfPlusPollerStateAuthWriteSector] = mf_plus_poller_handler_auth_write_sector,
    [MfPlusPollerStateRequestWriteBlock] = mf_plus_poller_handler_request_write_block,
    [MfPlusPollerStateWriteBlock] = mf_plus_poller_handler_write_block,
    [MfPlusPollerStateWriteFailed] = mf_plus_poller_handler_write_failed,
    [MfPlusPollerStateWriteSuccess] = mf_plus_poller_handler_write_success,
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
        // The ISO14443-4a layer faulted (card removed / RF error); surface a defined error code so
        // consumers can log it rather than a stale union value. Report it as a write fault in write
        // mode so the write scene shows its failure UI (else the read failure path).
        instance->error = MfPlusErrorTimeout;
        instance->mfp_event.data->error = instance->error;
        instance->mfp_event.type = (instance->mode == MfPlusPollerModeWrite) ?
                                       MfPlusPollerEventTypeWriteFailed :
                                       MfPlusPollerEventTypeReadFailed;
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
