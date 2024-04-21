#include "mf_desfire.h"
#include "mf_desfire_render.h"

#include <nfc/protocols/mf_desfire/mf_desfire_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"
#include "../iso14443_4a/iso14443_4a_i.h"

static void nfc_scene_info_on_enter_mf_desfire(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfDesfireData* data = nfc_device_get_data(device, NfcProtocolMfDesfire);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    furi_string_replace(temp_str, "Mifare", "MIFARE");
    nfc_render_mf_desfire_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_more_info_on_enter_mf_desfire(NfcApp* instance) {
    // Jump to advanced scene right away
    scene_manager_next_scene(instance->scene_manager, NfcSceneMfDesfireMoreInfo);
}

static NfcCommand nfc_scene_read_poller_callback_mf_desfire(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolMfDesfire);

    NfcCommand command = NfcCommandContinue;

    NfcApp* instance = context;
    const MfDesfirePollerEvent* mf_desfire_event = event.event_data;

    if(mf_desfire_event->type == MfDesfirePollerEventTypeReadSuccess) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolMfDesfire, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    } else if(mf_desfire_event->type == MfDesfirePollerEventTypeReadFailed) {
        command = NfcCommandReset;
    }

    return command;
}

static void nfc_scene_read_on_enter_mf_desfire(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_mf_desfire, instance);
}

static void nfc_scene_read_success_on_enter_mf_desfire(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const MfDesfireData* data = nfc_device_get_data(device, NfcProtocolMfDesfire);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    furi_string_replace(temp_str, "Mifare", "MIFARE");
    nfc_render_mf_desfire_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_emulate_on_enter_mf_desfire(NfcApp* instance) {
    const Iso14443_4aData* iso14443_4a_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolIso14443_4a);

    instance->listener =
        nfc_listener_alloc(instance->nfc, NfcProtocolIso14443_4a, iso14443_4a_data);
    nfc_listener_start(
        instance->listener, nfc_scene_emulate_listener_callback_iso14443_4a, instance);
}

const NfcProtocolSupportBase nfc_protocol_support_mf_desfire = {
    .features = NfcProtocolFeatureEmulateUid | NfcProtocolFeatureMoreInfo,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_mf_desfire,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_scene_more_info_on_enter_mf_desfire,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_mf_desfire,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_mf_desfire,
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
            .on_enter = nfc_scene_emulate_on_enter_mf_desfire,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};
