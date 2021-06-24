#pragma once

#include <gui/view.h>
#include "nfc_types.h"

typedef struct NfcEmv NfcEmv;

NfcEmv* nfc_emv_alloc(NfcCommon* nfc_common);

void nfc_emv_free(NfcEmv* nfc_emv);

View* nfc_emv_get_view(NfcEmv* nfc_emv);

void nfc_emv_view_dispatcher_callback(NfcEmv* nfc_emv, NfcMessage* message);
