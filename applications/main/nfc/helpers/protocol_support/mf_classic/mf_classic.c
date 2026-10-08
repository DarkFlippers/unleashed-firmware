#include "mf_classic.h"
#include "mf_classic_render.h"

#include <nfc/protocols/mf_classic/mf_classic_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"

#define TAG "MfClassicApp"

enum {
    SubmenuIndexDetectReader = SubmenuIndexCommonMax,
    SubmenuIndexDictAttack,
    SubmenuIndexCrackNonces,
    SubmenuIndexUpdate,
};

static void nfc_scene_info_on_enter_mf_classic(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    furi_string_replace(temp_str, "Mifare", "MIFARE");

    nfc_render_mf_classic_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_more_info_on_enter_mf_classic(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfClassicData* mfc_data = nfc_device_get_data(device, NfcProtocolMfClassic);

    furi_string_reset(instance->text_box_store);
    nfc_render_mf_classic_dump(mfc_data, instance->text_box_store);

    text_box_set_font(instance->text_box, TextBoxFontHex);
    text_box_set_text(instance->text_box, furi_string_get_cstr(instance->text_box_store));
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewTextBox);
}

static NfcCommand nfc_scene_read_poller_callback_mf_classic(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcApp* instance = context;
    const MfClassicPollerEvent* mfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfClassic, nfc_poller_get_data(instance->poller));
        size_t uid_len = 0;
        const uint8_t* uid = nfc_device_get_uid(instance->nfc_device, &uid_len);
        if(mf_classic_key_cache_load(instance->mfc_key_cache, uid, uid_len)) {
            FURI_LOG_I(TAG, "Key cache found");
            mfc_event->data->poller_mode.mode = MfClassicPollerModeRead;
        } else {
            FURI_LOG_I(TAG, "Key cache not found");
            view_dispatcher_send_custom_event(
                instance->view_dispatcher, NfcCustomEventPollerIncomplete);
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
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfClassic, nfc_poller_get_data(instance->poller));
        const MfClassicData* mfc_data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);
        NfcCustomEvent custom_event = mf_classic_is_card_read(mfc_data) ?
                                          NfcCustomEventPollerSuccess :
                                          NfcCustomEventPollerIncomplete;
        view_dispatcher_send_custom_event(instance->view_dispatcher, custom_event);
        command = NfcCommandStop;
    }

    return command;
}

static void nfc_scene_read_on_enter_mf_classic(NfcApp* instance) {
    mf_classic_key_cache_reset(instance->mfc_key_cache);
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_mf_classic, instance);
}

static bool nfc_scene_read_on_event_mf_classic(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom &&
       event.event == NfcCustomEventPollerIncomplete) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicDictAttack);
    }

    return true;
}

static void nfc_scene_read_menu_on_enter_mf_classic(NfcApp* instance) {
    Submenu* submenu = instance->submenu;
    const MfClassicData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);

    // Doesn't make sense to show "Write to Initial Card" right after reading
    submenu_remove_item(submenu, SubmenuIndexCommonWrite);

    if(!mf_classic_is_card_read(data)) {
        submenu_add_item(
            submenu,
            "Extract MF Keys",
            SubmenuIndexDetectReader,
            nfc_protocol_support_common_submenu_callback,
            instance);

        submenu_add_item(
            submenu,
            "Unlock with Dictionary",
            SubmenuIndexDictAttack,
            nfc_protocol_support_common_submenu_callback,
            instance);

        submenu_add_item(
            submenu,
            "Crack nonces in MFKey32",
            SubmenuIndexCrackNonces,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }
}

static void nfc_scene_read_success_on_enter_mf_classic(NfcApp* instance) { //-V524
    const NfcDevice* device = instance->nfc_device;
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    furi_string_replace(temp_str, "Mifare", "MIFARE");

    nfc_render_mf_classic_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_saved_menu_on_enter_mf_classic(NfcApp* instance) {
    Submenu* submenu = instance->submenu;
    const MfClassicData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);

    submenu_change_item_label(submenu, SubmenuIndexCommonWrite, "Write to Initial Card");

    if(!mf_classic_is_card_read(data)) {
        submenu_add_item(
            submenu,
            "Extract MF Keys",
            SubmenuIndexDetectReader,
            nfc_protocol_support_common_submenu_callback,
            instance);

        submenu_add_item(
            submenu,
            "Unlock with Dictionary",
            SubmenuIndexDictAttack,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }

    submenu_add_item(
        submenu,
        "Update from Initial Card",
        SubmenuIndexUpdate,
        nfc_protocol_support_common_submenu_callback,
        instance);
}

static void nfc_scene_emulate_on_enter_mf_classic(NfcApp* instance) {
    const MfClassicData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);
    instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolMfClassic, data);
    nfc_listener_start(instance->listener, NULL, NULL);
}

