#pragma once

#include "iso14443_3a_poller.h"

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISO14443_3A_POLLER_MAX_BUFFER_SIZE (512U)

#define ISO14443_3A_POLLER_SEL_CMD(cascade_lvl) (0x93 + 2 * (cascade_lvl))
#define ISO14443_3A_POLLER_SEL_PAR(bytes, bits) (((bytes) << 4 & 0xf0U) | ((bits) & 0x0fU))
#define ISO14443_3A_POLLER_SDD_CL               (0x88U)

typedef enum {
    Iso14443_3aPollerColResStateStateIdle,
    Iso14443_3aPollerColResStateStateNewCascade,
    Iso14443_3aPollerColResStateStateSelectCascade,
    Iso14443_3aPollerColResStateStateSuccess,
    Iso14443_3aPollerColResStateStateFail,
} Iso14443_3aPollerColResState;

typedef struct {
    Iso14443_3aPollerColResState state;
    Iso14443_3aSensResp sens_resp;
    Iso14443_3aSddReq sdd_req;
    Iso14443_3aSddResp sdd_resp;
    Iso14443_3aSelReq sel_req;
    Iso14443_3aSelResp sel_resp;
    uint8_t cascade_level;
} Iso14443_3aPollerColRes;

typedef enum {
    Iso14443_3aPollerStateIdle,
    Iso14443_3aPollerStateColResInProgress,
    Iso14443_3aPollerStateColResFailed,
    Iso14443_3aPollerStateActivated,
} Iso14443_3aPollerState;

struct Iso14443_3aPoller {
    Nfc* nfc;
    Iso14443_3aPollerState state;
    Iso14443_3aPollerColRes col_res;
    Iso14443_3aData* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;

    NfcGenericEvent general_event;
    Iso14443_3aPollerEvent iso14443_3a_event;
    Iso14443_3aPollerEventData iso14443_3a_event_data;
    NfcGenericCallback callback;
    void* context;
};

const Iso14443_3aData* iso14443_3a_poller_get_data(Iso14443_3aPoller* instance);

#ifdef __cplusplus
}
#endif
