#include "felica.h"
#include "felica_render.h"

#include <nfc/protocols/felica/felica_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"
#include "../nfc_protocol_support_unlock_helper.h"

enum {
    SubmenuIndexUnlock = SubmenuIndexCommonMax,
};

static void nfc_scene_info_on_enter_felica(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const FelicaData* data = nfc_device_get_data(device, NfcProtocolFelica);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_felica_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 48, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static bool nfc_scene_info_on_event_felica(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom && event.event == GuiButtonTypeRight) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneMoreInfo);
        return true;
    }

    return false;
}

static void nfc_scene_more_info_on_enter_felica(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const FelicaData* data = nfc_device_get_data(device, NfcProtocolFelica);

    FuriString* temp_str = furi_string_alloc();

    nfc_render_felica_dump(data, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static NfcCommand nfc_scene_read_poller_callback_felica(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolFelica);

    NfcApp* instance = context;
    const FelicaPollerEvent* felica_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(felica_event->type == FelicaPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolFelica, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    } else if(
        felica_event->type == FelicaPollerEventTypeError ||
        felica_event->type == FelicaPollerEventTypeIncomplete) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolFelica, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, NfcCustomEventPollerIncomplete);
        command = NfcCommandStop;
    } else if(felica_event->type == FelicaPollerEventTypeRequestAuthContext) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);
        FelicaAuthenticationContext* ctx = felica_event->data->auth_context;
        ctx->skip_auth = instance->felica_auth->skip_auth;
        memcpy(ctx->card_key.data, instance->felica_auth->card_key.data, FELICA_DATA_BLOCK_SIZE);
    }

    return command;
}

static void nfc_scene_read_on_enter_felica(NfcApp* instance) {
    nfc_unlock_helper_setup_from_state(instance);
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_felica, instance);
}

bool nfc_scene_read_on_event_felica(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventCardDetected) {
            nfc_unlock_helper_card_detected_handler(instance);
        } else if(event.event == NfcCustomEventPollerIncomplete) {
            notification_message(instance->notifications, &sequence_semi_success);
            scene_manager_next_scene(instance->scene_manager, NfcSceneReadSuccess);
            dolphin_deed(DolphinDeedNfcReadSuccess);
        }
    }
    return true;
}

static void nfc_scene_read_success_on_enter_felica(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const FelicaData* data = nfc_device_get_data(device, NfcProtocolFelica);

    FuriString* temp_str = furi_string_alloc();

    if(!scene_manager_has_previous_scene(instance->scene_manager, NfcSceneDesAuthUnlockWarn)) {
        furi_string_cat_printf(
            temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
        nfc_render_felica_info(data, NfcProtocolFormatTypeShort, temp_str);
    } else {
        bool all_unlocked = data->blocks_read == data->blocks_total;
        furi_string_cat_printf(
            temp_str,
            "\e#%s\n",
            all_unlocked ? "All Blocks Are Unlocked" : "Some Blocks Are Locked");
        nfc_render_felica_idm(data, NfcProtocolFormatTypeShort, temp_str);
        uint8_t* ck_data = instance->felica_auth->card_key.data;
        furi_string_cat_printf(temp_str, "Key:");
        for(uint8_t i = 0; i < 7; i++) {
            furi_string_cat_printf(temp_str, " %02X", ck_data[i]);
            if(i == 6) furi_string_cat_printf(temp_str, "...");
        }
        nfc_render_felica_blocks_count(data, temp_str, false);
    }
    felica_auth_reset(instance->felica_auth);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static bool nfc_scene_saved_menu_on_event_felica(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom && event.event == SubmenuIndexCommonEdit) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneSetUid);
        return true;
    }

    return false;
}

static void nfc_scene_emulate_on_enter_felica(NfcApp* instance) {
    const FelicaData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolFelica);
    instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolFelica, data);
    nfc_listener_start(instance->listener, NULL, NULL);
}

static void nfc_scene_read_menu_on_enter_felica(NfcApp* instance) {
    const FelicaData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolFelica);
    if(data->blocks_read != data->blocks_total) {
        submenu_add_item(
            instance->submenu,
            "Unlock",
            SubmenuIndexUnlock,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }
}

static bool nfc_scene_read_menu_on_event_felica(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexUnlock) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneDesAuthKeyInput);
            return true;
        }
    }
    return false;
}

const NfcProtocolSupportBase nfc_protocol_support_felica = {
    .features = NfcProtocolFeatureEmulateFull | NfcProtocolFeatureMoreInfo,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_felica,
            .on_event = nfc_scene_info_on_event_felica,
        },
    .scene_more_info =
        {
            .on_enter = nfc_scene_more_info_on_enter_felica,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_felica,
            .on_event = nfc_scene_read_on_event_felica,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_scene_read_menu_on_enter_felica,
            .on_event = nfc_scene_read_menu_on_event_felica,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_felica,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_saved_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_scene_saved_menu_on_event_felica,
        },
    .scene_save_name =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_emulate =
        {
            .on_enter = nfc_scene_emulate_on_enter_felica,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};
