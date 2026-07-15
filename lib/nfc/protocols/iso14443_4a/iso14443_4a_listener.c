#include "iso14443_4a_listener_i.h"

#include <furi.h>
#include <nfc/protocols/nfc_listener_base.h>

#define TAG "Iso14443_4aListener"

#define ISO14443_4A_LISTENER_BUF_SIZE (256U)

static Iso14443_4aListener*
    iso14443_4a_listener_alloc(Iso14443_3aListener* iso14443_3a_listener, Iso14443_4aData* data) {
    furi_assert(iso14443_3a_listener);

    Iso14443_4aListener* instance = malloc(sizeof(Iso14443_4aListener));
    instance->iso14443_3a_listener = iso14443_3a_listener;
    instance->data = data;
    instance->iso14443_4_layer = iso14443_4_layer_alloc();

    instance->rx_buffer = bit_buffer_alloc(ISO14443_4A_LISTENER_BUF_SIZE);
    instance->tx_buffer = bit_buffer_alloc(ISO14443_4A_LISTENER_BUF_SIZE);

    instance->iso14443_4a_event.data = &instance->iso14443_4a_event_data;
    instance->generic_event.protocol = NfcProtocolIso14443_4a;
    instance->generic_event.instance = instance;
    instance->generic_event.event_data = &instance->iso14443_4a_event;

    return instance;
}

static void iso14443_4a_listener_free(Iso14443_4aListener* instance) {
    furi_assert(instance);

    iso14443_4_layer_free(instance->iso14443_4_layer);
    bit_buffer_free(instance->rx_buffer);
    bit_buffer_free(instance->tx_buffer);
    free(instance);
}

static void iso14443_4a_listener_reset(Iso14443_4aListener* instance) {
    instance->state = Iso14443_4aListenerStateIdle;
    iso14443_4_layer_reset(instance->iso14443_4_layer);
}

static void iso14443_4a_listener_set_callback(
    Iso14443_4aListener* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

static const Iso14443_4aData* iso14443_4a_listener_get_data(Iso14443_4aListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static NfcCommand iso14443_4a_listener_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolIso14443_3a);
    furi_assert(event.event_data);

    Iso14443_4aListener* instance = context;
    Iso14443_3aListenerEvent* iso14443_3a_event = event.event_data;
    BitBuffer* rx_buffer = iso14443_3a_event->data->buffer;
    NfcCommand command = NfcCommandContinue;

    if(iso14443_3a_event->type == Iso14443_3aListenerEventTypeReceivedStandardFrame) {
        if(instance->state == Iso14443_4aListenerStateIdle) {
            if(bit_buffer_get_size_bytes(rx_buffer) == 2 &&
               bit_buffer_get_byte(rx_buffer, 0) == ISO14443_4A_CMD_READ_ATS) {
                if(iso14443_4a_listener_send_ats(instance, &instance->data->ats_data) ==
                   Iso14443_4aErrorNone) {
                    instance->state = Iso14443_4aListenerStateActive;
                    if(iso14443_4a_supports_frame_option(
                           instance->data, Iso14443_4aFrameOptionCid)) {
                        const uint8_t cid = bit_buffer_get_byte(rx_buffer, 1) &
                                            ISO14443_4A_READ_ATS_CID_MASK;
                        iso14443_4_layer_set_cid(instance->iso14443_4_layer, cid);
                    }
                    iso14443_4_layer_set_nad_supported(
                        instance->iso14443_4_layer,
                        iso14443_4a_supports_frame_option(
                            instance->data, Iso14443_4aFrameOptionNad));
                }
            }
        } else {
            Iso14443_4LayerResult status = iso14443_4_layer_decode_command(
                instance->iso14443_4_layer, rx_buffer, instance->rx_buffer);
            if(status & Iso14443_4LayerResultSend) {
                iso14443_3a_listener_send_standard_frame(
                    instance->iso14443_3a_listener, instance->rx_buffer);
            }
            if(status & Iso14443_4LayerResultHalt) {
                iso14443_4a_listener_reset(instance);
                if(instance->callback) {
                    instance->iso14443_4a_event.type = Iso14443_4aListenerEventTypeHalted;
                    instance->callback(instance->generic_event, instance->context);
                }
                command = NfcCommandSleep;
            }
            if(status & Iso14443_4LayerResultData) {
                instance->iso14443_4a_event.type = Iso14443_4aListenerEventTypeReceivedData;
                instance->iso14443_4a_event.data->buffer = instance->rx_buffer;

                if(instance->callback) {
                    command = instance->callback(instance->generic_event, instance->context);
                }
            }
        }
    } else if(
        iso14443_3a_event->type == Iso14443_3aListenerEventTypeHalted ||
        iso14443_3a_event->type == Iso14443_3aListenerEventTypeFieldOff) {
        iso14443_4a_listener_reset(instance);

        instance->iso14443_4a_event.type = iso14443_3a_event->type ==
                                                   Iso14443_3aListenerEventTypeHalted ?
                                               Iso14443_4aListenerEventTypeHalted :
                                               Iso14443_4aListenerEventTypeFieldOff;

        if(instance->callback) {
            command = instance->callback(instance->generic_event, instance->context);
        }
    }

    return command;
}

const NfcListenerBase nfc_listener_iso14443_4a = {
    .alloc = (NfcListenerAlloc)iso14443_4a_listener_alloc,
    .free = (NfcListenerFree)iso14443_4a_listener_free,
    .set_callback = (NfcListenerSetCallback)iso14443_4a_listener_set_callback,
    .get_data = (NfcListenerGetData)iso14443_4a_listener_get_data,
    .run = (NfcListenerRun)iso14443_4a_listener_run,
};
