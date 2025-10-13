#pragma once

#include <furi.h>
#include <nfc/nfc.h>
#include <nfc/nfc_poller.h>

typedef enum {
    NfcCliRawErrorNone,
    NfcCliRawErrorTimeout,
    NfcCliRawErrorNotPresent,
    NfcCliRawErrorWrongCrc,
    NfcCliRawErrorProtocol,
} NfcCliRawError;

typedef struct {
    bool select;
    bool keep_field;
    bool append_crc;
    NfcProtocol protocol;
    BitBuffer* tx_buffer;
    uint32_t timeout;
} NfcCliRawRequest;

typedef struct {
    NfcCliRawError result;
    BitBuffer* rx_buffer;
    FuriString* activation_string;
} NfcCliRawResponse;
