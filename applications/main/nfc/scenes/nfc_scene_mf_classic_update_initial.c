#include "../nfc_app_i.h"

#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller.h>

enum {
    NfcSceneMfClassicUpdateInitialStateCardSearch,
    NfcSceneMfClassicUpdateInitialStateCardFound,
};

// Overlay onto `base` only what `fresh` actually recovered. The read poller starts from an empty
// card (unlike the dictionary modes it is not seeded with our dump) and reports success as soon as
// the key cache runs out, so a sector that failed to authenticate this pass is simply absent from
// `fresh` -- replacing outright would drop its key and blocks from the dump. Sector trailers are
// safe to pass through: mf_classic_set_block_read() copies only the access bytes out of them, and
// the keys travel through mf_classic_set_key_found().
// Kept local rather than mirroring lib's mf_plus_merge_update(): every primitive it needs is
// already public, so there is no reason to grow the SDK API for one caller.
static void
    nfc_scene_mf_classic_update_initial_merge(MfClassicData* base, const MfClassicData* fresh) {
    const uint16_t blocks_total = mf_classic_get_total_block_num(fresh->type);
    for(uint16_t block_num = 0; block_num < blocks_total; block_num++) {
        if(!mf_classic_is_block_read(fresh, block_num)) continue;
        MfClassicBlock block = fresh->block[block_num];
        mf_classic_set_block_read(base, block_num, &block);
    }

    const uint8_t sectors_total = mf_classic_get_total_sectors_num(fresh->type);
    const MfClassicKeyType key_types[] = {MfClassicKeyTypeA, MfClassicKeyTypeB};
    for(uint8_t sector_num = 0; sector_num < sectors_total; sector_num++) {
        const MfClassicSectorTrailer* sec_tr =
            mf_classic_get_sector_trailer_by_sector(fresh, sector_num);
        for(size_t i = 0; i < COUNT_OF(key_types); i++) {
            if(!mf_classic_is_key_found(fresh, sector_num, key_types[i])) continue;
            const MfClassicKey* key = (key_types[i] == MfClassicKeyTypeA) ? &sec_tr->key_a :
                                                                            &sec_tr->key_b;
            mf_classic_set_key_found(
                base,
                sector_num,
                key_types[i],
                bit_lib_bytes_to_num_be(key->data, sizeof(MfClassicKey)));
        }
    }
}

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
        if(mf_classic_key_cache_get_next_key(
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
        MfClassicData* merged = mf_classic_alloc();
        mf_classic_copy(merged, nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic));
        nfc_scene_mf_classic_update_initial_merge(merged, updated_data);
        nfc_device_set_data(instance->nfc_device, NfcProtocolMfClassic, merged);
        mf_classic_free(merged);
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
            instance->popup, "Use the source\ncard only", 128, 32, AlignRight, AlignCenter);
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
            scene_manager_next_scene(
                instance->scene_manager, NfcSceneMfClassicUpdateInitialWrongCard);
            consumed = true;
        } else if(event.event == NfcCustomEventWorkerExit) {
            if(nfc_save_shadow_file(instance)) {
                scene_manager_next_scene(
                    instance->scene_manager, NfcSceneMfClassicUpdateInitialSuccess);
            } else {
                scene_manager_next_scene(
                    instance->scene_manager, NfcSceneMfClassicUpdateInitialWrongCard);
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
