#pragma once

#include "mf_ultralight.h"
#include <nfc/nfc.h>

#ifdef __cplusplus
extern "C" {
#endif

MfUltralightError mf_ultralight_poller_read_page(Nfc* nfc, uint16_t page, MfUltralightPage* data);

MfUltralightError mf_ultralight_poller_write_page(Nfc* nfc, uint16_t page, MfUltralightPage* data);

MfUltralightError mf_ultralight_poller_read_version(Nfc* nfc, MfUltralightVersion* data);

MfUltralightError mf_ultralight_poller_read_signature(Nfc* nfc, MfUltralightSignature* data);

MfUltralightError
    mf_ultralight_poller_read_counter(Nfc* nfc, uint8_t counter_num, MfUltralightCounter* data);

MfUltralightError mf_ultralight_poller_read_tearing_flag(
    Nfc* nfc,
    uint8_t flag_num,
    MfUltralightTearingFlag* data);

MfUltralightError mf_ultralight_poller_read_card(Nfc* nfc, MfUltralightData* data);

#ifdef __cplusplus
}
#endif
