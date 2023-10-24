#pragma once

#include "mf_ultralight_poller.h"
#include <lib/nfc/protocols/iso14443_3a/iso14443_3a_poller_i.h>
#include <lib/nfc/helpers/nfc_util.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MF_ULTRALIGHT_POLLER_STANDARD_FWT_FC (60000)
#define MF_ULTRALIGHT_MAX_BUFF_SIZE (64)

#define MF_ULTRALIGHT_DEFAULT_PASSWORD (0xffffffffUL)

#define MF_ULTRALIGHT_IS_NTAG_I2C(type)                                                \
    (((type) == MfUltralightTypeNTAGI2C1K) || ((type) == MfUltralightTypeNTAGI2C2K) || \
     ((type) == MfUltralightTypeNTAGI2CPlus1K) || ((type) == MfUltralightTypeNTAGI2CPlus2K))

typedef struct {
    MfUltralightPage page;
    uint8_t page_to_write;
} MfUltralightPollerWritePageCommand;

typedef struct {
    MfUltralightPageReadCommandData data;
    uint8_t start_page;
} MfUltralightPollerReadPageCommand;

typedef struct {
    MfUltralightCounter data;
    uint8_t counter_num;
} MfUltralightPollerReadCounterCommand;

typedef struct {
    MfUltralightTearingFlag data;
    uint8_t tearing_flag_num;
} MfUltralightPollerReadTearingFlagCommand;

typedef union {
    MfUltralightPollerWritePageCommand write_cmd;
    MfUltralightPollerReadPageCommand read_cmd;
    MfUltralightVersion version;
    MfUltralightSignature signature;
    MfUltralightPollerReadCounterCommand counter_cmd;
    MfUltralightPollerReadTearingFlagCommand tearing_flag_cmd;
    MfUltralightData* data;
} MfUltralightPollerContextData;

typedef enum {
    MfUltralightPollerStateIdle,
    MfUltralightPollerStateReadVersion,
    MfUltralightPollerStateDetectMfulC,
    MfUltralightPollerStateDetectNtag203,
    MfUltralightPollerStateGetFeatureSet,
    MfUltralightPollerStateReadSignature,
    MfUltralightPollerStateReadCounters,
    MfUltralightPollerStateReadTearingFlags,
    MfUltralightPollerStateAuth,
    MfUltralightPollerStateReadPages,
    MfUltralightPollerStateTryDefaultPass,
    MfUltralightPollerStateReadFailed,
    MfUltralightPollerStateReadSuccess,

    MfUltralightPollerStateNum,
} MfUltralightPollerState;

struct MfUltralightPoller {
    Iso14443_3aPoller* iso14443_3a_poller;
    MfUltralightPollerState state;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    MfUltralightData* data;
    MfUltralightPollerAuthContext auth_context;
    uint32_t feature_set;
    uint16_t pages_read;
    uint16_t pages_total;
    uint8_t counters_read;
    uint8_t counters_total;
    uint8_t tearing_flag_read;
    uint8_t tearing_flag_total;
    MfUltralightError error;

    NfcGenericEvent general_event;
    MfUltralightPollerEvent mfu_event;
    MfUltralightPollerEventData mfu_event_data;
    NfcGenericCallback callback;
    void* context;
};

MfUltralightError mf_ultralight_process_error(Iso14443_3aError error);

MfUltralightPoller* mf_ultralight_poller_alloc(Iso14443_3aPoller* iso14443_3a_poller);

void mf_ultralight_poller_free(MfUltralightPoller* instance);

const MfUltralightData* mf_ultralight_poller_get_data(MfUltralightPoller* instance);

bool mf_ultralight_poller_ntag_i2c_addr_lin_to_tag(
    MfUltralightPoller* instance,
    uint16_t lin_addr,
    uint8_t* sector,
    uint8_t* tag,
    uint8_t* pages_left);

MfUltralightError mf_ultralight_poller_async_auth_pwd(
    MfUltralightPoller* instance,
    MfUltralightPollerAuthContext* data);

MfUltralightError mf_ultralight_poller_async_authenticate(MfUltralightPoller* instance);

MfUltralightError mf_ultralight_poller_async_read_page(
    MfUltralightPoller* instance,
    uint8_t start_page,
    MfUltralightPageReadCommandData* data);

MfUltralightError mf_ultralight_poller_async_read_page_from_sector(
    MfUltralightPoller* instance,
    uint8_t sector,
    uint8_t tag,
    MfUltralightPageReadCommandData* data);

MfUltralightError mf_ultralight_poller_async_write_page(
    MfUltralightPoller* instance,
    uint8_t page,
    MfUltralightPage* data);

MfUltralightError mf_ultralight_poller_async_read_version(
    MfUltralightPoller* instance,
    MfUltralightVersion* data);

MfUltralightError mf_ultralight_poller_async_read_signature(
    MfUltralightPoller* instance,
    MfUltralightSignature* data);

MfUltralightError mf_ultralight_poller_async_read_counter(
    MfUltralightPoller* instance,
    uint8_t counter_num,
    MfUltralightCounter* data);

MfUltralightError mf_ultralight_poller_async_read_tearing_flag(
    MfUltralightPoller* instance,
    uint8_t tearing_falg_num,
    MfUltralightTearingFlag* data);

#ifdef __cplusplus
}
#endif
