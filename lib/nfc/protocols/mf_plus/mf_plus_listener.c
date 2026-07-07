#include "mf_plus_listener_i.h"
#include "mf_plus_listener_defs.h"
#include "mf_plus_i.h"

#include <furi.h>

#define TAG "MfPlusListener"

#define MF_PLUS_LISTENER_BUF_SIZE (64U)

// SL3 command bytes handled card-side (mirror of the poller).
#define MF_PLUS_CMD_AUTH_FIRST       (0x70)
#define MF_PLUS_CMD_AUTH_CONTINUE    (0x72)
#define MF_PLUS_CMD_READ_ENC         (0x31)
#define MF_PLUS_CMD_READ_PLAIN       (0x33)
#define MF_PLUS_CMD_READ_SIG         (0x3C)
#define MF_PLUS_CMD_WRITE_ENC        (0xA1)
#define MF_PLUS_CMD_WRITE_PLAIN      (0xA3)
#define MF_PLUS_CMD_WRITE_PERSO      (0xA8)
// GetVersion (0x60, from mf_plus.h) is chained with 0xAF and can arrive ISO7816-wrapped (0x90 0x60).
#define MF_PLUS_CMD_ADDITIONAL_FRAME (0xAF)
#define MF_PLUS_ISO_CLA              (0x90)

void mf_plus_listener_reset_session(MfPlusListener* instance) {
    furi_assert(instance);

    instance->state = MfPlusListenerStateIdle;
    memset(&instance->session, 0, sizeof(instance->session));
    memset(instance->rnd_b, 0, sizeof(instance->rnd_b));
    memset(&instance->auth_key, 0, sizeof(instance->auth_key));
    instance->get_version_stage = 0;
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
        const size_t rx_size = bit_buffer_get_size_bytes(rx_buffer);
        if(rx_size == 0) break;

        const uint8_t cmd = bit_buffer_get_byte(rx_buffer, 0);

        // GetVersion arrives native (0x60, then 0xAF continuations) or ISO7816-wrapped (0x90 0x60,
        // then 0x90 0xAF); peek the inner command byte to route both and to keep the multi-frame
        // chain alive across 0xAF. Any other command aborts an in-flight chain (below).
        const bool wrapped = (cmd == MF_PLUS_ISO_CLA && rx_size >= 2);
        const uint8_t inner = wrapped ? bit_buffer_get_byte(rx_buffer, 1) : cmd;
        if(inner == MF_PLUS_CMD_GET_VERSION) {
            command = mf_plus_listener_get_version_handler(instance, wrapped);
            break;
        }
        if(inner == MF_PLUS_CMD_ADDITIONAL_FRAME && instance->get_version_stage > 0) {
            command = mf_plus_listener_get_version_continue_handler(instance, wrapped);
            break;
        }
        instance->get_version_stage = 0;

        switch(cmd) {
        case MF_PLUS_CMD_AUTH_FIRST:
            command = mf_plus_listener_auth_first_handler(instance, rx_buffer);
            break;
        case MF_PLUS_CMD_AUTH_CONTINUE:
            command = mf_plus_listener_auth_continue_handler(instance, rx_buffer);
            break;
        case MF_PLUS_CMD_READ_ENC:
            command = mf_plus_listener_read_handler(instance, rx_buffer, false);
            break;
        case MF_PLUS_CMD_READ_PLAIN:
            command = mf_plus_listener_read_handler(instance, rx_buffer, true);
            break;
        case MF_PLUS_CMD_READ_SIG:
            command = mf_plus_listener_read_signature_handler(instance, rx_buffer);
            break;
        case MF_PLUS_CMD_WRITE_ENC:
            command = mf_plus_listener_write_handler(instance, rx_buffer, true);
            break;
        case MF_PLUS_CMD_WRITE_PLAIN:
            command = mf_plus_listener_write_handler(instance, rx_buffer, false);
            break;
        case MF_PLUS_CMD_WRITE_PERSO:
            command = mf_plus_listener_write_perso_handler(instance, rx_buffer);
            break;
        default:
            // Unimplemented command (non-first auth 0x76, wrapped non-GetVersion 0x90 xx, ...): NAK
            // it like a real card so a reader's info scan gets a response instead of timing out.
            // (GetVersion 0x60 is routed above -- served for EV1/EV2, NAKed for EV0.)
            FURI_LOG_D(TAG, "Unsupported command 0x%02X", cmd);
            command = mf_plus_listener_unsupported_handler(instance, rx_buffer);
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
