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

    if(!has_config) {
        furi_string_cat_printf(str, "\nThis card does not support\npassword protection!");
    } else if(mf_ultralight_is_pwd_pack_read(data)) {
        nfc_render_mf_ultralight_pwd_pack_lines(config, str);
    } else {
        // Unauthenticated reads mask these pages to zero - don't show 00 00 00 00 as a password.
        furi_string_cat_printf(str, "\nPassword not captured.");
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
    if(data->pages_read <= MF_ULTRALIGHT_AES_LOCK_KEYS_PAGE) return;

    const uint8_t* cfg =
        data->page[MF_ULTRALIGHT_AES_CFG_PAGE].data; // RID/SEC_MSG (b0), AUTH0 (b3)
    const uint8_t* acc =
        data->page[MF_ULTRALIGHT_AES_ACCESS_PAGE].data; // access (b0), AUTH_LIM (b2-3)
    const uint8_t lock = data->page[MF_ULTRALIGHT_AES_LOCK_KEYS_PAGE].data[0];

    const uint8_t auth0 = cfg[3];
    const uint16_t authlim = acc[2] | ((acc[3] & 0x03) << 8);
    const bool prot_rw = acc[0] & MF_ULTRALIGHT_AES_ACCESS_PROT;
    const bool user_cfg_locked = acc[0] & MF_ULTRALIGHT_AES_ACCESS_CFGLCK;
    const bool cnt2_rd_open = acc[0] & MF_ULTRALIGHT_AES_ACCESS_CNT_RD_EN;
    const bool cnt2_inc_open = acc[0] & MF_ULTRALIGHT_AES_ACCESS_CNT_INC_EN;
    const bool random_id = cfg[0] & MF_ULTRALIGHT_AES_CFG_RID_ACT;
    const bool secure_msg = cfg[0] & MF_ULTRALIGHT_AES_CFG_SEC_MSG_ACT;
    const bool dp_key_locked = lock & MF_ULTRALIGHT_AES_LOCK_KEY0;
    const bool uid_key_locked = lock & MF_ULTRALIGHT_AES_LOCK_KEY1;

    furi_string_cat_printf(str, "\n\e#UL-AES Config");
    // A Random-ID card presents a random anticollision UID (shown at the top of the info view); the
    // real static UID is revealed in pages 0-1 only after UIDRetrKey auth (they read as 0 otherwise,
    // so page 0 byte 0 == 0x04 means it was recovered). Surface it at the top of the config section.
    if(random_id && data->page[0].data[0] == 0x04) {
        const uint8_t* p0 = data->page[0].data;
        const uint8_t* p1 = data->page[1].data;
        const uint8_t real_uid[7] = {p0[0], p0[1], p0[2], p1[0], p1[1], p1[2], p1[3]};
        furi_string_cat_printf(str, "\nReal UID:");
        nfc_render_iso14443_3a_format_bytes(str, real_uid, sizeof(real_uid));
    }
    if(auth0 <= 0x3B) {
        furi_string_cat_printf(
            str, "\nAuth from: page 0x%02X (%s)", auth0, prot_rw ? "r+w" : "write");
    } else {
        furi_string_cat_printf(str, "\nAuth from: off (open)");
    }
    if(authlim == 0) {
        furi_string_cat_printf(str, "\nAuth limit: unlimited");
    } else {
        furi_string_cat_printf(str, "\nAuth limit: %u (locks!)", authlim);
    }
    furi_string_cat_printf(str, "\nUser cfg: %s", user_cfg_locked ? "locked" : "open");
    furi_string_cat_printf(
        str,
        "\nCounter 2: rd %s / inc %s",
        cnt2_rd_open ? "open" : "auth",
        cnt2_inc_open ? "open" : "auth");
    furi_string_cat_printf(str, "\nRandom ID: %s", random_id ? "on" : "off");
    furi_string_cat_printf(str, "\nSecure msg: %s", secure_msg ? "on" : "off");
    furi_string_cat_printf(
        str, "\nKey lock: DP %s / UID %s", dp_key_locked ? "Y" : "N", uid_key_locked ? "Y" : "N");
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
