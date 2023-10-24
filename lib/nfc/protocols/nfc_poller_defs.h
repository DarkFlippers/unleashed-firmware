#pragma once

#include "nfc_poller_base.h"
#include "nfc_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const NfcPollerBase* nfc_pollers_api[NfcProtocolNum];

#ifdef __cplusplus
}
#endif
