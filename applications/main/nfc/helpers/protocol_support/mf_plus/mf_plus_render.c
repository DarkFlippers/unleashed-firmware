#include "mf_plus_render.h"

#include "../iso14443_3a/iso14443_3a_render.h"
#include "../iso14443_4a/iso14443_4a_render.h"

// SL3 recovery progress, in sectors like MIFARE Classic ("Keys Found / Sectors Read"). Only an SL3
// card recovers any keys/blocks, so it is the only level that shows these lines.
static void nfc_render_mf_plus_recovery_stats(const MfPlusData* data, FuriString* str) {
    if(data->security_level != MfPlusSecurityLevel3) return;

    const uint8_t sectors_total = mf_plus_get_sector_count(data->size);
    uint8_t sectors_read = 0;
    uint8_t keys_found = 0;
    mf_plus_get_read_sectors_and_keys(data, &sectors_read, &keys_found);

    furi_string_cat_printf(str, "\nKeys Found: %u/%u", keys_found, sectors_total * 2);
    furi_string_cat_printf(str, "\nSectors Read: %u/%u", sectors_read, sectors_total);
}

void nfc_render_mf_plus_info(
    const MfPlusData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    // MIFARE-Classic-style header: [Tech (full only)] + UID + [ATQA/SAK (full only)]. tech_type
    // reports "ISO 14443-4" because the base 3a layer advertises ISO14443-4 support. The deeper
    // ISO14443-4 protocol detail (ATS, bit rates) lives in "More info", not on this screen.
    const Iso14443_3aData* iso3 = iso14443_4a_get_base_data(mf_plus_get_base_data(data));
    nfc_render_iso14443_3a_info(iso3, format_type, str);

    nfc_render_mf_plus_recovery_stats(data, str);
}

// Append `len` bytes as space-separated 2-byte groups (like the MIFARE Classic dump), or "??" per
// byte when the value was never recovered. The spaces give the word-wrapping dump view clean break
// points so a long line wraps between groups instead of mid-value.
static void nfc_render_mf_plus_hex_or_unknown(
    const uint8_t* bytes,
    size_t len,
    bool known,
    FuriString* str) {
    for(size_t i = 0; i < len; i++) {
        if(known) {
            furi_string_cat_printf(str, "%02X", bytes[i]);
        } else {
            furi_string_cat(str, "??");
        }
        if((i % 2) == 1 && i + 1 < len) furi_string_cat(str, " ");
    }
}

// MIFARE-Classic-style block dump: every block as hex, but a sector trailer shows the recovered
// Key A + the trailer block's access bits (its bytes 6-9) + the recovered Key B. SL3 keeps the AES
// keys in a separate keyspace (not in the trailer block), so the raw trailer's key bytes are
// meaningless -- substituting the recovered keys is what makes this read like a Classic dump.
static void nfc_render_mf_plus_blocks(const MfPlusData* data, FuriString* str) {
    const uint8_t sectors = mf_plus_get_sector_count(data->size);
    if(sectors == 0) return;

    furi_string_cat(str, "\n\e#Blocks\n");
    for(uint8_t s = 0; s < sectors; s++) {
        const uint16_t first = mf_plus_sector_get_first_block(s);
        const uint16_t trailer = first + mf_plus_sector_get_block_count(s) - 1;
        for(uint16_t b = first; b <= trailer; b++) {
            furi_string_cat_printf(str, "%u: ", b);
            if(b == trailer) {
                nfc_render_mf_plus_hex_or_unknown(
                    data->key_a[s].data,
                    MF_PLUS_KEY_SIZE,
                    mf_plus_is_key_found(data, s, MfPlusKeyTypeA),
                    str);
                furi_string_cat(str, " ");
                nfc_render_mf_plus_hex_or_unknown(
                    &data->block[b].data[6], 4, mf_plus_is_block_read(data, b), str);
                furi_string_cat(str, " ");
                nfc_render_mf_plus_hex_or_unknown(
                    data->key_b[s].data,
                    MF_PLUS_KEY_SIZE,
                    mf_plus_is_key_found(data, s, MfPlusKeyTypeB),
                    str);
            } else {
                nfc_render_mf_plus_hex_or_unknown(
                    data->block[b].data, MF_PLUS_BLOCK_SIZE, mf_plus_is_block_read(data, b), str);
            }
            furi_string_cat(str, "\n");
        }
    }
}

