#pragma once

#include <gui/view.h>
#include "../nfc_types.h"

typedef struct NfcMifareUl NfcMifareUl;

NfcMifareUl* nfc_mifare_ul_alloc(NfcCommon* nfc_common);

void nfc_mifare_ul_free(NfcMifareUl* nfc_mifare_ul);

View* nfc_mifare_ul_get_view(NfcMifareUl* nfc_mifare_ul);
