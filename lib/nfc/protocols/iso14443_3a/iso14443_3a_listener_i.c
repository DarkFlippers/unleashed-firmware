#include "iso14443_3a_listener_i.h"

#include "nfc/helpers/iso14443_crc.h"

#define TAG "Iso14443_3aListener"

static Iso14443_3aError iso14443_3a_listener_process_nfc_error(NfcError error) {
    Iso14443_3aError ret = Iso14443_3aErrorNone;

    if(error == NfcErrorNone) {
        ret = Iso14443_3aErrorNone;
    } else if(error == NfcErrorTimeout) {
        ret = Iso14443_3aErrorTimeout;
    } else {
        ret = Iso14443_3aErrorFieldOff;
    }

    return ret;
}

Iso14443_3aError
    iso14443_3a_listener_tx(Iso14443_3aListener* instance, const BitBuffer* tx_buffer) {
    furi_assert(instance);
    furi_assert(tx_buffer);

    Iso14443_3aError ret = Iso14443_3aErrorNone;
    NfcError error = nfc_listener_tx(instance->nfc, tx_buffer);
    if(error != NfcErrorNone) {
        FURI_LOG_W(TAG, "Tx error: %d", error);
        ret = iso14443_3a_listener_process_nfc_error(error);
    }

    return ret;
}

Iso14443_3aError iso14443_3a_listener_tx_with_custom_parity(
    Iso14443_3aListener* instance,
    const BitBuffer* tx_buffer) {
    furi_assert(instance);
    furi_assert(tx_buffer);

    Iso14443_3aError ret = Iso14443_3aErrorNone;
    NfcError error = nfc_iso14443a_listener_tx_custom_parity(instance->nfc, tx_buffer);
    if(error != NfcErrorNone) {
        FURI_LOG_W(TAG, "Tx error: %d", error);
        ret = iso14443_3a_listener_process_nfc_error(error);
    }

    return ret;
}

Iso14443_3aError iso14443_3a_listener_send_standard_frame(
    Iso14443_3aListener* instance,
    const BitBuffer* tx_buffer) {
    furi_assert(instance);
    furi_assert(tx_buffer);
    furi_assert(instance->tx_buffer);

    Iso14443_3aError ret = Iso14443_3aErrorNone;
    do {
        bit_buffer_copy(instance->tx_buffer, tx_buffer);
        iso14443_crc_append(Iso14443CrcTypeA, instance->tx_buffer);

        NfcError error = nfc_listener_tx(instance->nfc, instance->tx_buffer);
        if(error != NfcErrorNone) {
            FURI_LOG_W(TAG, "Tx error: %d", error);
            ret = iso14443_3a_listener_process_nfc_error(error);
            break;
        }
    } while(false);

    return ret;
}