void nfc_render_mf_plus_dump(const MfPlusData* data, FuriString* str) {
    if(data->security_level != MfPlusSecurityLevel3) {
        furi_string_cat(str, "No block data: only an SL3 card exposes keys/blocks here.\n");
        return;
    }

    nfc_render_mf_plus_blocks(data, str);

    furi_string_cat(str, "\n\e#Admin Keys\n");
    for(uint8_t a = 0; a < MfPlusAdminKeyNum; a++) {
        const MfPlusAdminKeyType type = (MfPlusAdminKeyType)a;
        furi_string_cat_printf(str, "%s:\n", mf_plus_get_admin_key_name(type));
        nfc_render_mf_plus_hex_or_unknown(
            data->admin_key[a].data, MF_PLUS_KEY_SIZE, mf_plus_is_admin_key_found(data, type), str);
        furi_string_cat(str, "\n");
    }

    furi_string_cat(str, "\n\e#Config Blocks\n");
    for(uint8_t c = 0; c < MF_PLUS_CONFIG_BLOCK_NUM; c++) {
        furi_string_cat_printf(str, "B00%u:", c);
        nfc_render_mf_plus_hex_or_unknown(
            data->config_block[c].data,
            MF_PLUS_BLOCK_SIZE,
            mf_plus_is_config_block_read(data, c),
            str);
        furi_string_cat(str, "\n");
    }

    if(data->signature_present) {
        furi_string_cat(str, "\n\e#Signature\n");
        nfc_render_mf_plus_hex_or_unknown(data->signature, MF_PLUS_SIGNATURE_SIZE, true, str);
        furi_string_cat(str, "\n");
    }
}

void nfc_render_mf_plus_iso4(const MfPlusData* data, FuriString* str) {
    MfPlusVersion empty_version = {0};
    if(memcmp(&data->version, &empty_version, sizeof(MfPlusVersion)) == 0) {
        const char* device_name = mf_plus_get_device_name(data, NfcDeviceNameTypeFull);
        if(data->type == MfPlusTypeUnknown || data->size == MfPlusSizeUnknown ||
           data->security_level == MfPlusSecurityLevelUnknown) {
            furi_string_cat_printf(str, "This %s", device_name);
            furi_string_replace(str, " Unknown", "");
        } else {
            furi_string_cat(str, device_name);
        }
        furi_string_replace(str, "Mifare", "MIFARE");
        furi_string_cat(str, " does not support the GetVersion command, extra info unavailable\n");
    } else {
        nfc_render_mf_plus_version(&data->version, str);
    }

    furi_string_cat(str, "\n\e#ISO14443-4 data");
    nfc_render_iso14443_4a_extra(mf_plus_get_base_data(data), str);
}

void nfc_render_mf_plus_version(const MfPlusVersion* data, FuriString* str) {
    furi_string_cat_printf(
        str,
        "%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
        data->uid[0],
        data->uid[1],
        data->uid[2],
        data->uid[3],
        data->uid[4],
        data->uid[5],
        data->uid[6]);
    furi_string_cat_printf(
        str,
        "hw %02x type %02x sub %02x\n"
        " maj %02x min %02x\n"
        " size %02x proto %02x\n",
        data->hw_vendor,
        data->hw_type,
        data->hw_subtype,
        data->hw_major,
        data->hw_minor,
        data->hw_storage,
        data->hw_proto);
    furi_string_cat_printf(
        str,
        "sw %02x type %02x sub %02x\n"
        " maj %02x min %02x\n"
        " size %02x proto %02x\n",
        data->sw_vendor,
        data->sw_type,
        data->sw_subtype,
        data->sw_major,
        data->sw_minor,
        data->sw_storage,
        data->sw_proto);
    furi_string_cat_printf(
        str,
        "batch %02x:%02x:%02x:%02x:%02x\n"
        "week %d year %d\n",
        data->batch[0],
        data->batch[1],
        data->batch[2],
        data->batch[3],
        data->batch[4],
        data->prod_week,
        data->prod_year);
}
