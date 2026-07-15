#include "nfc_cli_scanner.h"
#include <nfc/nfc_scanner.h>
#include "nfc_cli_format.h"

#define NFC_CLI_SCANNER_FLAG_DETECTED (1UL << 0)

struct NfcCliScanner {
    Nfc* nfc;
    size_t protocols_detected_num;
    NfcProtocol protocols_detected[NfcProtocolNum];
    FuriThreadId thread_id;
    NfcScanner* scanner;
};

NfcCliScanner* nfc_cli_scanner_alloc(Nfc* nfc) {
    NfcCliScanner* instance = malloc(sizeof(NfcCliScanner));
    instance->nfc = nfc;
    instance->thread_id = furi_thread_get_current_id();
    return instance;
}

void nfc_cli_scanner_free(NfcCliScanner* instance) {
    furi_assert(instance);
    free(instance);
}

static void nfc_cli_scanner_detect_callback(NfcScannerEvent event, void* context) {
    furi_assert(context);
    NfcCliScanner* instance = context;

    if(event.type == NfcScannerEventTypeDetected) {
        instance->protocols_detected_num = event.data.protocol_num;
        memcpy(
            instance->protocols_detected,
            event.data.protocols,
            event.data.protocol_num * sizeof(NfcProtocol));
        furi_thread_flags_set(instance->thread_id, NFC_CLI_SCANNER_FLAG_DETECTED);
    }
}

bool nfc_cli_scanner_detect_protocol(NfcCliScanner* instance, uint32_t timeout) {
    instance->scanner = nfc_scanner_alloc(instance->nfc);
    nfc_scanner_start(instance->scanner, nfc_cli_scanner_detect_callback, instance);
    uint32_t event =
        furi_thread_flags_wait(NFC_CLI_SCANNER_FLAG_DETECTED, FuriFlagWaitAny, timeout);
    nfc_scanner_stop(instance->scanner);
    nfc_scanner_free(instance->scanner);
    return (event == NFC_CLI_SCANNER_FLAG_DETECTED);
}

void nfc_cli_scanner_begin_scan(NfcCliScanner* instance) {
    instance->scanner = nfc_scanner_alloc(instance->nfc);
    nfc_scanner_start(instance->scanner, nfc_cli_scanner_detect_callback, instance);
}

bool nfc_cli_scanner_wait_scan(NfcCliScanner* instance, uint32_t timeout) {
    UNUSED(instance);
    uint32_t event =
        furi_thread_flags_wait(NFC_CLI_SCANNER_FLAG_DETECTED, FuriFlagWaitAny, timeout);
    return (event == NFC_CLI_SCANNER_FLAG_DETECTED);
}

void nfc_cli_scanner_end_scan(NfcCliScanner* instance) {
    nfc_scanner_stop(instance->scanner);
    nfc_scanner_free(instance->scanner);
}

void nfc_cli_scanner_list_detected_protocols(NfcCliScanner* instance) {
    printf("Protocols detected: ");
    size_t n = instance->protocols_detected_num;
    for(size_t i = 0; i < n; i++) {
        const char* name = nfc_cli_get_protocol_name(instance->protocols_detected[i]);
        printf((i == (n - 1)) ? "%s\r\n" : "%s, ", name);
    }
}

bool nfc_cli_scanner_protocol_was_detected(NfcCliScanner* instance, NfcProtocol protocol) {
    furi_assert(instance);
    furi_assert(protocol < NfcProtocolNum);

    for(size_t i = 0; i < instance->protocols_detected_num; i++) {
        if(instance->protocols_detected[i] == protocol) return true;
    }
    return false;
}

NfcProtocol nfc_cli_scanner_get_protocol(NfcCliScanner* instance, size_t idx) {
    furi_assert(instance);
    furi_assert(idx < instance->protocols_detected_num);
    return instance->protocols_detected[idx];
}

size_t nfc_cli_scanner_detected_protocol_num(NfcCliScanner* instance) {
    furi_assert(instance);
    return instance->protocols_detected_num;
}
