#pragma once

#include "mf_desfire_poller.h"

#include <lib/nfc/protocols/iso14443_4a/iso14443_4a_poller_i.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MfDesfirePollerStateIdle,
    MfDesfirePollerStateReadVersion,
    MfDesfirePollerStateReadFreeMemory,
    MfDesfirePollerStateReadMasterKeySettings,
    MfDesfirePollerStateReadMasterKeyVersion,
    MfDesfirePollerStateReadApplicationIds,
    MfDesfirePollerStateReadApplications,
    MfDesfirePollerStateReadFailed,
    MfDesfirePollerStateReadSuccess,

    MfDesfirePollerStateNum,
} MfDesfirePollerState;

typedef enum {
    MfDesfirePollerSessionStateIdle,
    MfDesfirePollerSessionStateActive,
    MfDesfirePollerSessionStateStopRequest,
} MfDesfirePollerSessionState;

struct MfDesfirePoller {
    Iso14443_4aPoller* iso14443_4a_poller;
    MfDesfirePollerSessionState session_state;
    MfDesfirePollerState state;
    MfDesfireError error;
    MfDesfireData* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    BitBuffer* input_buffer;
    BitBuffer* result_buffer;
    MfDesfirePollerEventData mf_desfire_event_data;
    MfDesfirePollerEvent mf_desfire_event;
    NfcGenericEvent general_event;
    NfcGenericCallback callback;
    void* context;
};

MfDesfireError mf_desfire_process_error(Iso14443_4aError error);

const MfDesfireData* mf_desfire_poller_get_data(MfDesfirePoller* instance);

MfDesfireError mf_desfire_send_chunks(
    MfDesfirePoller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer);

MfDesfireError
    mf_desfire_poller_async_read_version(MfDesfirePoller* instance, MfDesfireVersion* data);

MfDesfireError
    mf_desfire_poller_async_read_free_memory(MfDesfirePoller* instance, MfDesfireFreeMemory* data);

MfDesfireError mf_desfire_poller_async_read_key_settings(
    MfDesfirePoller* instance,
    MfDesfireKeySettings* data);

MfDesfireError mf_desfire_poller_async_read_key_versions(
    MfDesfirePoller* instance,
    SimpleArray* data,
    uint32_t count);

MfDesfireError
    mf_desfire_poller_async_read_application_ids(MfDesfirePoller* instance, SimpleArray* data);

MfDesfireError mf_desfire_poller_async_select_application(
    MfDesfirePoller* instance,
    const MfDesfireApplicationId* id);

MfDesfireError mf_desfire_poller_async_read_file_ids(MfDesfirePoller* instance, SimpleArray* data);

MfDesfireError mf_desfire_poller_async_read_file_settings(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    MfDesfireFileSettings* data);

MfDesfireError mf_desfire_poller_async_read_file_settings_multi(
    MfDesfirePoller* instance,
    const SimpleArray* file_ids,
    SimpleArray* data);

MfDesfireError mf_desfire_poller_async_read_file_data(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    uint32_t offset,
    size_t size,
    MfDesfireFileData* data);

MfDesfireError mf_desfire_poller_async_read_file_value(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    MfDesfireFileData* data);

MfDesfireError mf_desfire_poller_async_read_file_records(
    MfDesfirePoller* instance,
    MfDesfireFileId id,
    uint32_t offset,
    size_t size,
    MfDesfireFileData* data);

MfDesfireError mf_desfire_poller_async_read_file_data_multi(
    MfDesfirePoller* instance,
    const SimpleArray* file_ids,
    const SimpleArray* file_settings,
    SimpleArray* data);

MfDesfireError
    mf_desfire_poller_async_read_application(MfDesfirePoller* instance, MfDesfireApplication* data);

MfDesfireError mf_desfire_poller_async_read_applications(
    MfDesfirePoller* instance,
    const SimpleArray* app_ids,
    SimpleArray* data);

#ifdef __cplusplus
}
#endif
