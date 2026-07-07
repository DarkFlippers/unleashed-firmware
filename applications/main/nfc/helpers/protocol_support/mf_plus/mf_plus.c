#include "mf_plus.h"
#include "mf_plus_render.h"

#include <nfc/protocols/mf_plus/mf_plus_poller.h>

#include "nfc/nfc_app_i.h"
#include "../../mf_plus_key_cache.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"
#include "../iso14443_4a/iso14443_4a_i.h"

enum {
    SubmenuIndexShowKeys = SubmenuIndexCommonMax,
    SubmenuIndexUpdate,
};

// SL3 is the only level with recovered AES content: SL0/SL1/SL2 have no sector keys/blocks here
// (SL1 is read as MIFARE Classic and never reaches this handler), so only SL3 gets full emulation,
// More Info, and the Show Keys view. The dictionary attack runs automatically on read (see the read
// on_event), so there is no manual "Unlock with Dictionary" menu entry.
static bool nfc_scene_mf_plus_is_sl3(NfcApp* instance) {
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
    return data->security_level == MfPlusSecurityLevel3;
}

// "Show Keys" lists the recovered SL3 sector and admin keys. Offered on both the read and saved
// menus for SL3 cards; always present (even with no keys yet) so the view is always reachable. The
// ISO14443-4/version details live behind the Info screen's "More" hub, not on these menus.
static void nfc_scene_mf_plus_add_show_keys(NfcApp* instance) {
    if(!nfc_scene_mf_plus_is_sl3(instance)) return;
    submenu_add_item(
        instance->submenu,
        "Show Keys",
        SubmenuIndexShowKeys,
        nfc_protocol_support_common_submenu_callback,
        instance);
}

// Read menu: writing straight after a read makes no sense, so drop the generic "Write" item
// (mirrors MIFARE Classic).
static void nfc_scene_mf_plus_read_menu_on_enter(NfcApp* instance) {
    submenu_remove_item(instance->submenu, SubmenuIndexCommonWrite);
    nfc_scene_mf_plus_add_show_keys(instance);
}

// Saved menu: the generic "Write" item (advertised only for SL3) writes the dump back to the source
// card, so relabel it to say exactly that, and offer "Update from Initial Card" (re-read the source
// card with the saved keys to refresh the dump).
static void nfc_scene_mf_plus_saved_menu_on_enter(NfcApp* instance) {
    if(nfc_scene_mf_plus_is_sl3(instance)) {
        submenu_change_item_label(
            instance->submenu, SubmenuIndexCommonWrite, "Write to Initial Card");
        submenu_add_item(
            instance->submenu,
            "Update from Initial Card",
            SubmenuIndexUpdate,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }
    nfc_scene_mf_plus_add_show_keys(instance);
}

static bool nfc_scene_mf_plus_menu_on_event(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexShowKeys) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusShowKeys);
            return true;
        } else if(event.event == SubmenuIndexUpdate) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusUpdateInitial);
            return true;
        }
    }
    return false;
}

static void nfc_scene_info_on_enter_mf_plus(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfPlusData* data = nfc_device_get_data(device, NfcProtocolMfPlus);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    furi_string_replace(temp_str, "Mifare", "MIFARE");
    nfc_render_mf_plus_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

// The Info screen's "More" button lands here; jump straight to the More-info hub (View Dump /
// ISO14443-4 Data), DESFire-style, since that hub owns its own submenu view and back handling.
static void nfc_scene_more_info_on_enter_mf_plus(NfcApp* instance) {
    scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusMoreInfo);
}

