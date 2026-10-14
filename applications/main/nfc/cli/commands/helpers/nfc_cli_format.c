#include "nfc_cli_format.h"

#include <nfc/nfc_device.h>

// The protocol layer already owns every protocol's name, and a protocol cannot be registered
// without one. A second table here only had to be grown by hand, and the two that were missed
// reached printf("%s", NULL) - a hard fault, not a readable crash.
const char* nfc_cli_get_protocol_name(NfcProtocol protocol) {
    return nfc_device_get_protocol_name(protocol);
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
