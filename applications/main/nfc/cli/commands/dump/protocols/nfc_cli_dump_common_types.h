#pragma once

#include <furi.h>

#include "../../../../helpers/mf_classic_key_cache.h"
#include "../../helpers/nfc_cli_scanner.h"

#include <nfc/nfc.h>
#include <nfc/protocols/nfc_protocol.h>
#include <nfc/nfc_device.h>
#include <nfc/nfc_poller.h>

#include <nfc/protocols/felica/felica.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>

#include <storage/storage.h>

#define NFC_CLI_DUMP_KEY_MAX_SIZE (16)

typedef union {
    MfUltralightAuthPassword password;
    FelicaCardKey felica_key;
    MfUltralightC3DesAuthKey tdes_key;
    uint8_t key[NFC_CLI_DUMP_KEY_MAX_SIZE];
} NfcCliDumpKeyUnion;

typedef struct {
    NfcCliDumpKeyUnion key;
    uint8_t key_size;
    bool skip_auth;
} NfcCliDumpAuthContext;

typedef enum {
    NfcCliDumpErrorNone,
    NfcCliDumpErrorTimeout,
    NfcCliDumpErrorNotPresent,
    NfcCliDumpErrorFailedToRead,
    NfcCliDumpErrorAuthFailed,

    NfcCliDumpErrorNum,
} NfcCliDumpError;

typedef struct {
    Nfc* nfc;
    FuriString* file_path;
    Storage* storage;
    NfcCliScanner* scanner;
    NfcProtocol desired_protocol;
    uint32_t timeout;
    FuriSemaphore* sem_done;

    NfcCliDumpError result;

    NfcCliDumpAuthContext auth_ctx;
    MfClassicKeyCache* mfc_key_cache;

    NfcPoller* poller;
    NfcDevice* nfc_device;
} NfcCliDumpContext;
