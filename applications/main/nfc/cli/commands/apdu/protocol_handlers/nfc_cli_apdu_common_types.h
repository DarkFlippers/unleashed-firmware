#pragma once

#include <furi.h>
#include <nfc/nfc.h>
#include <nfc/nfc_poller.h>

typedef enum {
    NfcCliApduErrorNone,
    NfcCliApduErrorTimeout,
    NfcCliApduErrorNotPresent,
    NfcCliApduErrorWrongCrc,
    NfcCliApduErrorProtocol,
} NfcCliApduError;

typedef struct {
    NfcProtocol protocol;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;
    NfcCliApduError result;
} NfcCliApduRequestResponse;
