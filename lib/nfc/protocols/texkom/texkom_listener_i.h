#pragma once

#include "texkom_listener.h"

#include <nfc/nfc_listener.h>

#ifdef __cplusplus
extern "C" {
#endif

struct TexkomListener {
    Nfc* nfc;
    TexkomData* data;
    BitBuffer* tx_buffer;

    NfcGenericCallback callback;
    void* context;
};

#ifdef __cplusplus
}
#endif
