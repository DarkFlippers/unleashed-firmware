#include "../nfc_app_i.h"

#include <nfc/protocols/mf_plus/mf_plus_poller.h>

// "Update from Initial Card": re-read the source SL3 card using the dump's already-recovered keys
// and refresh the saved data (+ shadow file). Unlike the read scene it runs no dictionary attack --
// it just feeds back the known keys.

// The read poller re-requests the same (sector, key type) / admin key after a failed authentication
// to get the next dictionary candidate. Here there is only ever one candidate -- the recovered key
// -- so an immediate repeat means that key no longer works (the card was re-keyed): answer "no key"
// to let the scan advance instead of looping. The scene state holds (last fed key id) + 1, 0 = none.
// Sector keys: id = sector*2 + key_type (0..79); admin keys: 80 + admin_type.
#define MF_PLUS_UPDATE_SECTOR_KEY_ID(sector, key_type) ((uint32_t)(sector) * 2 + (key_type))
#define MF_PLUS_UPDATE_ADMIN_KEY_ID(type)              (80u + (uint32_t)(type))

static bool nfc_scene_mf_plus_update_initial_feed_once(NfcApp* instance, uint32_t key_id) {
    const uint32_t last =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfPlusUpdateInitial);
    if(last == key_id + 1) {
        // Same key requested again -> its previous feed didn't authenticate. Stop feeding it.
        scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfPlusUpdateInitial, 0);
        return false;
    }
    scene_manager_set_scene_state(
        instance->scene_manager, NfcSceneMfPlusUpdateInitial, key_id + 1);
    return true;
}

NfcCommand nfc_mf_plus_update_initial_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfPlus);

    NfcCommand command = NfcCommandContinue;
    const MfPlusPollerEvent* mfp_event = event.event_data;
    NfcApp* instance = context;
    const MfPlusData* old_data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    switch(mfp_event->type) {
    case MfPlusPollerEventTypeRequestMode: {
        // Only refresh from the exact card the dump came from: match the UID before reading.
        const MfPlusData* tag_data = nfc_poller_get_data(instance->poller);
        size_t tag_uid_len = 0, old_uid_len = 0;
        const uint8_t* tag_uid = mf_plus_get_uid(tag_data, &tag_uid_len);
        const uint8_t* old_uid = mf_plus_get_uid(old_data, &old_uid_len);
        if(tag_uid_len == old_uid_len && memcmp(tag_uid, old_uid, tag_uid_len) == 0) {
            mfp_event->data->mode_request.mode = MfPlusPollerModeRead;
        } else {
            view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
            command = NfcCommandStop;
        }
        break;
    }
    case MfPlusPollerEventTypeRequestKey: {
        MfPlusPollerEventDataKeyRequest* req = &mfp_event->data->key_request;
        if(req->is_admin) {
            if(mf_plus_is_admin_key_found(old_data, req->admin_type) &&
               nfc_scene_mf_plus_update_initial_feed_once(
                   instance, MF_PLUS_UPDATE_ADMIN_KEY_ID(req->admin_type))) {
                req->key = old_data->admin_key[req->admin_type];
                req->key_provided = true;
            }
        } else if(
            mf_plus_is_key_found(old_data, req->sector, req->key_type) &&
            nfc_scene_mf_plus_update_initial_feed_once(
                instance, MF_PLUS_UPDATE_SECTOR_KEY_ID(req->sector, req->key_type))) {
            req->key = (req->key_type == MfPlusKeyTypeB) ? old_data->key_b[req->sector] :
                                                           old_data->key_a[req->sector];
            req->key_provided = true;
        }
        break;
    }
    case MfPlusPollerEventTypeReadSuccess: {
        // Merge the re-read over the saved dump instead of replacing it: the poller reports success
        // even if the best-effort admin/config/signature phase was cut short, so a wholesale replace
        // could silently drop keys/config the dump already had.
        const MfPlusData* updated_data = nfc_poller_get_data(instance->poller);
        MfPlusData* merged = mf_plus_alloc();
        mf_plus_copy(merged, old_data);
        mf_plus_merge_update(merged, updated_data);
        nfc_device_set_data(instance->nfc_device, NfcProtocolMfPlus, merged);
        mf_plus_free(merged);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWorkerExit);
        command = NfcCommandStop;
        break;
    }
    case MfPlusPollerEventTypeReadFailed:
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
        command = NfcCommandStop;
        break;
    default:
        break;
    }

    return command;
}

void nfc_scene_mf_plus_update_initial_on_enter(void* context) {
    NfcApp* instance = context;
    dolphin_deed(DolphinDeedNfcEmulate);

    scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfPlusUpdateInitial, 0);

    Popup* popup = instance->popup;
    popup_reset(popup);
    popup_set_text(popup, "Use the source\ncard only", 128, 32, AlignRight, AlignCenter);
    popup_set_icon(popup, 0, 8, &I_NFC_manual_60x50);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);

    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfPlus);
    nfc_poller_start(instance->poller, nfc_mf_plus_update_initial_worker_callback, instance);
    nfc_blink_emulate_start(instance);
}

bool nfc_scene_mf_plus_update_initial_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWrongCard) {
            scene_manager_next_scene(
                instance->scene_manager, NfcSceneUpdateInitialWrongCard);
            consumed = true;
        } else if(event.event == NfcCustomEventWorkerExit) {
            if(nfc_save_shadow_file(instance)) {
                scene_manager_next_scene(
                    instance->scene_manager, NfcSceneUpdateInitialSuccess);
            } else {
                scene_manager_next_scene(
                    instance->scene_manager, NfcSceneUpdateInitialWrongCard);
            }
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mf_plus_update_initial_on_exit(void* context) {
    NfcApp* instance = context;

    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    scene_manager_set_scene_state(instance->scene_manager, NfcSceneMfPlusUpdateInitial, 0);
    popup_reset(instance->popup);
    nfc_blink_stop(instance);
}
