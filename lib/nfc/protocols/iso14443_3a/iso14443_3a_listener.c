#include "iso14443_3a_listener_i.h"

#include "nfc/protocols/nfc_listener_base.h"
#include "nfc/helpers/iso14443_crc.h"

#include <furi.h>
#include <lib/nfc/nfc.h>

#define TAG "Iso14443_3aListener"

#define ISO14443_3A_LISTENER_MAX_BUFFER_SIZE (256)

static bool iso14443_3a_listener_halt_received(BitBuffer* buf) {
    bool halt_cmd_received = false;

    do {
        if(bit_buffer_get_size_bytes(buf) != 4) break;
        if(!iso14443_crc_check(Iso14443CrcTypeA, buf)) break;
        if(bit_buffer_get_byte(buf, 0) != 0x50) break;
        if(bit_buffer_get_byte(buf, 1) != 0x00) break;
        halt_cmd_received = true;
    } while(false);

    return halt_cmd_received;
}

Iso14443_3aListener* iso14443_3a_listener_alloc(Nfc* nfc, Iso14443_3aData* data) {
    furi_assert(nfc);

    Iso14443_3aListener* instance = malloc(sizeof(Iso14443_3aListener));
    instance->nfc = nfc;
    instance->data = data;
    instance->tx_buffer = bit_buffer_alloc(ISO14443_3A_LISTENER_MAX_BUFFER_SIZE);

    instance->iso14443_3a_event.data = &instance->iso14443_3a_event_data;
    instance->generic_event.protocol = NfcProtocolIso14443_3a;
    instance->generic_event.instance = instance;
    instance->generic_event.event_data = &instance->iso14443_3a_event;

    nfc_set_fdt_listen_fc(instance->nfc, ISO14443_3A_FDT_LISTEN_FC);
    nfc_config(instance->nfc, NfcModeListener, NfcTechIso14443a);
    nfc_iso14443a_listener_set_col_res_data(
        instance->nfc,
        instance->data->uid,
        instance->data->uid_len,
        instance->data->atqa,
        instance->data->sak);

    return instance;
}

void iso14443_3a_listener_free(Iso14443_3aListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);
    furi_assert(instance->tx_buffer);

    bit_buffer_free(instance->tx_buffer);
    free(instance);
}

void iso14443_3a_listener_set_callback(
    Iso14443_3aListener* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

const Iso14443_3aData* iso14443_3a_listener_get_data(Iso14443_3aListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

NfcCommand iso14443_3a_listener_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolInvalid);
    furi_assert(event.event_data);

    Iso14443_3aListener* instance = context;
    NfcEvent* nfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    if(nfc_event->type == NfcEventTypeListenerActivated) {
        instance->state = Iso14443_3aListenerStateActive;
    } else if(nfc_event->type == NfcEventTypeFieldOff) {
        instance->state = Iso14443_3aListenerStateIdle;
        if(instance->callback) {
            instance->iso14443_3a_event.type = Iso14443_3aListenerEventTypeFieldOff;
            instance->callback(instance->generic_event, instance->context);
        }
        command = NfcCommandSleep;
    } else if(nfc_event->type == NfcEventTypeRxEnd) {
        if(iso14443_3a_listener_halt_received(nfc_event->data.buffer)) {
            if(instance->callback) {
                instance->iso14443_3a_event.type = Iso14443_3aListenerEventTypeHalted;
                instance->callback(instance->generic_event, instance->context);
            }
            command = NfcCommandSleep;
        } else {
            if(iso14443_crc_check(Iso14443CrcTypeA, nfc_event->data.buffer)) {
                instance->iso14443_3a_event.type =
                    Iso14443_3aListenerEventTypeReceivedStandardFrame;
                iso14443_crc_trim(nfc_event->data.buffer);
            } else {
                instance->iso14443_3a_event.type = Iso14443_3aListenerEventTypeReceivedData;
            }
            instance->iso14443_3a_event_data.buffer = nfc_event->data.buffer;
            if(instance->callback) {
                command = instance->callback(instance->generic_event, instance->context);
            }
        }
    }

    return command;
}

const NfcListenerBase nfc_listener_iso14443_3a = {
    .alloc = (NfcListenerAlloc)iso14443_3a_listener_alloc,
    .free = (NfcListenerFree)iso14443_3a_listener_free,
    .set_callback = (NfcListenerSetCallback)iso14443_3a_listener_set_callback,
    .get_data = (NfcListenerGetData)iso14443_3a_listener_get_data,
    .run = (NfcListenerRun)iso14443_3a_listener_run,
};
