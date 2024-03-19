#include "iso14443_4b_poller_i.h"

#include <furi.h>

#include "iso14443_4b_i.h"

#define TAG "Iso14443_4bPoller"

Iso14443_4bError iso14443_4b_poller_halt(Iso14443_4bPoller* instance) {
    furi_check(instance);

    iso14443_3b_poller_halt(instance->iso14443_3b_poller);
    instance->poller_state = Iso14443_4bPollerStateIdle;

    return Iso14443_4bErrorNone;
}

Iso14443_4bError iso14443_4b_poller_send_block(
    Iso14443_4bPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer) {
    furi_check(instance);
    furi_check(tx_buffer);
    furi_check(rx_buffer);

    bit_buffer_reset(instance->tx_buffer);
    iso14443_4_layer_encode_block(instance->iso14443_4_layer, tx_buffer, instance->tx_buffer);

    Iso14443_4bError error = Iso14443_4bErrorNone;

    do {
        Iso14443_3bError iso14443_3b_error = iso14443_3b_poller_send_frame(
            instance->iso14443_3b_poller, instance->tx_buffer, instance->rx_buffer);

        if(iso14443_3b_error != Iso14443_3bErrorNone) {
            error = iso14443_4b_process_error(iso14443_3b_error);
            break;

        } else if(!iso14443_4_layer_decode_block(
                      instance->iso14443_4_layer, rx_buffer, instance->rx_buffer)) {
            error = Iso14443_4bErrorProtocol;
            break;
        }
    } while(false);

    return error;
}
