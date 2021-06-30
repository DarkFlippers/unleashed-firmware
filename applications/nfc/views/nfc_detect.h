#pragma once

#include <gui/view.h>
#include "../nfc_types.h"

typedef struct NfcDetect NfcDetect;

NfcDetect* nfc_detect_alloc(NfcCommon* nfc_common);

void nfc_detect_free(NfcDetect* nfc_detect);

View* nfc_detect_get_view(NfcDetect* nfc_detect);
