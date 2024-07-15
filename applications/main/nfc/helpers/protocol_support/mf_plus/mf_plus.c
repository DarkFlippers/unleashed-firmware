#include "mf_plus.h"
#include "mf_plus_render.h"

#include <nfc/protocols/mf_plus/mf_plus_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"
#include "../iso14443_4a/iso14443_4a_i.h"

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
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    } else if(mf_plus_event->type == MfPlusPollerEventTypeReadFailed) {
        command = NfcCommandReset;
    }

    return command;
}

static void nfc_scene_read_on_enter_mf_plus(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_mf_plus, instance);
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
    const Iso14443_4aData* iso14443_4a_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolIso14443_4a);

    instance->listener =
        nfc_listener_alloc(instance->nfc, NfcProtocolIso14443_4a, iso14443_4a_data);
    nfc_listener_start(
        instance->listener, nfc_scene_emulate_listener_callback_iso14443_4a, instance);
}

const NfcProtocolSupportBase nfc_protocol_support_mf_plus = {
    .features = NfcProtocolFeatureEmulateUid,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_mf_plus,
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
            .on_enter = nfc_scene_emulate_on_enter_mf_plus,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};