static NfcCommand nfc_scene_read_poller_callback_mf_plus(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolMfPlus);
    furi_assert(event.event_data);

    NfcApp* instance = context;
    const MfPlusPollerEvent* mf_plus_event = event.event_data;

    NfcCommand command = NfcCommandContinue;

    if(mf_plus_event->type == MfPlusPollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfPlus, nfc_poller_get_data(instance->poller));
        // The identity scan is done. An SL3 card can be dictionary-attacked to recover its keys
        // and blocks, so continue straight into the dictionary attack (like MIFARE Classic auto-
        // runs its dict). SL0/SL1/SL2 have nothing further to read here, so finish.
        const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);
        const NfcCustomEvent custom_event = (data->security_level == MfPlusSecurityLevel3) ?
                                                NfcCustomEventPollerIncomplete :
                                                NfcCustomEventPollerSuccess;
        view_dispatcher_send_custom_event(instance->view_dispatcher, custom_event);
        command = NfcCommandStop;
    } else if(mf_plus_event->type == MfPlusPollerEventTypeReadFailed) {
        command = NfcCommandReset;
    }

    return command;
}

static void nfc_scene_read_on_enter_mf_plus(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_mf_plus, instance);
}

static bool nfc_scene_read_on_event_mf_plus(NfcApp* instance, SceneManagerEvent event) {
    // Auto-continue an SL3 identity read into the dictionary attack (mirrors MIFARE Classic).
    if(event.type == SceneManagerEventTypeCustom &&
       event.event == NfcCustomEventPollerIncomplete) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneMfPlusDictAttack);
        return true;
    }
    return false;
}

static void nfc_scene_read_success_on_enter_mf_plus(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfPlusData* data = nfc_device_get_data(device, NfcProtocolMfPlus);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    furi_string_replace(temp_str, "Mifare", "MIFARE");
    nfc_render_mf_plus_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_emulate_on_enter_mf_plus(NfcApp* instance) {
    const MfPlusData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    if(data->security_level == MfPlusSecurityLevel3) {
        // SL3 has recovered keys/blocks/config: emulate the full native card, so a reader can
        // authenticate, read and write it (a reader write mutates the data in place; the generic
        // emulate-exit diff then saves a .shd shadow).
        instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolMfPlus, data);
        nfc_listener_start(instance->listener, NULL, NULL);
    } else {
        // SL0/SL1/SL2 have no recovered SL3 memory to emulate: fall back to UID-only, like UL-AES.
        const Iso14443_4aData* iso14443_4a_data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolIso14443_4a);
        instance->listener =
            nfc_listener_alloc(instance->nfc, NfcProtocolIso14443_4a, iso14443_4a_data);
        nfc_listener_start(
            instance->listener, nfc_scene_emulate_listener_callback_iso14443_4a, instance);
    }
}

// Write the loaded dump's recovered data blocks back to the source card. Answers the write poller:
// gate on a UID match (write only the card the dump came from), then feed the recovered sector keys
// and data blocks from the dump. Skipping a sector/block the dump lacks is expressed by leaving
// key_provided / block_provided false.
static NfcCommand nfc_scene_write_poller_callback_mf_plus(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolMfPlus);
    furi_assert(event.event_data);

    NfcApp* instance = context;
    const MfPlusPollerEvent* mfp_event = event.event_data;
    const MfPlusData* write_data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus);

    NfcCommand command = NfcCommandContinue;

    switch(mfp_event->type) {
    case MfPlusPollerEventTypeRequestMode: {
        // Only write the exact card the dump came from: match the UID before entering write mode.
        const MfPlusData* tag_data = nfc_poller_get_data(instance->poller);
        size_t tag_uid_len = 0, dump_uid_len = 0;
        const uint8_t* tag_uid = mf_plus_get_uid(tag_data, &tag_uid_len);
        const uint8_t* dump_uid = mf_plus_get_uid(write_data, &dump_uid_len);
        if(tag_uid_len == dump_uid_len && memcmp(tag_uid, dump_uid, tag_uid_len) == 0) {
            mfp_event->data->mode_request.mode = MfPlusPollerModeWrite;
        } else {
            furi_string_set(instance->text_box_store, "Use the source\ncard only");
            view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
            command = NfcCommandStop;
        }
        break;
    }
    case MfPlusPollerEventTypeRequestWriteSector: {
        // Provide the sector's recovered AES key (prefer A) so the poller can authenticate to write.
        const uint8_t sector = mfp_event->data->write_sector_request.sector;
        if(mf_plus_is_key_found(write_data, sector, MfPlusKeyTypeA)) {
            mfp_event->data->write_sector_request.key = write_data->key_a[sector];
            mfp_event->data->write_sector_request.key_type = MfPlusKeyTypeA;
            mfp_event->data->write_sector_request.key_provided = true;
        } else if(mf_plus_is_key_found(write_data, sector, MfPlusKeyTypeB)) {
            mfp_event->data->write_sector_request.key = write_data->key_b[sector];
            mfp_event->data->write_sector_request.key_type = MfPlusKeyTypeB;
            mfp_event->data->write_sector_request.key_provided = true;
        }
        break;
    }
    case MfPlusPollerEventTypeRequestWriteBlock: {
        // Provide the block only if the dump captured it.
        const uint16_t block_num = mfp_event->data->write_block_request.block_num;
        if(mf_plus_is_block_read(write_data, block_num)) {
            mfp_event->data->write_block_request.block = write_data->block[block_num];
            mfp_event->data->write_block_request.block_provided = true;
        }
        break;
    }
    case MfPlusPollerEventTypeWriteSuccess:
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
        break;
    case MfPlusPollerEventTypeWriteFailed:
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
        command = NfcCommandStop;
        break;
    default:
        break;
    }

    return command;
}

