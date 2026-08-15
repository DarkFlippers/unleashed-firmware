#include "../nfc_app_i.h"
#include <dolphin/dolphin.h>

#include <nfc/protocols/mf_plus/mf_plus_poller.h>

#define TAG "NfcMfPlusDictAttack"

// The MIFARE Plus SL3 poller visits every sector once, asking the app for candidate keys per
// (sector, key type). Unlike MIFARE Classic it has no resume-from-prior-data path, so both the
// user and the built-in system dictionary are consumed within a SINGLE poller pass (user keys
// first) — restarting the poller between dictionaries would discard keys already recovered.

// Yield the next candidate key for the current (sector, key type): user dictionary first, then the
// system dictionary. Returns false only once both are exhausted for this request.
static bool
    nfc_scene_mf_plus_dict_attack_next_key(NfcMfPlusDictAttackContext* ctx, MfPlusKey* key) {
    if(!ctx->on_system_dict && ctx->user_dict != NULL) {
        if(keys_dict_get_next_key(ctx->user_dict, key->data, sizeof(MfPlusKey))) {
            return true;
        }
        // User dictionary exhausted for this request; continue with the system dictionary.
        ctx->on_system_dict = true;
    }

    if(ctx->system_dict != NULL) {
        return keys_dict_get_next_key(ctx->system_dict, key->data, sizeof(MfPlusKey));
    }

    return false;
}

NfcCommand nfc_mf_plus_dict_attack_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfPlus);

    NfcCommand command = NfcCommandContinue;
    NfcApp* instance = context;
    MfPlusPollerEvent* mfp_event = event.event_data;
    NfcMfPlusDictAttackContext* ctx = &instance->mf_plus_dict_context;

    if(mfp_event->type == MfPlusPollerEventTypeRequestMode) {
        mfp_event->data->mode_request.mode = MfPlusPollerModeRead;
        // The poller has parsed the version/type by now, so the card geometry is known.
        const MfPlusData* data = nfc_poller_get_data(instance->poller);
        ctx->sectors_total = mf_plus_get_sector_count(data->size);
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfp_event->type == MfPlusPollerEventTypeRequestKey) {
        const bool is_admin = mfp_event->data->key_request.is_admin;
        const uint8_t sector = mfp_event->data->key_request.sector;
        const uint8_t key_type = mfp_event->data->key_request.key_type;
        const uint8_t admin_type = mfp_event->data->key_request.admin_type;

        // A change of target (sector key A/B, or which admin key) since the last request means the
        // poller advanced; restart the combined key stream from the top of the user dictionary.
        bool target_changed;
        if(!ctx->request_seen || is_admin != ctx->last_is_admin) {
            target_changed = true;
        } else if(is_admin) {
            target_changed = admin_type != ctx->last_admin_type;
        } else {
            target_changed = sector != ctx->last_sector || key_type != ctx->last_key_type;
        }
        if(target_changed) {
            if(ctx->user_dict != NULL) keys_dict_rewind(ctx->user_dict);
            if(ctx->system_dict != NULL) keys_dict_rewind(ctx->system_dict);
            ctx->on_system_dict = (ctx->user_dict == NULL);
            ctx->dict_keys_current = 0;
            ctx->cache_key_fed = false;
            ctx->request_seen = true;
            ctx->last_is_admin = is_admin;
            ctx->last_sector = sector;
            ctx->last_key_type = key_type;
            ctx->last_admin_type = admin_type;
        }
        // Admin keys aren't sectors; leave the sector display untouched during the admin phase.
        if(!is_admin) ctx->current_sector = sector;

        MfPlusKey key = {};
        bool cache_hit = false;
        // Offer the cached key for this target first, once. Cache hits don't advance the dictionary
        // progress counter -- they aren't part of dict_keys_total. If the cached key fails to
        // authenticate the poller re-requests the same target, and cache_key_fed then routes it to
        // the dictionaries (covers a re-keyed card).
        if(!ctx->cache_key_fed) {
            ctx->cache_key_fed = true;
            cache_hit =
                is_admin ?
                    mf_plus_key_cache_get_admin_key(ctx->key_cache, admin_type, &key) :
                    mf_plus_key_cache_get_sector_key(ctx->key_cache, sector, key_type, &key);
        }
        if(cache_hit) {
            mfp_event->data->key_request.key = key;
            mfp_event->data->key_request.key_provided = true;
        } else if(nfc_scene_mf_plus_dict_attack_next_key(ctx, &key)) {
            mfp_event->data->key_request.key = key;
            mfp_event->data->key_request.key_provided = true;
            ctx->dict_keys_current++;
            if(ctx->dict_keys_current % 10 == 0) {
                view_dispatcher_send_custom_event(
                    instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
            }
        } else {
            mfp_event->data->key_request.key_provided = false;
        }
    } else if(mfp_event->type == MfPlusPollerEventTypeDataUpdate) {
        ctx->current_sector = mfp_event->data->data_update.current_sector;
        ctx->sectors_read = mfp_event->data->data_update.sectors_read;
        ctx->keys_found = mfp_event->data->data_update.keys_found;
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackDataUpdate);
    } else if(mfp_event->type == MfPlusPollerEventTypeReadSuccess) {
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventDictAttackComplete);
        command = NfcCommandStop;
    } else if(mfp_event->type == MfPlusPollerEventTypeReadFailed) {
        // A lost card or comms fault is NOT the same as an incomplete dictionary: log it and route
        // to a distinct failure path so the scan is never graded as a clean read. The finish
        // handler still preserves whatever was recovered before the fault.
        FURI_LOG_W(
            TAG,
            "Read aborted (poller error %d) after %u/%u sectors",
            mfp_event->data->error,
            ctx->sectors_read,
            ctx->sectors_total);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
        command = NfcCommandStop;
    } else {
        FURI_LOG_E(TAG, "Unhandled poller event %d", mfp_event->type);
    }

    return command;
}

