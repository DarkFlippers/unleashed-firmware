#include "texkom.h"
#include "texkom_render.h"

#include <nfc/protocols/texkom/texkom_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"

static void nfc_scene_info_on_enter_texkom(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const TexkomData* data = nfc_device_get_data(device, NfcProtocolTexkom);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_texkom_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static NfcCommand nfc_scene_read_poller_callback_texkom(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolTexkom);

    NfcApp* instance = context;
    const TexkomPollerEvent* texkom_event = event.event_data;

    if(texkom_event->type == TexkomPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolTexkom, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static void nfc_scene_read_on_enter_texkom(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_texkom, instance);
}

static void nfc_scene_read_success_on_enter_texkom(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const TexkomData* data = nfc_device_get_data(device, NfcProtocolTexkom);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_texkom_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_emulate_on_enter_texkom(NfcApp* instance) {
    const TexkomData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolTexkom);
    instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolTexkom, data);
    nfc_listener_start(instance->listener, NULL, NULL);
}

const NfcProtocolSupportBase nfc_protocol_support_texkom = {
    // No NfcProtocolFeatureEditUid: that only adds "Edit UID" to the saved-card menu, where
    // rewriting a key you just read off a fob has no use. Add Manually's own "Change UID" is
    // gated on the scene stack instead, so it is unaffected.
    .features = NfcProtocolFeatureEmulateFull,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_texkom,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_texkom,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_texkom,
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
    .scene_emulate =
        {
            .on_enter = nfc_scene_emulate_on_enter_texkom,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_write =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};

NFC_PROTOCOL_SUPPORT_PLUGIN(texkom, NfcProtocolTexkom);
