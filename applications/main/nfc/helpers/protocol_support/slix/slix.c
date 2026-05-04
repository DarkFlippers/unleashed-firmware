#include "slix.h"
#include "slix_render.h"

#include <nfc/protocols/slix/slix_poller.h>
#include <nfc/protocols/slix/slix_listener.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"

#include <string.h>

#define TAG "SlixWrite"

static void nfc_scene_info_on_enter_slix(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const SlixData* data = nfc_device_get_data(device, NfcProtocolSlix);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_slix_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_reset(instance->widget);
    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static void nfc_scene_more_info_on_enter_slix(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const SlixData* data = nfc_device_get_data(device, NfcProtocolSlix);

    FuriString* temp_str = furi_string_alloc();
    nfc_render_iso15693_3_system_info(slix_get_base_data(data), temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);
}

static NfcCommand nfc_scene_read_poller_callback_slix(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolSlix);

    NfcApp* instance = context;
    const SlixPollerEvent* slix_event = event.event_data;

    if(slix_event->type == SlixPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolSlix, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static void nfc_scene_read_on_enter_slix(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_slix, instance);
}

static void nfc_scene_read_success_on_enter_slix(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const SlixData* data = nfc_device_get_data(device, NfcProtocolSlix);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_slix_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static NfcCommand nfc_scene_emulate_listener_callback_slix(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolSlix);
    furi_assert(event.event_data);

    NfcApp* instance = context;
    SlixListenerEvent* slix_event = event.event_data;

    if(slix_event->type == SlixListenerEventTypeCustomCommand) {
        if(furi_string_size(instance->text_box_store) < NFC_LOG_SIZE_MAX) {
            furi_string_cat_printf(instance->text_box_store, "R:");
            for(size_t i = 0; i < bit_buffer_get_size_bytes(slix_event->data->buffer); i++) {
                furi_string_cat_printf(
                    instance->text_box_store,
                    " %02X",
                    bit_buffer_get_byte(slix_event->data->buffer, i));
            }
            furi_string_push_back(instance->text_box_store, '\n');
            view_dispatcher_send_custom_event(
                instance->view_dispatcher, NfcCustomEventListenerUpdate);
        }
    }

    return NfcCommandContinue;
}

static void nfc_scene_emulate_on_enter_slix(NfcApp* instance) {
    const SlixData* data = nfc_device_get_data(instance->nfc_device, NfcProtocolSlix);

    instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolSlix, data);
    nfc_listener_start(instance->listener, nfc_scene_emulate_listener_callback_slix, instance);
}

static NfcCommand nfc_scene_write_poller_callback_slix(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolSlix);

    NfcApp* instance = context;
    SlixPoller* poller = event.instance;
    const SlixPollerEvent* slix_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(slix_event->type == SlixPollerEventTypeReady) {
        const SlixData* write_data = nfc_device_get_data(instance->nfc_device, NfcProtocolSlix);
        const Iso15693_3Data* write_iso_data = slix_get_base_data(write_data);
        const Iso15693_3Data* card_iso_data =
            slix_get_base_data((const SlixData*)nfc_poller_get_data(instance->poller));

        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);

        if(card_iso_data->system_info.block_count != write_iso_data->system_info.block_count ||
           card_iso_data->system_info.block_size != write_iso_data->system_info.block_size) {
            furi_string_set(
                instance->text_box_store, "Incompatible card\n(block count/size\nmismatch)");
            view_dispatcher_send_custom_event(
                instance->view_dispatcher, NfcCustomEventPollerFailure);
        } else {
            SlixError error = SlixErrorNone;
            uint16_t failed_block = 0;
            const uint8_t block_size = write_iso_data->system_info.block_size;
            const uint8_t* write_data_ptr =
                (const uint8_t*)simple_array_cget_data(write_iso_data->block_data);
            const uint8_t* card_data_ptr =
                (const uint8_t*)simple_array_cget_data(card_iso_data->block_data);
            for(uint16_t i = 0; i < write_iso_data->system_info.block_count; i++) {
                if(iso15693_3_is_block_locked(write_iso_data, i)) continue;
                // Skip blocks that already contain the correct data
                if(memcmp(
                       write_data_ptr + i * block_size,
                       card_data_ptr + i * block_size,
                       block_size) == 0)
                    continue;
                error = slix_poller_write_block(
                    poller, write_data_ptr + i * block_size, i, block_size);
                if(error == SlixErrorInternal) {
                    // Block is locked on the target card, skip it
                    error = SlixErrorNone;
                    continue;
                }
                if(error != SlixErrorNone) {
                    failed_block = i;
                    FURI_LOG_E(TAG, "Write failed: block %u, error %d", failed_block, (int)error);
                    break;
                }
            }

            if(error == SlixErrorNone) {
                view_dispatcher_send_custom_event(
                    instance->view_dispatcher, NfcCustomEventPollerSuccess);
            } else {
                const char* err_name;
                switch(error) {
                case SlixErrorTimeout:
                    err_name = "Timeout";
                    break;
                case SlixErrorFormat:
                    err_name = "BadCRC";
                    break;
                case SlixErrorNotSupported:
                    err_name = "NotSupported";
                    break;
                case SlixErrorInternal:
                    err_name = "Locked";
                    break;
                case SlixErrorWrongPassword:
                    err_name = "WrongPwd";
                    break;
                default:
                    err_name = "Unknown";
                    break;
                }
                furi_string_printf(
                    instance->text_box_store, "Block %u: %s", failed_block, err_name);
                view_dispatcher_send_custom_event(
                    instance->view_dispatcher, NfcCustomEventPollerFailure);
            }
        }
        command = NfcCommandStop;
    }

    return command;
}

static void nfc_scene_write_on_enter_slix(NfcApp* instance) {
    furi_string_set(instance->text_box_store, "Apply the\ntarget card now");
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolSlix);
    nfc_poller_start(instance->poller, nfc_scene_write_poller_callback_slix, instance);
}

const NfcProtocolSupportBase nfc_protocol_support_slix = {
    .features = NfcProtocolFeatureEmulateFull | NfcProtocolFeatureMoreInfo |
                NfcProtocolFeatureWrite,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_slix,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_more_info =
        {
            .on_enter = nfc_scene_more_info_on_enter_slix,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_slix,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_slix,
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
            .on_enter = nfc_scene_emulate_on_enter_slix,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_write =
        {
            .on_enter = nfc_scene_write_on_enter_slix,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};

NFC_PROTOCOL_SUPPORT_PLUGIN(slix, NfcProtocolSlix);
