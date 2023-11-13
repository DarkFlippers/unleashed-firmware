#pragma once

#include "nfc_listener_base.h"
#include "nfc_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const NfcListenerBase* nfc_listeners_api[NfcProtocolNum];

#ifdef __cplusplus
}
#endif
