#include "iso14443_4b.h"
#include "iso14443_4b_render.h"

#include <nfc/protocols/iso14443_4b/iso14443_4b_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"
#include "../iso14443_3b/iso14443_3b_i.h"

static void nfc_scene_info_on_enter_iso14443_4b(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Iso14443_4bData* data = nfc_device_get_data(device, NfcProtocolIso14443_4b);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_iso14443_4b_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static NfcCommand
    nfc_scene_read_poller_callback_iso14443_4b(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_4b);

    NfcApp* instance = context;
    const Iso14443_4bPollerEvent* iso14443_4b_event = event.event_data;

    if(iso14443_4b_event->type == Iso14443_4bPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolIso14443_4b, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static void nfc_scene_read_on_enter_iso14443_4b(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_iso14443_4b, instance);
}

static void nfc_scene_read_success_on_enter_iso14443_4b(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Iso14443_4bData* data = nfc_device_get_data(device, NfcProtocolIso14443_4b);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_iso14443_4b_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_saved_menu_on_enter_iso14443_4b(NfcApp* instance) {
    UNUSED(instance);
}

static bool nfc_scene_read_menu_on_event_iso14443_4b(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom && event.event == SubmenuIndexCommonEmulate) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneEmulate);
        return true;
    }

    return false;
}

static bool nfc_scene_saved_menu_on_event_iso14443_4b(NfcApp* instance, SceneManagerEvent event) {
    return nfc_scene_saved_menu_on_event_iso14443_3b_common(instance, event);
}

const NfcProtocolSupportBase nfc_protocol_support_iso14443_4b = {
    .features = NfcProtocolFeatureNone,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_iso14443_4b,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_iso14443_4b,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_scene_read_menu_on_event_iso14443_4b,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_iso14443_4b,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_saved_menu =
        {
            .on_enter = nfc_scene_saved_menu_on_enter_iso14443_4b,
            .on_event = nfc_scene_saved_menu_on_event_iso14443_4b,
        },
    .scene_save_name =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_emulate =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};