static bool nfc_scene_read_menu_on_event_mf_classic(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexDetectReader) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneSaveConfirm,
                NfcSceneSaveConfirmStateDetectReader);

            scene_manager_next_scene(instance->scene_manager, NfcSceneSaveConfirm);
            dolphin_deed(DolphinDeedNfcDetectReader);
            consumed = true;
        } else if(event.event == SubmenuIndexDictAttack) {
            if(!scene_manager_search_and_switch_to_previous_scene(
                   instance->scene_manager, NfcSceneMfClassicDictAttack)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicDictAttack);
            }
            consumed = true;
        } else if(event.event == SubmenuIndexCrackNonces) {
            scene_manager_set_scene_state(
                instance->scene_manager, NfcSceneSaveConfirm, NfcSceneSaveConfirmStateCrackNonces);
            scene_manager_next_scene(instance->scene_manager, NfcSceneSaveConfirm);
            consumed = true;
        }
    }

    return consumed;
}

static bool nfc_scene_saved_menu_on_event_mf_classic(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexDetectReader) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicDetectReader);
            consumed = true;
        } else if(event.event == SubmenuIndexDictAttack) {
            if(!scene_manager_search_and_switch_to_previous_scene(
                   instance->scene_manager, NfcSceneMfClassicDictAttack)) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicDictAttack);
            }
            consumed = true;
        } else if(event.event == SubmenuIndexUpdate) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicUpdateInitial);
            consumed = true;
        }
    }

    return consumed;
}

static bool nfc_scene_save_name_on_event_mf_classic(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom && event.event == NfcCustomEventTextInputDone) {
        mf_classic_key_cache_save(
            instance->mfc_key_cache,
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic));
        consumed = true;
    }

    return consumed;
}

static NfcCommand
    nfc_scene_write_poller_callback_mf_classic(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcApp* instance = context;
    MfClassicPollerEvent* mfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;
    const MfClassicData* write_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic);

    if(mfc_event->type == MfClassicPollerEventTypeCardDetected) {
        furi_string_reset(instance->text_box_store);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);
    } else if(mfc_event->type == MfClassicPollerEventTypeCardLost) {
        furi_string_set(instance->text_box_store, "Use the source\ncard only");
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardLost);
    } else if(mfc_event->type == MfClassicPollerEventTypeRequestMode) {
        const MfClassicData* tag_data = nfc_poller_get_data(instance->poller);
        if(iso14443_3a_is_equal(tag_data->iso14443_3a_data, write_data->iso14443_3a_data)) {
            mfc_event->data->poller_mode.mode = MfClassicPollerModeWrite;
        } else {
            furi_string_set(
                instance->text_box_store, "Use source card!\nTo write blanks\nuse NFC Magic app");
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
        furi_string_reset(instance->text_box_store);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    } else if(mfc_event->type == MfClassicPollerEventTypeFail) {
        furi_string_set(instance->text_box_store, "Not all sectors\nwere written\ncorrectly");
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
        command = NfcCommandStop;
    }

    return command;
}

static void nfc_scene_write_on_enter_mf_classic(NfcApp* instance) {
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfClassic);
    nfc_poller_start(instance->poller, nfc_scene_write_poller_callback_mf_classic, instance);
    furi_string_set(instance->text_box_store, "Use the source\ncard only");
}

const NfcProtocolSupportBase nfc_protocol_support_mf_classic = {
    .features = NfcProtocolFeatureEmulateFull | NfcProtocolFeatureMoreInfo |
                NfcProtocolFeatureWrite,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_mf_classic,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_scene_more_info_on_enter_mf_classic,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_mf_classic,
            .on_event = nfc_scene_read_on_event_mf_classic,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_scene_read_menu_on_enter_mf_classic,
            .on_event = nfc_scene_read_menu_on_event_mf_classic,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_mf_classic,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_saved_menu =
        {
            .on_enter = nfc_scene_saved_menu_on_enter_mf_classic,
            .on_event = nfc_scene_saved_menu_on_event_mf_classic,
        },
    .scene_save_name =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_scene_save_name_on_event_mf_classic,
        },
    .scene_emulate =
        {
            .on_enter = nfc_scene_emulate_on_enter_mf_classic,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_write =
        {
            .on_enter = nfc_scene_write_on_enter_mf_classic,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};
