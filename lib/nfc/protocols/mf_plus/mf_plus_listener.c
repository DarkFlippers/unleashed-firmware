#include "mf_plus_listener_i.h"
#include "mf_plus_listener_defs.h"
#include "mf_plus_i.h"

#include <furi.h>

#define TAG "MfPlusListener"

#define MF_PLUS_LISTENER_BUF_SIZE (64U)

// SL3 command bytes handled card-side (mirror of the poller).
#define MF_PLUS_CMD_AUTH_FIRST    (0x70)
#define MF_PLUS_CMD_AUTH_CONTINUE (0x72)
#define MF_PLUS_CMD_READ_ENC      (0x31)
#define MF_PLUS_CMD_READ_SIG      (0x3C)

void mf_plus_listener_reset_session(MfPlusListener* instance) {
    furi_assert(instance);

    instance->state = MfPlusListenerStateIdle;
    memset(&instance->session, 0, sizeof(instance->session));
    memset(instance->rnd_b, 0, sizeof(instance->rnd_b));
    memset(&instance->auth_key, 0, sizeof(instance->auth_key));
}

static MfPlusListener*
    mf_plus_listener_alloc(Iso14443_4aListener* iso14443_4a_listener, MfPlusData* data) {
    furi_assert(iso14443_4a_listener);
    furi_assert(data);

    MfPlusListener* instance = malloc(sizeof(MfPlusListener));
    instance->iso14443_4a_listener = iso14443_4a_listener;
    instance->data = data;
    instance->tx_buffer = bit_buffer_alloc(MF_PLUS_LISTENER_BUF_SIZE);

    instance->mfp_event.data = &instance->mfp_event_data;
    instance->generic_event.protocol = NfcProtocolMfPlus;
    instance->generic_event.instance = instance;
    instance->generic_event.event_data = &instance->mfp_event;

    mf_plus_listener_reset_session(instance);

    return instance;
}

static void mf_plus_listener_free(MfPlusListener* instance) {
    furi_assert(instance);
    furi_assert(instance->tx_buffer);

    bit_buffer_free(instance->tx_buffer);
    free(instance);
}

static void mf_plus_listener_set_callback(
    MfPlusListener* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

static const MfPlusData* mf_plus_listener_get_data(const MfPlusListener* instance) {
    furi_assert(instance);
    furi_assert(instance->data);

    return instance->data;
}

static NfcCommand mf_plus_listener_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    // The parent transport is ISO14443-4A (NOT ISO15693, as the type_4_tag template mistakenly
    // asserts) -- MfPlus is registered as a child of NfcProtocolIso14443_4a.
    furi_assert(event.protocol == NfcProtocolIso14443_4a);
    furi_assert(event.event_data);

    MfPlusListener* instance = context;
    Iso14443_4aListenerEvent* iso14443_4a_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    switch(iso14443_4a_event->type) {
    case Iso14443_4aListenerEventTypeFieldOff:
        mf_plus_listener_reset_session(instance);
        command = NfcCommandSleep;
        break;
    case Iso14443_4aListenerEventTypeHalted:
        mf_plus_listener_reset_session(instance);
        break;
    case Iso14443_4aListenerEventTypeReceivedData: {
        const BitBuffer* rx_buffer = iso14443_4a_event->data->buffer;
        if(bit_buffer_get_size_bytes(rx_buffer) == 0) break;

        const uint8_t cmd = bit_buffer_get_byte(rx_buffer, 0);
        switch(cmd) {
        case MF_PLUS_CMD_AUTH_FIRST:
            command = mf_plus_listener_auth_first_handler(instance, rx_buffer);
            break;
        case MF_PLUS_CMD_AUTH_CONTINUE:
            command = mf_plus_listener_auth_continue_handler(instance, rx_buffer);
            break;
        case MF_PLUS_CMD_READ_ENC:
            command = mf_plus_listener_read_encrypted_handler(instance, rx_buffer);
            break;
        case MF_PLUS_CMD_READ_SIG:
            command = mf_plus_listener_read_signature_handler(instance, rx_buffer);
            break;
        default:
            // Unhandled command (GetVersion, WritePerso probe, ...): stay silent so the reader
            // times out and falls back (the poller identifies via ATS when GetVersion is unanswered).
            FURI_LOG_D(TAG, "Unhandled command 0x%02X", cmd);
            break;
        }
        break;
    }
    default:
        break;
    }

    return command;
}

const NfcListenerBase mf_plus_listener = {
    .alloc = (NfcListenerAlloc)mf_plus_listener_alloc,
    .free = (NfcListenerFree)mf_plus_listener_free,
    .set_callback = (NfcListenerSetCallback)mf_plus_listener_set_callback,
    .get_data = (NfcListenerGetData)mf_plus_listener_get_data,
    .run = (NfcListenerRun)mf_plus_listener_run,
};
