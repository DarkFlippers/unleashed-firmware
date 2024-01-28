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
    furi_string_cat_printf(str, "PAN: ");
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

void nfc_render_emv_transactions(const EmvApplication* apl, FuriString* str) {
    const uint8_t len = apl->active_tr;
    if(!len) {
        return;
    }
    furi_string_cat_printf(str, "Transactions:\n");
    for(int i = 0; i < len; i++) {
        if(!apl->trans[i].amount) continue;
        uint8_t* a = (uint8_t*)&apl->trans[i].amount;
        furi_string_cat_printf(str, "%d: ", apl->trans[i].atc);
        bool top = true;
        for(int x = 0; x < 6; x++) {
            if(x == 5) {
                furi_string_cat_printf(str, ".%02X", a[x]);
                break;
            }
            if(a[x]) {
                if(top) {
                    furi_string_cat_printf(str, "%X", a[x]);
                    top = false;
                } else {
                    furi_string_cat_printf(str, "%02X", a[x]);
                }
            }
        }
        // TODO to string
        furi_string_cat_printf(str, " %x\n", apl->trans[i].currency);

        // TODO to string
        if(apl->trans[i].country)
            furi_string_cat_printf(str, "country: %x\n", apl->trans[i].country);

        if(apl->trans[i].time)
            furi_string_cat_printf(
                str,
                "%02lx:%02lx:%02lx ",
                apl->trans[i].time & 0xff,
                (apl->trans[i].time >> 8) & 0xff,
                apl->trans[i].time >> 16);
        if(apl->trans[i].date)
            furi_string_cat_printf(
                str,
                "%02lx/%02lx/%02lx\n",
                apl->trans[i].date >> 16,
                (apl->trans[i].date >> 8) & 0xff,
                apl->trans[i].date & 0xff);
    }
}

void nfc_render_emv_extra(const EmvData* data, FuriString* str) {
    nfc_render_emv_application(&data->emv_application, str);
    //nfc_render_emv_transactions(&data->emv_application, str);
}
