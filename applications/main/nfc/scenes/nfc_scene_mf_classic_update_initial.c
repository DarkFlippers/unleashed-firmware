#include "../nfc_app_i.h"

#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller.h>

#define TAG "NfcMfClassicUpdateInitial"

enum {
    NfcSceneMfClassicUpdateInitialStateCardSearch,
    NfcSceneMfClassicUpdateInitialStateCardFound,
};

// Overlay onto `base` only what `fresh` actually recovered. The read poller is not seeded with our
// dump -- only the dictionary modes are -- and it reports success as soon as the key cache runs
// out, so a sector that failed to authenticate this pass is simply absent from `fresh`; replacing
// outright would drop its key and blocks. Trailers pass through safely: mf_classic_set_block_read()
// copies only the access bytes out of one, and keys travel through mf_classic_set_key_found().
// Bounds are the array maxima rather than either card's geometry, because the key cache offers
// every sector `base` holds a key for -- `fresh` can come back with sectors past its own type.
static void
    nfc_scene_mf_classic_update_initial_merge(MfClassicData* base, const MfClassicData* fresh) {
    for(uint16_t block_num = 0; block_num < MF_CLASSIC_TOTAL_BLOCKS_MAX; block_num++) {
        if(!mf_classic_is_block_read(fresh, block_num)) continue;
        MfClassicBlock block = fresh->block[block_num]; // set_block_read() wants it mutable
        mf_classic_set_block_read(base, block_num, &block);
    }

    const MfClassicKeyType key_types[] = {MfClassicKeyTypeA, MfClassicKeyTypeB};
    for(uint8_t sector_num = 0; sector_num < MF_CLASSIC_TOTAL_SECTORS_MAX; sector_num++) {
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

// The merge keeps the user's data safe but also hides a weak pass: the poller calls it a success
// once the key cache empties, and the popup says "Updated" either way. Log what the screen cannot.
static void nfc_scene_mf_classic_update_initial_log_gaps(
    const MfClassicData* dump,
    const MfClassicData* fresh) {
    uint8_t sectors_had = 0, keys_had = 0, sectors_got = 0, keys_got = 0;
    mf_classic_get_read_sectors_and_keys(dump, &sectors_had, &keys_had);
    mf_classic_get_read_sectors_and_keys(fresh, &sectors_got, &keys_got);
    if(sectors_got < sectors_had || keys_got < keys_had) {
        FURI_LOG_W(
            TAG,
            "Partial re-read: %u/%u sectors, %u/%u keys; keeping the rest of the dump",
            sectors_got,
            sectors_had,
            keys_got,
            keys_had);
    }
    if(dump->type != fresh->type) {
        FURI_LOG_W(
            TAG,
            "Type mismatch: dump %s, card %s; keeping the dump's",
            mf_classic_get_device_name(dump, NfcDeviceNameTypeShort),
            mf_classic_get_device_name(fresh, NfcDeviceNameTypeShort));
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
        nfc_device_copy_data(instance->nfc_device, NfcProtocolMfClassic, merged);
        nfc_scene_mf_classic_update_initial_log_gaps(merged, updated_data);
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
                instance->scene_manager, NfcSceneUpdateInitialWrongCard);
            consumed = true;
        } else if(event.event == NfcCustomEventWorkerExit) {
            if(nfc_save_shadow_file(instance)) {
                scene_manager_next_scene(
                    instance->scene_manager, NfcSceneUpdateInitialSuccess);
            } else {
                scene_manager_next_scene(
                    instance->scene_manager, NfcSceneUpdateInitialWrongCard);
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
