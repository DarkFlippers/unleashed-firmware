#pragma once

#include "../nfc_cli_raw_common_types.h"

NfcCommand nfc_cli_raw_felica_handler(
    NfcGenericInstance* poller,
    const NfcCliRawRequest* request,
    NfcCliRawResponse* const response);
