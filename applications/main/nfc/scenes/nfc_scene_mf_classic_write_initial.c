#include "../nfc_app_i.h"

#include <nfc/protocols/mf_classic/mf_classic_poller.h>

enum {
    NfcSceneMfClassicWriteInitialStateCardSearch,
    NfcSceneMfClassicWriteInitialStateCardFound,
};

NfcCommand
    nfc_scene_mf_classic_write_initial_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcCommand command = NfcCommandContinue;
    NfcApp* instance = context;
    MfClassicPollerEvent* mfc_event = event.event_data;
    const MfClassicData* write_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);

    if(mfc_event->type == MfClassicPollerEventTypeCardDetected) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);
    } else if(mfc_event->type == MfClassicPollerEventTypeCardLost) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardLost);
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        const MfClassicData* tag_data = nfc_poller_get_data(instance->poller);
        if(iso14443_3a_is_equal(tag_data->iso14443_3a_data, write_data->iso14443_3a_data)) {
            mfc_event->data->poller_mode.mode = MfClassicPollerModeWrite;
        } else {
            view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
            command = NfcCommandStop;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestSectorTrailer) {
        uint8_t sector = mfc_event->data->sec_tr_data.sector_num;
        uint8_t sec_tr = mf_classic_get_sector_trailer_num_by_sector(sector);
        if(mf_classic_is_block_read(write_data, sec_tr)) {
            mfc_event->data->sec_tr_data.sector_trailer = write_data->block[sec_tr];
            mfc_event->data->sec_tr_data.sector_trailer_provided = true;
        } else {
            mfc_event->data->sec_tr_data.sector_trailer_provided = false;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestWriteBlock) {
        uint8_t block_num = mfc_event->data->write_block_data.block_num;
        if(mf_classic_is_block_read(write_data, block_num)) {
            mfc_event->data->write_block_data.write_block = write_data->block[block_num];
            mfc_event->data->write_block_data.write_block_provided = true;
        } else {
            mfc_event->data->write_block_data.write_block_provided = false;
        }
    } else if(mfc_event->type == MfClassicPollerEventTypeSuccess) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    } else if(mfc_event->type == MfClassicPollerEventTypeFail) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
        command = NfcCommandStop;
    }
    return command;
}

static void nfc_scene_mf_classic_write_initial_setup_view(NfcApp* instance) {
    Popup* popup = instance->popup;
    popup_reset(popup);
    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfClassicWriteInitial);

    if(state == NfcSceneMfClassicWriteInitialStateCardSearch) {
        popup_set_header(instance->popup, "Writing", 95, 20, AlignCenter, AlignCenter);
        popup_set_text(
            instance->popup, "Use the source\ncard only", 95, 38, AlignCenter, AlignCenter);
        popup_set_icon(instance->popup, 0, 8, &I_NFC_manual_60x50);
    } else {
        popup_set_header(popup, "Writing\nDon't move...", 52, 32, AlignLeft, AlignCenter);
        popup_set_icon(popup, 12, 23, &A_Loading_24);
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);
}

void nfc_scene_mf_classic_write_initial_on_enter(void* context) {
    NfcApp* instance = context;
    dolphin_deed(DolphinDeedNfcEmulate);

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfClassicWriteInitial,
        NfcSceneMfClassicWriteInitialStateCardSearch);
    nfc_scene_mf_classic_write_initial_setup_view(instance);

    // Setup and start worker
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
    nfc_poller_start(
        instance->poller, nfc_scene_mf_classic_write_initial_worker_callback, instance);

    nfc_blink_emulate_start(instance);
}

bool nfc_scene_mf_classic_write_initial_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventCardDetected) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfClassicWriteInitial,
                NfcSceneMfClassicWriteInitialStateCardFound);
            nfc_scene_mf_classic_write_initial_setup_view(instance);
            consumed = true;
        } else if(event.event == NfcCustomEventCardLost) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfClassicWriteInitial,
                NfcSceneMfClassicWriteInitialStateCardSearch);
            nfc_scene_mf_classic_write_initial_setup_view(instance);
            consumed = true;
        } else if(event.event == NfcCustomEventWrongCard) {
            scene_manager_next_scene(
                instance->scene_manager, NfcSceneMfClassicWriteInitialWrongCard);
            consumed = true;
        } else if(event.event == NfcCustomEventPollerSuccess) {
            scene_manager_next_scene(
                instance->scene_manager, NfcSceneMfClassicWriteInitialSuccess);
            consumed = true;
        } else if(event.event == NfcCustomEventPollerFailure) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicWriteInitialFail);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mf_classic_write_initial_on_exit(void* context) {
    NfcApp* instance = context;

    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfClassicWriteInitial,
        NfcSceneMfClassicWriteInitialStateCardSearch);
    // Clear view
    popup_reset(instance->popup);

    nfc_blink_stop(instance);
}