static void nfc_scene_mf_plus_dict_attack_result_callback(DictAttackEvent event, void* context) {
    furi_assert(context);
    NfcApp* instance = context;
    if(event == DictAttackEventSkipPressed) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventDictAttackSkip);
    }
}

static void nfc_scene_mf_plus_dict_attack_update_view(NfcApp* instance) {
    NfcMfPlusDictAttackContext* ctx = &instance->mf_plus_dict_context;
    dict_attack_set_sectors_total(instance->dict_attack, ctx->sectors_total);
    dict_attack_set_sectors_read(instance->dict_attack, ctx->sectors_read);
    dict_attack_set_keys_found(instance->dict_attack, ctx->keys_found);
    dict_attack_set_current_sector(instance->dict_attack, ctx->current_sector);
    dict_attack_set_current_dict_key(instance->dict_attack, ctx->dict_keys_current);
}

static void nfc_scene_mf_plus_dict_attack_setup_dicts(NfcApp* instance) {
    NfcMfPlusDictAttackContext* ctx = &instance->mf_plus_dict_context;

    // The user dictionary is optional and lazily created by the key-management scenes; drop the
    // handle when the file is missing or holds no keys so the pass falls straight to the system one.
    if(keys_dict_check_presence(NFC_APP_MF_PLUS_DICT_USER_PATH)) {
        ctx->user_dict = keys_dict_alloc(
            NFC_APP_MF_PLUS_DICT_USER_PATH, KeysDictModeOpenAlways, sizeof(MfPlusKey));
        if(keys_dict_get_total_keys(ctx->user_dict) == 0) {
            keys_dict_free(ctx->user_dict);
            ctx->user_dict = NULL;
        }
    }
    // A missing system dictionary is not fatal (the attack just finds nothing), but it means a
    // broken install, so make it diagnosable instead of a silent "found no keys".
    if(!keys_dict_check_presence(NFC_APP_MF_PLUS_DICT_SYSTEM_PATH)) {
        FURI_LOG_W(TAG, "System dictionary %s is missing", NFC_APP_MF_PLUS_DICT_SYSTEM_PATH);
    }
    ctx->system_dict = keys_dict_alloc(
        NFC_APP_MF_PLUS_DICT_SYSTEM_PATH, KeysDictModeOpenExisting, sizeof(MfPlusKey));

    ctx->dict_keys_total =
        (ctx->user_dict != NULL ? keys_dict_get_total_keys(ctx->user_dict) : 0) +
        keys_dict_get_total_keys(ctx->system_dict);
    ctx->dict_keys_current = 0;
    ctx->on_system_dict = (ctx->user_dict == NULL);
    ctx->request_seen = false;

    // Per-UID key cache: if this exact card was saved before, its recovered keys authenticate on the
    // first try per sector, so the pass flies through instead of walking the dictionaries. A miss
    // (no cache file / different card) leaves the cache empty and the attack runs exactly as before.
    ctx->key_cache = mf_plus_key_cache_alloc();
    ctx->cache_key_fed = false;
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
    size_t uid_len = 0;
    const uint8_t* uid = mf_plus_get_uid(data, &uid_len);
    if(mf_plus_key_cache_load(ctx->key_cache, uid, uid_len)) {
        FURI_LOG_I(TAG, "Key cache hit; seeding dictionary attack with saved keys");
    }

    // The dict_attack view is shared; its type persists from whichever scene last used it (e.g. the
    // Ultralight-C scene never resets it), so set the sector-oriented layout explicitly on entry.
    dict_attack_set_type(instance->dict_attack, DictAttackTypeMfClassic);
    dict_attack_set_header(instance->dict_attack, "MF Plus Dictionary");
    dict_attack_set_total_dict_keys(instance->dict_attack, ctx->dict_keys_total);
    dict_attack_set_current_dict_key(instance->dict_attack, 0);
    dict_attack_set_callback(
        instance->dict_attack, nfc_scene_mf_plus_dict_attack_result_callback, instance);
}

