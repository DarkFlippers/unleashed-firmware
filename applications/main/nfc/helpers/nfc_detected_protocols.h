#pragma once

#include <stdint.h>
#include <nfc/protocols/nfc_protocol.h>

typedef struct NfcDetectedProtocols NfcDetectedProtocols;

NfcDetectedProtocols* nfc_detected_protocols_alloc(void);

void nfc_detected_protocols_free(NfcDetectedProtocols* instance);

void nfc_detected_protocols_reset(NfcDetectedProtocols* instance);

void nfc_detected_protocols_select(NfcDetectedProtocols* instance, uint32_t idx);

void nfc_detected_protocols_set(
    NfcDetectedProtocols* instance,
    const NfcProtocol* types,
    uint32_t count);

uint32_t nfc_detected_protocols_get_num(NfcDetectedProtocols* instance);

NfcProtocol nfc_detected_protocols_get_protocol(NfcDetectedProtocols* instance, uint32_t idx);

void nfc_detected_protocols_fill_all_protocols(NfcDetectedProtocols* instance);

NfcProtocol nfc_detected_protocols_get_selected(NfcDetectedProtocols* instance);

uint32_t nfc_detected_protocols_get_selected_idx(NfcDetectedProtocols* instance);
