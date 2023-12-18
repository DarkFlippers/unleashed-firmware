#include "../nfc_app_i.h"

#include <nfc/protocols/mf_classic/mf_classic_poller.h>

enum {
    NfcSceneMfClassicUpdateInitialStateCardSearch,
    NfcSceneMfClassicUpdateInitialStateCardFound,
};

NfcCommand nfc_mf_classic_update_initial_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcCommand command = NfcCommandContinue;
    const MfClassicPollerEvent* mfc_event = event.event_data;
    NfcApp* instance = context;

    if(mfc_event->type == MfClassicPollerEventTypeCardDetected) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);
    } else if(mfc_event->type == MfClassicPollerEventTypeCardLost) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardLost);
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        const MfClassicData* updated_data = nfc_poller_get_data(instance->poller);
        const MfClassicData* old_data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);
        if(iso14443_3a_is_equal(updated_data->iso14443_3a_data, old_data->iso14443_3a_data)) {
            mfc_event->data->poller_mode.mode = MfClassicPollerModeRead;
        } else {
            view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
            command = NfcCommandStop;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestReadSector) {
        uint8_t sector_num = 0;
        MfClassicKey key = {};
        MfClassicKeyType key_type = MfClassicKeyTypeA;
        if(mf_classic_key_cahce_get_next_key(
               instance->mfc_key_cache, &sector_num, &key, &key_type)) {
            mfc_event->data->read_sector_request_data.sector_num = sector_num;
            mfc_event->data->read_sector_request_data.key = key;
            mfc_event->data->read_sector_request_data.key_type = key_type;
            mfc_event->data->read_sector_request_data.key_provided = true;
        } else {
            mfc_event->data->read_sector_request_data.key_provided = false;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeSuccess) {
        const MfClassicData* updated_data = nfc_poller_get_data(instance->poller);
        nfc_device_set_data(instance->nfc_device, NfcProtocolMfClassic, updated_data);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWorkerExit);
        command = NfcCommandStop;
    }

    return command;
}

static void nfc_scene_mf_classic_update_initial_setup_view(NfcApp* instance) {
    Popup* popup = instance->popup;
    popup_reset(popup);
    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicUpdateInitial);

    if(state == NfcSceneMfClassicUpdateInitialStateCardSearch) {
        popup_set_text(
            instance->popup, "Apply the initial\ncard only", 128, 32, AlignRight, AlignCenter);
        popup_set_icon(instance->popup, 0, 8, &I_NFC_manual_60x50);
    } else {
        popup_set_header(popup, "Updating\nDon't move...", 52, 32, AlignLeft, AlignCenter);
        popup_set_icon(popup, 12, 23, &A_Loading_24);
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);
}

void nfc_scene_mf_classic_update_initial_on_enter(void* context) {
    NfcApp* instance = context;
    dolphin_deed(DolphinDeedNfcEmulate);

    const MfClassicData* mfc_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);
    mf_classic_key_cache_load_from_data(instance->mfc_key_cache, mfc_data);

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfClassicUpdateInitial,
        NfcSceneMfClassicUpdateInitialStateCardSearch);
    nfc_scene_mf_classic_update_initial_setup_view(instance);

    // Setup and start worker
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
    nfc_poller_start(instance->poller, nfc_mf_classic_update_initial_worker_callback, instance);
    nfc_blink_emulate_start(instance);
}

bool nfc_scene_mf_classic_update_initial_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventCardDetected) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfClassicUpdateInitial,
                NfcSceneMfClassicUpdateInitialStateCardFound);
            nfc_scene_mf_classic_update_initial_setup_view(instance);
            consumed = true;
        } else if(event.event == NfcCustomEventCardLost) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfClassicUpdateInitial,
                NfcSceneMfClassicUpdateInitialStateCardSearch);
            nfc_scene_mf_classic_update_initial_setup_view(instance);
            consumed = true;
        } else if(event.event == NfcCustomEventWrongCard) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicWrongCard);
            consumed = true;
        } else if(event.event == NfcCustomEventWorkerExit) {
            if(nfc_save_shadow_file(instance)) {
                scene_manager_next_scene(
                    instance->scene_manager, NfcSceneMfClassicUpdateInitialSuccess);
            } else {
                scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicWrongCard);
                consumed = true;
            }
        }
    }

    return consumed;
}

void nfc_scene_mf_classic_update_initial_on_exit(void* context) {
    NfcApp* instance = context;

    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfClassicUpdateInitial,
        NfcSceneMfClassicUpdateInitialStateCardSearch);
    // Clear view
    popup_reset(instance->popup);

    nfc_blink_stop(instance);
}
