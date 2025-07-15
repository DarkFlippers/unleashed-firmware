#include "type_4_tag_listener_i.h"
#include "type_4_tag_listener_defs.h"
#include "type_4_tag_i.h"

#define TAG "Type4TagListener"

static void type_4_tag_listener_reset_state(Type4TagListener* instance) {
    instance->state = Type4TagListenerStateIdle;
}

static Type4TagListener*
    type_4_tag_listener_alloc(Iso14443_4aListener* iso14443_4a_listener, Type4TagData* data) {
    furi_assert(iso14443_4a_listener);

    Type4TagListener* instance = malloc(sizeof(Type4TagListener));
    instance->iso14443_4a_listener = iso14443_4a_listener;
    instance->data = data;

    instance->tx_buffer = bit_buffer_alloc(TYPE_4_TAG_BUF_SIZE);

    instance->type_4_tag_event.data = &instance->type_4_tag_event_data;
    instance->generic_event.protocol = NfcProtocolType4Tag;
    instance->generic_event.instance = instance;
    instance->generic_event.event_data = &instance->type_4_tag_event;

    return instance;
}

static void type_4_tag_listener_free(Type4TagListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);
    furi_assert(instance->tx_buffer);

    bit_buffer_free(instance->tx_buffer);
    free(instance);
}

static void type_4_tag_listener_set_callback(
    Type4TagListener* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

static const Type4TagData* type_4_tag_listener_get_data(Type4TagListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static NfcCommand type_4_tag_listener_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolIso15693_3);
    furi_assert(event.event_data);

    Type4TagListener* instance = context;
    Iso14443_4aListenerEvent* iso14443_4a_event = event.event_data;
    BitBuffer* rx_buffer = iso14443_4a_event->data->buffer;
    NfcCommand command = NfcCommandContinue;

    if(iso14443_4a_event->type == Iso14443_4aListenerEventTypeFieldOff) {
        type_4_tag_listener_reset_state(instance);
        command = NfcCommandSleep;
    } else if(iso14443_4a_event->type == Iso14443_4aListenerEventTypeHalted) {
        type_4_tag_listener_reset_state(instance);
    } else if(iso14443_4a_event->type == Iso14443_4aListenerEventTypeReceivedData) {
        const Type4TagError error = type_4_tag_listener_handle_apdu(instance, rx_buffer);
        if(error == Type4TagErrorCustomCommand && instance->callback) {
            instance->type_4_tag_event.type = Type4TagListenerEventTypeCustomCommand;
            instance->type_4_tag_event.data->buffer = rx_buffer;
            command = instance->callback(instance->generic_event, instance->context);
        }
    }

    return command;
}

const NfcListenerBase nfc_listener_type_4_tag = {
    .alloc = (NfcListenerAlloc)type_4_tag_listener_alloc,
    .free = (NfcListenerFree)type_4_tag_listener_free,
    .set_callback = (NfcListenerSetCallback)type_4_tag_listener_set_callback,
    .get_data = (NfcListenerGetData)type_4_tag_listener_get_data,
    .run = (NfcListenerRun)type_4_tag_listener_run,
};
