#include "emv.h"
#include "emv_render.h"

#include <nfc/protocols/emv/emv_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"
#include "../iso14443_4a/iso14443_4a_i.h"

enum {
    SubmenuIndexTransactions = SubmenuIndexCommonMax,
};

static void nfc_scene_info_on_enter_emv(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const EmvData* data = nfc_device_get_data(device, NfcProtocolEmv);

    FuriString* temp_str = furi_string_alloc();
    // furi_string_cat_printf(
    //     temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_emv_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static NfcCommand nfc_scene_read_poller_callback_emv(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolEmv);

    NfcApp* instance = context;
    const EmvPollerEvent* emv_event = event.event_data;

    if(emv_event->type == EmvPollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolEmv, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static void nfc_scene_read_on_enter_emv(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_emv, instance);
}

static void nfc_scene_read_menu_on_enter_emv(NfcApp* instance) {
    Submenu* submenu = instance->submenu;
    const EmvData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolEmv);

    if(data->emv_application.active_tr > 0) {
        submenu_add_item(
            submenu,
            "Transactions",
            SubmenuIndexTransactions,
            nfc_protocol_support_common_submenu_callback,
            instance);
    }
}

static void nfc_scene_read_success_on_enter_emv(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const EmvData* data = nfc_device_get_data(device, NfcProtocolEmv);

    FuriString* temp_str = furi_string_alloc();
    // furi_string_cat_printf(
    //     temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_emv_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static bool nfc_scene_read_menu_on_event_emv(NfcApp* instance, SceneManagerEvent event) {
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexTransactions) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneEmvTransactions);
            consumed = true;
        }
    }

    return consumed;
}

const NfcProtocolSupportBase nfc_protocol_support_emv = {
    .features = NfcProtocolFeatureNone,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_emv,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_emv,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_scene_read_menu_on_enter_emv,
            .on_event = nfc_scene_read_menu_on_event_emv,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_emv,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_saved_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_save_name =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};
