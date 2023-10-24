#pragma once

#include "protocols/st25tb/st25tb.h"
#include "st25tb_poller.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ST25TB_POLLER_MAX_BUFFER_SIZE (16U)

typedef enum {
    St25tbPollerStateIdle,
    St25tbPollerStateInitiateInProgress,
    St25tbPollerStateInitiateFailed,
    St25tbPollerStateActivationInProgress,
    St25tbPollerStateActivationFailed,
    St25tbPollerStateActivated,
} St25tbPollerState;

struct St25tbPoller {
    Nfc* nfc;
    St25tbPollerState state;
    St25tbData* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;

    NfcGenericEvent general_event;
    St25tbPollerEvent st25tb_event;
    St25tbPollerEventData st25tb_event_data;
    NfcGenericCallback callback;
    void* context;
};

const St25tbData* st25tb_poller_get_data(St25tbPoller* instance);

St25tbError st25tb_poller_async_initiate(St25tbPoller* instance, uint8_t* chip_id);

St25tbError st25tb_poller_async_activate(St25tbPoller* instance, St25tbData* data);

St25tbError st25tb_poller_async_get_uid(St25tbPoller* instance, uint8_t uid[ST25TB_UID_SIZE]);

St25tbError
    st25tb_poller_async_read_block(St25tbPoller* instance, uint32_t* block, uint8_t block_number);

St25tbError st25tb_poller_halt(St25tbPoller* instance);

St25tbError st25tb_poller_send_frame(
    St25tbPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

#ifdef __cplusplus
}
#endif
