#include "iso15693_3.h"
#include "iso15693_3_render.h"

#include <nfc/protocols/iso15693_3/iso15693_3_poller.h>
#include <nfc/protocols/iso15693_3/iso15693_3_listener.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"

static void nfc_scene_info_on_enter_iso15693_3(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Iso15693_3Data* data = nfc_device_get_data(device, NfcProtocolIso15693_3);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_iso15693_3_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_reset(instance->widget);
    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_more_info_on_enter_iso15693_3(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Iso15693_3Data* data = nfc_device_get_data(device, NfcProtocolIso15693_3);

    FuriString* temp_str = furi_string_alloc();
    nfc_render_iso15693_3_system_info(data, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);
}

static NfcCommand nfc_scene_read_poller_callback_iso15693_3(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso15693_3);

    NfcApp* instance = context;
    const Iso15693_3PollerEvent* iso15693_3_event = event.event_data;

    if(iso15693_3_event->type == Iso15693_3PollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolIso15693_3, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static void nfc_scene_read_on_enter_iso15693_3(NfcApp* instance) {
    UNUSED(instance);
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_iso15693_3, instance);
}

static void nfc_scene_read_success_on_enter_iso15693_3(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const Iso15693_3Data* data = nfc_device_get_data(device, NfcProtocolIso15693_3);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_iso15693_3_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static NfcCommand
    nfc_scene_emulate_listener_callback_iso15693_3(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolIso15693_3);
    furi_assert(event.event_data);

    NfcApp* instance = context;
    Iso15693_3ListenerEvent* iso15693_3_event = event.event_data;

    if(iso15693_3_event->type == Iso15693_3ListenerEventTypeCustomCommand) {
        if(furi_string_size(instance->text_box_store) < NFC_LOG_SIZE_MAX) {
            furi_string_cat_printf(instance->text_box_store, "R:");
            for(size_t i = 0; i < bit_buffer_get_size_bytes(iso15693_3_event->data->buffer); i++) {
                furi_string_cat_printf(
                    instance->text_box_store,
                    " %02X",
                    bit_buffer_get_byte(iso15693_3_event->data->buffer, i));
            }
            furi_string_push_back(instance->text_box_store, '\n');
            view_dispatcher_send_custom_event(
                instance->view_dispatcher, NfcCustomEventListenerUpdate);
        }
    }

    return NfcCommandContinue;
}

static void nfc_scene_emulate_on_enter_iso15693_3(NfcApp* instance) {
    const Iso15693_3Data* data = nfc_device_get_data(instance->nfc_device, NfcProtocolIso15693_3);

    instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolIso15693_3, data);
    nfc_listener_start(
        instance->listener, nfc_scene_emulate_listener_callback_iso15693_3, instance);
}

static NfcCommand
    nfc_scene_write_poller_callback_iso15693_3(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolIso15693_3);

    NfcApp* instance = context;
    Iso15693_3Poller* poller = event.instance;
    const Iso15693_3PollerEvent* iso15693_3_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(iso15693_3_event->type == Iso15693_3PollerEventTypeReady) {
        const Iso15693_3Data* write_data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolIso15693_3);
        const Iso15693_3Data* card_data = iso15693_3_poller_get_data(poller);

        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);

        if(card_data->system_info.block_count != write_data->system_info.block_count ||
           card_data->system_info.block_size != write_data->system_info.block_size) {
            furi_string_set(
                instance->text_box_store, "Incompatible card\n(block count/size\nmismatch)");
            view_dispatcher_send_custom_event(
                instance->view_dispatcher, NfcCustomEventPollerFailure);
        } else {
            Iso15693_3Error error = Iso15693_3ErrorNone;
            const uint8_t block_size = write_data->system_info.block_size;
            const uint8_t* write_block_data =
                (const uint8_t*)simple_array_cget_data(write_data->block_data);
            const uint8_t* card_block_data =
                (const uint8_t*)simple_array_cget_data(card_data->block_data);
            for(uint16_t i = 0; i < write_data->system_info.block_count; i++) {
                if(iso15693_3_is_block_locked(write_data, i)) continue;
                // Skip blocks that already contain the correct data
                if(memcmp(
                       write_block_data + i * block_size,
                       card_block_data + i * block_size,
                       block_size) == 0)
                    continue;
                error = iso15693_3_poller_write_block(
                    poller, write_block_data + i * block_size, i, block_size);
                if(error == Iso15693_3ErrorInternal) {
                    // Block is locked on the target card, skip it
                    error = Iso15693_3ErrorNone;
                    continue;
                }
                if(error != Iso15693_3ErrorNone) break;
            }

            if(error == Iso15693_3ErrorNone) {
                view_dispatcher_send_custom_event(
                    instance->view_dispatcher, NfcCustomEventPollerSuccess);
            } else {
                view_dispatcher_send_custom_event(
                    instance->view_dispatcher, NfcCustomEventPollerFailure);
            }
        }
        command = NfcCommandStop;
    }

    return command;
}

static void nfc_scene_write_on_enter_iso15693_3(NfcApp* instance) {
    furi_string_set(instance->text_box_store, "Apply the\ntarget card now");
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolIso15693_3);
    nfc_poller_start(instance->poller, nfc_scene_write_poller_callback_iso15693_3, instance);
}

const NfcProtocolSupportBase nfc_protocol_support_iso15693_3 = {
    .features = NfcProtocolFeatureEmulateFull | NfcProtocolFeatureEditUid |
                NfcProtocolFeatureMoreInfo | NfcProtocolFeatureWrite,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_iso15693_3,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_scene_more_info_on_enter_iso15693_3,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_iso15693_3,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_iso15693_3,
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
            .on_enter = nfc_scene_emulate_on_enter_iso15693_3,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_write =
        {
            .on_enter = nfc_scene_write_on_enter_iso15693_3,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};

NFC_PROTOCOL_SUPPORT_PLUGIN(iso15693_3, NfcProtocolIso15693_3);
