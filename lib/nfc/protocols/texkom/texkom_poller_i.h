#pragma once

#include "texkom_poller.h"
#include "texkom_i.h"

#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

struct TexkomPoller {
    Nfc* nfc;
    TexkomData* data;
    BitBuffer* rx_buffer;

    NfcGenericEvent general_event;
    TexkomPollerEvent texkom_event;
    NfcGenericCallback callback;
    void* context;
};

const TexkomData* texkom_poller_get_data(TexkomPoller* instance);

#ifdef __cplusplus
}
#endif
