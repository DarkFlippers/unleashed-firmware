#pragma once

#include <furi.h>
#include <nfc.h>
#include <nfc/protocols/nfc_protocol.h>

typedef struct NfcCliScanner NfcCliScanner;

NfcCliScanner* nfc_cli_scanner_alloc(Nfc* nfc);
void nfc_cli_scanner_free(NfcCliScanner* instance);

bool nfc_cli_scanner_detect_protocol(NfcCliScanner* instance, uint32_t timeout);

void nfc_cli_scanner_begin_scan(NfcCliScanner* instance);
bool nfc_cli_scanner_wait_scan(NfcCliScanner* instance, uint32_t timeout);
void nfc_cli_scanner_end_scan(NfcCliScanner* instance);

void nfc_cli_scanner_list_detected_protocols(NfcCliScanner* instance);
size_t nfc_cli_scanner_detected_protocol_num(NfcCliScanner* instance);
bool nfc_cli_scanner_protocol_was_detected(NfcCliScanner* instance, NfcProtocol protocol);
NfcProtocol nfc_cli_scanner_get_protocol(NfcCliScanner* instance, size_t idx);