static void nfc_scene_write_on_enter_mf_plus(NfcApp* instance) {
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfPlus);
    nfc_poller_start(instance->poller, nfc_scene_write_poller_callback_mf_plus, instance);
    furi_string_set(instance->text_box_store, "Use the source\ncard only");
}

// On save-name confirm (the dump file is already written by the generic handler), cache the
// recovered SL3 keys under /ext/nfc/.cache so a later read of this same card authenticates straight
// from the cache instead of re-running the whole dictionary attack (mirrors MIFARE Classic). Only
// SL3 carries recovered keys; the cache save is a no-op for a card with none.
static bool nfc_scene_save_name_on_event_mf_plus(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom && event.event == NfcCustomEventTextInputDone) {
        if(nfc_scene_mf_plus_is_sl3(instance)) {
            mf_plus_key_cache_save(nfc_device_get_data(instance->nfc_device, NfcProtocolMfPlus));
        }
        consumed = true;
    }

    return consumed;
}

// SL3 exposes full native emulation (a reader can authenticate and read the recovered card) plus
// write-to-card (write the recovered data blocks back to the source card); SL0/SL1/SL2 have no
// recovered memory, so they only emulate the UID and can't be written. Evaluated at runtime per
// loaded card -- a static .features field would wrongly offer these for every level.
#define MF_PLUS_SL3_FEATURES \
    (NfcProtocolFeatureEmulateFull | NfcProtocolFeatureMoreInfo | NfcProtocolFeatureWrite)
#define MF_PLUS_UID_FEATURES (NfcProtocolFeatureEmulateUid | NfcProtocolFeatureMoreInfo)

static uint32_t nfc_mf_plus_get_features(NfcApp* instance) {
    return nfc_scene_mf_plus_is_sl3(instance) ? MF_PLUS_SL3_FEATURES : MF_PLUS_UID_FEATURES;
}

const NfcProtocolSupportBase nfc_protocol_support_mf_plus = {
    .features = MF_PLUS_UID_FEATURES,
    .get_features = nfc_mf_plus_get_features,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_scene_more_info_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_mf_plus,
            .on_event = nfc_scene_read_on_event_mf_plus,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_scene_mf_plus_read_menu_on_enter,
            .on_event = nfc_scene_mf_plus_menu_on_event,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_saved_menu =
        {
            .on_enter = nfc_scene_mf_plus_saved_menu_on_enter,
            .on_event = nfc_scene_mf_plus_menu_on_event,
        },
    .scene_save_name =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_scene_save_name_on_event_mf_plus,
        },
    .scene_emulate =
        {
            .on_enter = nfc_scene_emulate_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_write =
        {
            .on_enter = nfc_scene_write_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};

NFC_PROTOCOL_SUPPORT_PLUGIN(mf_plus, NfcProtocolMfPlus);
