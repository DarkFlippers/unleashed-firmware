#include "ntag4xx.h"
#include "ntag4xx_render.h"

#include <nfc/protocols/ntag4xx/ntag4xx_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"
#include "../iso14443_4a/iso14443_4a_i.h"

static void nfc_scene_info_on_enter_ntag4xx(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Ntag4xxData* data = nfc_device_get_data(device, NfcProtocolNtag4xx);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_ntag4xx_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_more_info_on_enter_ntag4xx(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Ntag4xxData* data = nfc_device_get_data(device, NfcProtocolNtag4xx);

    furi_string_reset(instance->text_box_store);
    nfc_render_ntag4xx_data(data, instance->text_box_store);

    text_box_set_font(instance->text_box, TextBoxFontHex);
    text_box_set_text(instance->text_box, furi_string_get_cstr(instance->text_box_store));

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewTextBox);
}

static NfcCommand nfc_scene_read_poller_callback_ntag4xx(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolNtag4xx);

    NfcCommand command = NfcCommandContinue;

    NfcApp* instance = context;
    const Ntag4xxPollerEvent* ntag4xx_event = event.event_data;

    if(ntag4xx_event->type == Ntag4xxPollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolNtag4xx, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    } else if(ntag4xx_event->type == Ntag4xxPollerEventTypeReadFailed) {
        command = NfcCommandReset;
    }

    return command;
}

static void nfc_scene_read_on_enter_ntag4xx(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_ntag4xx, instance);
}

static void nfc_scene_read_success_on_enter_ntag4xx(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Ntag4xxData* data = nfc_device_get_data(device, NfcProtocolNtag4xx);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_ntag4xx_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_emulate_on_enter_ntag4xx(NfcApp* instance) {
    const Iso14443_4aData* iso14443_4a_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolIso14443_4a);

    instance->listener =
        nfc_listener_alloc(instance->nfc, NfcProtocolIso14443_4a, iso14443_4a_data);
    nfc_listener_start(
        instance->listener, nfc_scene_emulate_listener_callback_iso14443_4a, instance);
}

const NfcProtocolSupportBase nfc_protocol_support_ntag4xx = {
    .features = NfcProtocolFeatureEmulateUid | NfcProtocolFeatureMoreInfo,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_ntag4xx,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_scene_more_info_on_enter_ntag4xx,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_ntag4xx,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_ntag4xx,
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
            .on_enter = nfc_scene_emulate_on_enter_ntag4xx,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_write =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};
