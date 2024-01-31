#include "iso14443_4a.h"
#include "iso14443_4a_render.h"

#include <nfc/protocols/iso14443_4a/iso14443_4a_poller.h>
#include <nfc/protocols/iso14443_4a/iso14443_4a_listener.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"

static void nfc_scene_info_on_enter_iso14443_4a(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Iso14443_4aData* data = nfc_device_get_data(device, NfcProtocolIso14443_4a);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_iso14443_4a_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static NfcCommand
    nfc_scene_read_poller_callback_iso14443_4a(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso14443_4a);

    NfcApp* instance = context;
    const Iso14443_4aPollerEvent* iso14443_4a_event = event.event_data;

    if(iso14443_4a_event->type == Iso14443_4aPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolIso14443_4a, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static void nfc_scene_read_on_enter_iso14443_4a(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_iso14443_4a, instance);
}

static void nfc_scene_read_success_on_enter_iso14443_4a(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Iso14443_4aData* data = nfc_device_get_data(device, NfcProtocolIso14443_4a);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_iso14443_4a_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_saved_menu_on_enter_iso14443_4a(NfcApp* instance) {
    UNUSED(instance);
}

NfcCommand nfc_scene_emulate_listener_callback_iso14443_4a(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolIso14443_4a);
    furi_assert(event.event_data);

    NfcApp* nfc = context;
    Iso14443_4aListenerEvent* iso14443_4a_event = event.event_data;

    if(iso14443_4a_event->type == Iso14443_4aListenerEventTypeReceivedData) {
        if(furi_string_size(nfc->text_box_store) < NFC_LOG_SIZE_MAX) {
            furi_string_cat_printf(nfc->text_box_store, "R:");
            for(size_t i = 0; i < bit_buffer_get_size_bytes(iso14443_4a_event->data->buffer);
                i++) {
                furi_string_cat_printf(
                    nfc->text_box_store,
                    " %02X",
                    bit_buffer_get_byte(iso14443_4a_event->data->buffer, i));
            }
            furi_string_push_back(nfc->text_box_store, '\n');
            view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventListenerUpdate);
        }
    }

    return NfcCommandContinue;
}

static void nfc_scene_emulate_on_enter_iso14443_4a(NfcApp* instance) {
    const Iso14443_4aData* data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolIso14443_4a);

    instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolIso14443_4a, data);
    nfc_listener_start(
        instance->listener, nfc_scene_emulate_listener_callback_iso14443_4a, instance);
}

static bool nfc_scene_read_menu_on_event_iso14443_4a(NfcApp* instance, SceneManagerEvent event) {
    if(event.type == SceneManagerEventTypeCustom && event.event == SubmenuIndexCommonEmulate) {
        scene_manager_next_scene(instance->scene_manager, NfcSceneEmulate);
        return true;
    }

    return false;
}

const NfcProtocolSupportBase nfc_protocol_support_iso14443_4a = {
    .features = NfcProtocolFeatureEmulateUid | NfcProtocolFeatureEditUid,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_iso14443_4a,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_iso14443_4a,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_scene_read_menu_on_event_iso14443_4a,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_iso14443_4a,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_saved_menu =
        {
            .on_enter = nfc_scene_saved_menu_on_enter_iso14443_4a,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_save_name =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_emulate =
        {
            .on_enter = nfc_scene_emulate_on_enter_iso14443_4a,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};
