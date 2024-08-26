#include "mf_ultralight_render.h"

#include "../iso14443_3a/iso14443_3a_render.h"

static void nfc_render_mf_ultralight_pages_count(const MfUltralightData* data, FuriString* str) {
    furi_string_cat_printf(str, "\nPages Read: %u/%u", data->pages_read, data->pages_total);
    if(data->pages_read != data->pages_total) {
        furi_string_cat_printf(str, "\nPassword-protected pages!");
    }
}

void nfc_render_mf_ultralight_pwd_pack(const MfUltralightData* data, FuriString* str) {
    MfUltralightConfigPages* config;

    bool all_pages = mf_ultralight_is_all_data_read(data);
    bool has_config = mf_ultralight_get_config_page(data, &config);

    if(!has_config) {
        furi_string_cat_printf(str, "\e#Already Unlocked!");
    } else if(all_pages) {
        furi_string_cat_printf(str, "\e#All Pages Are Unlocked!");
    } else {
        furi_string_cat_printf(str, "\e#Some Pages Are Locked!");
    }

    if(has_config) {
        furi_string_cat_printf(str, "\nPassword: ");
        nfc_render_iso14443_3a_format_bytes(
            str, config->password.data, MF_ULTRALIGHT_AUTH_PASSWORD_SIZE);

        furi_string_cat_printf(str, "\nPACK: ");
        nfc_render_iso14443_3a_format_bytes(str, config->pack.data, MF_ULTRALIGHT_AUTH_PACK_SIZE);
    } else {
        furi_string_cat_printf(str, "\nThis card does not support\npassword protection!");
    }

    nfc_render_mf_ultralight_pages_count(data, str);
}

void nfc_render_mf_ultralight_info(
    const MfUltralightData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    nfc_render_iso14443_3a_info(data->iso14443_3a_data, format_type, str);

    nfc_render_mf_ultralight_pages_count(data, str);
}

void nfc_render_mf_ultralight_dump(const MfUltralightData* data, FuriString* str) {
    furi_string_cat_printf(str, "\e*");
    for(size_t i = 0; i < data->pages_read; i++) {
        const uint8_t* page_data = data->page[i].data;
        for(size_t j = 0; j < MF_ULTRALIGHT_PAGE_SIZE; j += 2) {
            furi_string_cat_printf(str, " %02X%02X", page_data[j], page_data[j + 1]);
        }
    }
}
