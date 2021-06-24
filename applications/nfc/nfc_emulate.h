#pragma once

#include <gui/view.h>
#include "nfc_types.h"

typedef struct NfcEmulate NfcEmulate;

NfcEmulate* nfc_emulate_alloc(NfcCommon* nfc_common);

void nfc_emulate_free(NfcEmulate* nfc_emulate);

View* nfc_emulate_get_view(NfcEmulate* nfc_emulate);
