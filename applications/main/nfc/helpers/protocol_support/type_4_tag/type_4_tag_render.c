#include "type_4_tag_render.h"

#include "../iso14443_4a/iso14443_4a_render.h"

void nfc_render_type_4_tag_info(
    const Type4TagData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    nfc_render_iso14443_4a_brief(type_4_tag_get_base_data(data), str);

    furi_string_cat(str, "\n:::::::::::::::[Stored NDEF]:::::::::::::::\n");
    furi_string_cat_printf(str, "Current NDEF Size: %lu", simple_array_get_count(data->ndef_data));

    if(data->is_tag_specific) {
        furi_string_cat(str, "\n::::::::::::::::::[Tag Specs]::::::::::::::::::\n");
        furi_string_cat_printf(
            str,
            "Card: %s\n",
            furi_string_empty(data->platform_name) ? "unknown" :
                                                     furi_string_get_cstr(data->platform_name));
        furi_string_cat_printf(
            str, "T4T Mapping Version: %u.%u\n", data->t4t_version.major, data->t4t_version.minor);
        furi_string_cat_printf(str, "NDEF File ID: %04X\n", data->ndef_file_id);
        furi_string_cat_printf(str, "Max NDEF Size: %u\n", data->ndef_max_len);
        furi_string_cat_printf(
            str, "APDU Sizes: R:%u W:%u\n", data->chunk_max_read, data->chunk_max_write);
        furi_string_cat_printf(
            str,
            "Read Lock: %02X%s\n",
            data->ndef_read_lock,
            data->ndef_read_lock == 0 ? " (unlocked)" : "");
        furi_string_cat_printf(
            str,
            "Write Lock: %02X%s",
            data->ndef_write_lock,
            data->ndef_write_lock == 0 ? " (unlocked)" : "");
    }

    if(format_type != NfcProtocolFormatTypeFull) return;

    furi_string_cat(str, "\n\e#ISO14443-4 data");
    nfc_render_iso14443_4a_extra(type_4_tag_get_base_data(data), str);
}

void nfc_render_type_4_tag_dump(const Type4TagData* data, FuriString* str) {
    size_t ndef_len = simple_array_get_count(data->ndef_data);
    if(ndef_len == 0) {
        furi_string_cat_str(str, "No NDEF data to show");
        return;
    }
    const uint8_t* ndef_data = simple_array_cget_data(data->ndef_data);
    furi_string_cat_printf(str, "\e*");
    for(size_t i = 0; i < ndef_len; i += TYPE_4_TAG_RENDER_BYTES_PER_LINE) {
        const uint8_t* line_data = &ndef_data[i];
        for(size_t j = 0; j < TYPE_4_TAG_RENDER_BYTES_PER_LINE; j += 2) {
            furi_string_cat_printf(str, " %02X%02X", line_data[j], line_data[j + 1]);
        }
    }
}
