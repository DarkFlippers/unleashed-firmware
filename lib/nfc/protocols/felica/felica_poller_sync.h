#pragma once

#include "felica.h"
#include <nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

FelicaError felica_poller_sync_read(Nfc* nfc, FelicaData* data, const FelicaCardKey* card_key);

#ifdef __cplusplus
}
#endif
