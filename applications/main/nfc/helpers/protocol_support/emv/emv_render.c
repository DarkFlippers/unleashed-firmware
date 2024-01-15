#include "emv_render.h"

#include "../iso14443_4a/iso14443_4a_render.h"

void nfc_render_emv_info(const EmvData* data, NfcProtocolFormatType format_type, FuriString* str) {
    nfc_render_emv_name(data->emv_application.name, str);
    nfc_render_emv_pan(data->emv_application.pan, data->emv_application.pan_len, str);
    nfc_render_emv_expired(&data->emv_application, str);

    if(format_type == NfcProtocolFormatTypeFull) nfc_render_emv_extra(data, str);
}

void nfc_render_emv_data(const EmvData* data, FuriString* str) {
    nfc_render_emv_pan(data->emv_application.pan, data->emv_application.pan_len, str);
    nfc_render_emv_name(data->emv_application.name, str);
}

void nfc_render_emv_pan(const uint8_t* data, const uint8_t len, FuriString* str) {
    if(len == 0) return;
    for(uint8_t i = 0; i < len; i += 2) {
        furi_string_cat_printf(str, "%02X%02X ", data[i], data[i + 1]);
    }
    furi_string_cat_printf(str, "\n");
}

void nfc_render_emv_expired(const EmvApplication* apl, FuriString* str) {
    if(apl->exp_month == 0) return;
    furi_string_cat_printf(str, "Exp: %02X/%02X\n", apl->exp_month, apl->exp_year);
}

void nfc_render_emv_currency(const EmvApplication* apl, FuriString* str) {
    UNUSED(apl);
    UNUSED(str);
    // nfc/assets/currency_code.nfc
}

void nfc_render_emv_country(const EmvApplication* apl, FuriString* str) {
    UNUSED(apl);
    UNUSED(str);
    // nfc/assets/country_code.nfc
}

void nfc_render_emv_name(const char* data, FuriString* str) {
    if(strlen(data) == 0) return;
    furi_string_cat_printf(str, "\e#");
    furi_string_cat(str, data);
    furi_string_cat_printf(str, "\n");
}

void nfc_render_emv_application(const EmvApplication* apl, FuriString* str) {
    const uint8_t len = apl->aid_len;
    if(len) {
        furi_string_cat_printf(str, "AID: ");
        for(uint8_t i = 0; i < len; i++) furi_string_cat_printf(str, "%02X", apl->aid[i]);
        // nfc/assets/aid.nfc
    } else {
        furi_string_cat_printf(str, "No Pay Application found");
    }
    furi_string_cat_printf(str, "\n");
}

void nfc_render_emv_extra(const EmvData* data, FuriString* str) {
    nfc_render_emv_application(&data->emv_application, str);
}
