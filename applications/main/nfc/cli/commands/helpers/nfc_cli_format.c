#include "nfc_cli_format.h"

static const char* protocol_names[NfcProtocolNum] = {
    [NfcProtocolIso14443_3a] = "Iso14443-3a",
    [NfcProtocolIso14443_3b] = "Iso14443-3b",
    [NfcProtocolIso14443_4a] = "Iso14443-4a",
    [NfcProtocolIso14443_4b] = "Iso14443-4b",
    [NfcProtocolIso15693_3] = "Iso15693-3",
    [NfcProtocolFelica] = "FeliCa",
    [NfcProtocolMfUltralight] = "Mifare Ultralight",
    [NfcProtocolMfClassic] = "Mifare Classic",
    [NfcProtocolMfDesfire] = "Mifare DESFire",
    [NfcProtocolMfPlus] = "Mifare Plus",
    [NfcProtocolSlix] = "Slix",
    [NfcProtocolSt25tb] = "St25tb",
};

const char* nfc_cli_get_protocol_name(NfcProtocol protocol) {
    furi_assert(protocol < NfcProtocolNum);
    return protocol_names[protocol];
}

static const char* mf_ultralight_error_names[] = {
    [MfUltralightErrorNone] = "OK",
    [MfUltralightErrorNotPresent] = "Card not present",
    [MfUltralightErrorProtocol] = "Protocol failure",
    [MfUltralightErrorAuth] = "Auth failed",
    [MfUltralightErrorTimeout] = "Timeout",
};

const char* nfc_cli_mf_ultralight_get_error(MfUltralightError error) {
    furi_assert(error < COUNT_OF(mf_ultralight_error_names));
    return mf_ultralight_error_names[error];
}

void nfc_cli_format_array(
    const uint8_t* data,
    const size_t data_size,
    const char* header,
    FuriString* output) {
    furi_assert(data);
    furi_assert(data_size > 0);
    furi_assert(header);
    furi_assert(output);

    furi_string_cat_printf(output, "%s", header);
    for(size_t i = 0; i < data_size; i++) {
        furi_string_cat_printf(output, "%02X ", data[i]);
    }
}

void nfc_cli_printf_array(const uint8_t* data, const size_t data_size, const char* header) {
    furi_assert(data);
    furi_assert(data_size > 0);
    furi_assert(header);

    printf("%s", header);
    for(size_t i = 0; i < data_size; i++) {
        printf("%02X ", data[i]);
    }
}
