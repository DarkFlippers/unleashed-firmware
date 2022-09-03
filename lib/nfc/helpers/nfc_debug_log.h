#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct NfcDebugLog NfcDebugLog;

NfcDebugLog* nfc_debug_log_alloc();

void nfc_debug_log_free(NfcDebugLog* instance);

void nfc_debug_log_process_data(
    NfcDebugLog* instance,
    uint8_t* data,
    uint16_t len,
    bool reader_to_tag,
    bool crc_dropped);
