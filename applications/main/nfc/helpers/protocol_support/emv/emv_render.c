#include "emv_render.h"

#include "../iso14443_4a/iso14443_4a_render.h"

void nfc_render_emv_info(const EmvData* data, NfcProtocolFormatType format_type, FuriString* str) {
    nfc_render_iso14443_4a_brief(emv_get_base_data(data), str);

    nfc_render_emv_pan(data->emv_application.pan, data->emv_application.pan_len, str);
    nfc_render_emv_name(data->emv_application.name, str);

    if(format_type != NfcProtocolFormatTypeFull) return;

    furi_string_cat(str, "\n\e#ISO14443-4 data");
    nfc_render_iso14443_4a_extra(emv_get_base_data(data), str);
}

void nfc_render_emv_data(const EmvData* data, FuriString* str) {
    nfc_render_emv_pan(data->emv_application.pan, data->emv_application.pan_len, str);
    nfc_render_emv_name(data->emv_application.name, str);
}

void nfc_render_emv_pan(const uint8_t* data, const uint8_t len, FuriString* str) {
    for(uint8_t i = 0; i < len; i++) furi_string_cat_printf(str, "%u", data[i]);
    furi_string_cat_printf(str, "\n");
}

void nfc_render_emv_name(const char* data, FuriString* str) {
    UNUSED(data);
    furi_string_cat_printf(str, "\n");
}

void nfc_render_emv_application(const EmvApplication* data, FuriString* str) {
    UNUSED(data);
    furi_string_cat_printf(str, "\n");
}