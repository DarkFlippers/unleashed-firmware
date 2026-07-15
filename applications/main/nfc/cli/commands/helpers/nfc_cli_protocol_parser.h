#pragma once

#include <furi.h>
#include <nfc/protocols/nfc_protocol.h>

typedef struct {
    const char* name;
    NfcProtocol value;
} NfcProtocolNameValuePair;

typedef struct NfcCliProtocolParser NfcCliProtocolParser;

NfcCliProtocolParser* nfc_cli_protocol_parser_alloc(
    const NfcProtocolNameValuePair* valid_protocols,
    const size_t valid_count);

void nfc_cli_protocol_parser_free(NfcCliProtocolParser* instance);
bool nfc_cli_protocol_parser_get(
    NfcCliProtocolParser* instance,
    FuriString* key,
    NfcProtocol* result);
