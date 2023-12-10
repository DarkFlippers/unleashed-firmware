#pragma once

#include "iso14443_3a.h"
#include <nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

Iso14443_3aError iso14443_3a_poller_sync_read(Nfc* nfc, Iso14443_3aData* iso14443_3a_data);

#ifdef __cplusplus
}
#endif
