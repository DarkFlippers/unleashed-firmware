#pragma once

#include "iso15693_3_poller.h"

#include "iso15693_3_i.h"

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISO15693_3_POLLER_MAX_BUFFER_SIZE (64U)

typedef enum {
    Iso15693_3PollerStateIdle,
    Iso15693_3PollerStateColResInProgress,
    Iso15693_3PollerStateColResFailed,
    Iso15693_3PollerStateActivated,
} Iso15693_3PollerState;

struct Iso15693_3Poller {
    Nfc* nfc;
    Iso15693_3PollerState state;
    Iso15693_3Data* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;

    NfcGenericEvent general_event;
    Iso15693_3PollerEvent iso15693_3_event;
    Iso15693_3PollerEventData iso15693_3_event_data;
    NfcGenericCallback callback;
    void* context;
};

const Iso15693_3Data* iso15693_3_poller_get_data(Iso15693_3Poller* instance);

Iso15693_3Error iso15693_3_poller_async_activate(Iso15693_3Poller* instance, Iso15693_3Data* data);

Iso15693_3Error iso15693_3_poller_async_inventory(Iso15693_3Poller* instance, uint8_t* uid);

Iso15693_3Error
    iso15693_3_poller_async_get_system_info(Iso15693_3Poller* instance, Iso15693_3SystemInfo* data);

Iso15693_3Error iso15693_3_poller_async_read_block(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint8_t block_number,
    uint8_t block_size);

Iso15693_3Error iso15693_3_poller_async_read_blocks(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint16_t block_count,
    uint8_t block_size);

Iso15693_3Error iso15693_3_poller_async_get_blocks_security(
    Iso15693_3Poller* instance,
    uint8_t* data,
    uint16_t block_count);

Iso15693_3Error iso15693_3_poller_send_frame(
    Iso15693_3Poller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

#ifdef __cplusplus
}
#endif
