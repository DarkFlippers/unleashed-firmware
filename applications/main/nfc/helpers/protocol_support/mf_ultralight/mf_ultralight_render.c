#include "mf_ultralight_render.h"

#include "../iso14443_3a/iso14443_3a_render.h"

static void nfc_render_mf_ultralight_pages_count(const MfUltralightData* data, FuriString* str) {
    furi_string_cat_printf(str, "\nPages Read: %u/%u", data->pages_read, data->pages_total);
    if(data->pages_read != data->pages_total) {
        furi_string_cat_printf(str, "\nPassword-protected pages!");
    }
}

static void nfc_render_mf_ultralight_counters(const MfUltralightData* data, FuriString* str) {
    for(uint8_t i = 0; i < MF_ULTRALIGHT_COUNTER_NUM; i++)
        furi_string_cat_printf(str, "\nCounter %u: %lu", i, data->counter[i].counter);
}

static void nfc_render_mf_ultralight_pwd_pack_lines(
    const MfUltralightConfigPages* config,
    FuriString* str) {
    furi_string_cat_printf(str, "\nPassword: ");
    nfc_render_iso14443_3a_format_bytes(
        str, config->password.data, MF_ULTRALIGHT_AUTH_PASSWORD_SIZE);

    furi_string_cat_printf(str, "\nPACK: ");
    nfc_render_iso14443_3a_format_bytes(str, config->pack.data, MF_ULTRALIGHT_AUTH_PACK_SIZE);
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
        nfc_render_mf_ultralight_pwd_pack_lines(config, str);
    } else {
        furi_string_cat_printf(str, "\nThis card does not support\npassword protection!");
    }

    nfc_render_mf_ultralight_pages_count(data, str);
}

void nfc_render_mf_ultralight_pwd_pack_if_read(const MfUltralightData* data, FuriString* str) {
    // Only when the dump actually captured them (see mf_ultralight_is_pwd_pack_read).
    MfUltralightConfigPages* config = NULL;
    if(mf_ultralight_is_pwd_pack_read(data) && mf_ultralight_get_config_page(data, &config)) {
        nfc_render_mf_ultralight_pwd_pack_lines(config, str);
    }
}

// A recovered UL-AES DataProtKey is stashed at pages 0x30-0x33 (real cards read the key pages as
// zero) in card byte order (reverse of the key value); reverse it back to show the actual key. A
// genuine all-zero key can't be told apart from "no key recovered" without carrying an extra flag
// through save/load, so an all-zero key is not displayed here.
static void nfc_render_mf_ultralight_aes_key(const MfUltralightData* data, FuriString* str) {
    const uint8_t* stored = data->page[MF_ULTRALIGHT_AES_DATA_KEY_PAGE].data;
    uint8_t key[MF_ULTRALIGHT_AES_KEY_SIZE];
    bool has_key = false;
    for(size_t i = 0; i < MF_ULTRALIGHT_AES_KEY_SIZE; i++) {
        key[i] = stored[MF_ULTRALIGHT_AES_KEY_SIZE - 1 - i];
        if(key[i] != 0) has_key = true;
    }
    if(has_key) {
        furi_string_cat_printf(str, "\nDataProtKey: ");
        nfc_render_iso14443_3a_format_bytes(str, key, MF_ULTRALIGHT_AES_KEY_SIZE);
    }
}

void nfc_render_mf_ultralight_info(
    const MfUltralightData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    // UL-AES has no counters and no MfUltralightConfigPages, so show identity + pages read + the
    // recovered key, skipping the counter/PWD/PACK lines. The Short (read-result) form of
    // iso14443_3a_info omits the Tech line, so add it explicitly to match MIFARE Plus.
    if(data->type == MfUltralightTypeUltralightAES) {
        if(format_type != NfcProtocolFormatTypeFull) {
            nfc_render_iso14443_tech_type(data->iso14443_3a_data, str);
        }
        nfc_render_iso14443_3a_info(data->iso14443_3a_data, format_type, str);
        // Inline pages line (not the shared helper, whose "Password-protected pages!" note is wrong
        // for an AES-protected card).
        furi_string_cat_printf(str, "\nPages Read: %u/%u", data->pages_read, data->pages_total);
        if(data->pages_read != data->pages_total) {
            furi_string_cat_printf(str, "\nAES-protected pages!");
        }
        nfc_render_mf_ultralight_aes_key(data, str);
        return;
    }

    nfc_render_iso14443_3a_info(data->iso14443_3a_data, format_type, str);

    nfc_render_mf_ultralight_pages_count(data, str);

    nfc_render_mf_ultralight_counters(data, str);

    // PWD/PACK is a verbose extra, like the other Full-only fields.
    if(format_type == NfcProtocolFormatTypeFull) {
        nfc_render_mf_ultralight_pwd_pack_if_read(data, str);
    }
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
