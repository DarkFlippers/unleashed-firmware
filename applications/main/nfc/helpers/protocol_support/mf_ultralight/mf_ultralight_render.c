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
    uint8_t key[MF_ULTRALIGHT_AES_KEY_SIZE];
    mf_ultralight_aes_get_key(data, key);
    bool has_key = false;
    for(size_t i = 0; i < MF_ULTRALIGHT_AES_KEY_SIZE; i++) {
        if(key[i] != 0) {
            has_key = true;
            break;
        }
    }
    if(has_key) {
        furi_string_cat_printf(str, "\nDataProtKey: ");
        nfc_render_iso14443_3a_format_bytes(str, key, MF_ULTRALIGHT_AES_KEY_SIZE);
    }
}

// Decode the UL-AES configuration pages (0x29 / 0x2A / 0x2D). Captured only when the protected
// area was read (authenticated); layout per datasheet Tables 7-15 (cross-checked vs PM3
// ulaes_print_configuration). AUTH_LIM is the negative-auth limit that permanently locks the
// protected memory once hit - shown so a dictionary attack's lock risk is visible.
static void nfc_render_mf_ultralight_aes_config(const MfUltralightData* data, FuriString* str) {
    if(data->pages_read <= 0x2D) return;

    const uint8_t* cfg0 = data->page[0x29].data; // RID/SEC_MSG (b0), AUTH0 (b3)
    const uint8_t* cfg1 = data->page[0x2A].data; // CNT cfg (b0), VCTID (b1), AUTH_LIM (b2-3)
    const uint8_t* lock = data->page[0x2D].data; // LOCK_KEYS (b0)

    const uint8_t auth0 = cfg0[3];
    const uint16_t authlim = cfg1[2] | ((cfg1[3] & 0x03) << 8);

    furi_string_cat_printf(str, "\n\e#UL-AES Config");
    if(auth0 <= 0x3B) {
        furi_string_cat_printf(
            str, "\nAuth from: page 0x%02X (%s)", auth0, (cfg1[0] & 0x80) ? "r+w" : "write");
    } else {
        furi_string_cat_printf(str, "\nAuth from: off (open)");
    }
    if(authlim == 0) {
        furi_string_cat_printf(str, "\nAuth limit: unlimited");
    } else {
        furi_string_cat_printf(str, "\nAuth limit: %u (locks!)", authlim);
    }
    furi_string_cat_printf(str, "\nUser cfg: %s", (cfg1[0] & 0x40) ? "locked" : "open");
    furi_string_cat_printf(
        str,
        "\nCounter 2: rd %s / inc %s",
        (cfg1[0] & 0x04) ? "open" : "auth",
        (cfg1[0] & 0x08) ? "open" : "auth");
    furi_string_cat_printf(str, "\nRandom ID: %s", (cfg0[0] & 0x01) ? "on" : "off");
    furi_string_cat_printf(str, "\nSecure msg: %s", (cfg0[0] & 0x02) ? "on" : "off");
    furi_string_cat_printf(
        str,
        "\nKey lock: DP %s / UID %s",
        (lock[0] & 0x40) ? "Y" : "N",
        (lock[0] & 0x80) ? "Y" : "N");
}

void nfc_render_mf_ultralight_info(
    const MfUltralightData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    // UL-AES has no MfUltralightConfigPages, so show identity + pages read + counters + the
    // recovered key, skipping the PWD/PACK lines. The Short (read-result) form of iso14443_3a_info
    // omits the Tech line, so add it explicitly to match MIFARE Plus.
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
        nfc_render_mf_ultralight_counters(data, str);
        nfc_render_mf_ultralight_aes_key(data, str);
        if(format_type == NfcProtocolFormatTypeFull) {
            nfc_render_mf_ultralight_aes_config(data, str);
        }
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