void nfc_scene_mf_plus_dict_attack_on_enter(void* context) {
    NfcApp* instance = context;

    nfc_scene_mf_plus_dict_attack_setup_dicts(instance);
    nfc_scene_mf_plus_dict_attack_update_view(instance);
    dict_attack_set_card_state(instance->dict_attack, true);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDictAttack);
    nfc_blink_read_start(instance);

    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfPlus);
    nfc_poller_start(instance->poller, nfc_mf_plus_dict_attack_worker_callback, instance);
}

static void nfc_scene_mf_plus_dict_attack_finish(NfcApp* instance, bool aborted) {
    // Merge rather than replace: this poller is never seeded with the card we arrived with, so
    // adopting it outright drops anything this pass did not recover -- everything when Skip is
    // pressed before the poller even activates, which it can be from the first frame.
    MfPlusData* merged = mf_plus_alloc();
    nfc_device_copy_data(instance->nfc_device, NfcProtocolMfPlus, merged);
    mf_plus_merge_update(merged, nfc_poller_get_data(instance->poller));
    nfc_device_set_data(instance->nfc_device, NfcProtocolMfPlus, merged);
    mf_plus_free(merged);

    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
    // A clean pass that captured every block is a full success; an aborted scan (comms fault / lost
    // card) or a partial recovery is semi-success. mf_plus_is_card_read checks captured blocks, not
    // the poller's sector counter, which also counts sectors that aborted mid-read.
    bool fully_read = !aborted && mf_plus_is_card_read(data);
    notification_message(
        instance->notifications, fully_read ? &sequence_success : &sequence_semi_success);
    scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
    dolphin_deed(DolphinDeedNfcReadSuccess);
}

bool nfc_scene_mf_plus_dict_attack_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventDictAttackComplete ||
           event.event == NfcCustomEventDictAttackSkip) {
            nfc_scene_mf_plus_dict_attack_finish(instance, false);
            consumed = true;
        } else if(event.event == NfcCustomEventPollerFailure) {
            nfc_scene_mf_plus_dict_attack_finish(instance, true);
            consumed = true;
        } else if(event.event == NfcCustomEventDictAttackDataUpdate) {
            nfc_scene_mf_plus_dict_attack_update_view(instance);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }

    return consumed;
}

void nfc_scene_mf_plus_dict_attack_on_exit(void* context) {
    NfcApp* instance = context;
    NfcMfPlusDictAttackContext* ctx = &instance->mf_plus_dict_context;

    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    dict_attack_reset(instance->dict_attack);

    if(ctx->user_dict != NULL) {
        keys_dict_free(ctx->user_dict);
        ctx->user_dict = NULL;
    }
    if(ctx->system_dict != NULL) {
        keys_dict_free(ctx->system_dict);
        ctx->system_dict = NULL;
    }
    if(ctx->key_cache != NULL) {
        mf_plus_key_cache_free(ctx->key_cache);
        ctx->key_cache = NULL;
    }

    ctx->on_system_dict = false;
    ctx->sectors_total = 0;
    ctx->sectors_read = 0;
    ctx->current_sector = 0;
    ctx->keys_found = 0;
    ctx->dict_keys_total = 0;
    ctx->dict_keys_current = 0;
    ctx->request_seen = false;
    ctx->last_is_admin = false;
    ctx->last_sector = 0;
    ctx->last_key_type = 0;
    ctx->last_admin_type = 0;
    ctx->cache_key_fed = false;

    nfc_blink_stop(instance);
}
