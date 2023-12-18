#pragma once

#include "st25tb.h"
#include <lib/nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct St25tbPoller St25tbPoller;

typedef enum {
    St25tbPollerEventTypeReady,
    St25tbPollerEventTypeRequestMode,
    St25tbPollerEventTypeFailure,
    St25tbPollerEventTypeSuccess,
} St25tbPollerEventType;

typedef struct {
    St25tbType type;
} St25tbPollerReadyData;

typedef enum {
    St25tbPollerModeRead,
    St25tbPollerModeWrite,

    St25tbPollerModeNum,
} St25tbPollerMode;

typedef struct {
    uint8_t block_number;
    uint32_t block_data;
} St25tbPollerEventDataModeRequestWriteParams;

typedef union {
    St25tbPollerEventDataModeRequestWriteParams write_params;
} St25tbPollerEventDataModeRequestParams;

typedef struct {
    St25tbPollerMode mode;
    St25tbPollerEventDataModeRequestParams params;
} St25tbPollerEventDataModeRequest;

typedef union {
    St25tbPollerReadyData ready;
    St25tbPollerEventDataModeRequest mode_request;
    St25tbError error;
} St25tbPollerEventData;

typedef struct {
    St25tbPollerEventType type;
    St25tbPollerEventData* data;
} St25tbPollerEvent;

St25tbError st25tb_poller_send_frame(
    St25tbPoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

St25tbError st25tb_poller_initiate(St25tbPoller* instance, uint8_t* chip_id_ptr);

St25tbError st25tb_poller_select(St25tbPoller* instance, uint8_t* chip_id_ptr);

St25tbError st25tb_poller_get_uid(St25tbPoller* instance, uint8_t* uid);

St25tbError
    st25tb_poller_read_block(St25tbPoller* instance, uint32_t* block, uint8_t block_number);

St25tbError
    st25tb_poller_write_block(St25tbPoller* instance, uint32_t block, uint8_t block_number);

St25tbError st25tb_poller_halt(St25tbPoller* instance);

#ifdef __cplusplus
}
#endif
