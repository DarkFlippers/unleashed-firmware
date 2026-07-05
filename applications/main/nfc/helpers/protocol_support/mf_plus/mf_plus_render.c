#include "mf_plus_render.h"

#include "../iso14443_4a/iso14443_4a_render.h"

void nfc_render_mf_plus_info(
    const MfPlusData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    nfc_render_iso14443_4a_brief(mf_plus_get_base_data(data), str);

    if(format_type != NfcProtocolFormatTypeFull) return;

    furi_string_cat(str, "\n\e#ISO14443-4 data");
    nfc_render_iso14443_4a_extra(mf_plus_get_base_data(data), str);
}

// Append `len` bytes as contiguous hex, or "??" per byte when the value was never recovered (the
// same unknown marker the saved dump uses), so the view stays readable regardless of what unlocked.
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
    }
}

// The SL3 content recovered by a dictionary attack: signature, sector keys, admin keys (0x90xx),
// config blocks (0xB0xx) and the data blocks. Only present on a fully-AES (SL3) card.
static void nfc_render_mf_plus_sl3_content(const MfPlusData* data, FuriString* str) {
    if(data->security_level != MfPlusSecurityLevel3) return;

    if(data->signature_present) {
        furi_string_cat(str, "\n\e#Signature\n");
        nfc_render_mf_plus_hex_or_unknown(data->signature, MF_PLUS_SIGNATURE_SIZE, true, str);
        furi_string_cat(str, "\n");
    }

    const uint8_t sectors = mf_plus_get_sector_count(data->size);
    if(sectors > 0) {
        furi_string_cat(str, "\n\e#Sector Keys\n");
        for(uint8_t s = 0; s < sectors; s++) {
            furi_string_cat_printf(str, "S%u A:", s);
            nfc_render_mf_plus_hex_or_unknown(
                data->key_a[s].data,
                MF_PLUS_KEY_SIZE,
                mf_plus_is_key_found(data, s, MfPlusKeyTypeA),
                str);
            furi_string_cat(str, "\n   B:");
            nfc_render_mf_plus_hex_or_unknown(
                data->key_b[s].data,
                MF_PLUS_KEY_SIZE,
                mf_plus_is_key_found(data, s, MfPlusKeyTypeB),
                str);
            furi_string_cat(str, "\n");
        }
    }

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

    const uint16_t blocks = mf_plus_get_block_count(data->size);
    if(blocks > 0) {
        furi_string_cat(str, "\n\e#Blocks\n");
        for(uint16_t b = 0; b < blocks; b++) {
            furi_string_cat_printf(str, "%3u:", b);
            nfc_render_mf_plus_hex_or_unknown(
                data->block[b].data, MF_PLUS_BLOCK_SIZE, mf_plus_is_block_read(data, b), str);
            furi_string_cat(str, "\n");
        }
    }
}

void nfc_render_mf_plus_data(const MfPlusData* data, FuriString* str) {
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

    nfc_render_mf_plus_sl3_content(data, str);
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
