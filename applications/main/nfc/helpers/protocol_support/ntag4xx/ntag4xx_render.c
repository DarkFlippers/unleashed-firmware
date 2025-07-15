#include "ntag4xx_render.h"

#include "../iso14443_4a/iso14443_4a_render.h"

void nfc_render_ntag4xx_info(
    const Ntag4xxData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    nfc_render_iso14443_4a_brief(ntag4xx_get_base_data(data), str);

    const Ntag4xxType type = ntag4xx_get_type_from_version(&data->version);
    if(type >= Ntag4xxTypeUnknown) {
        furi_string_cat(str, "Memory Size: unknown");
    } else {
        size_t size_cc = 32;
        size_t size_ndef = 0;
        size_t size_proprietary = 0;
        bool has_tagtamper = false;
        switch(type) {
        case Ntag4xxType413DNA:
            size_ndef = 128;
            size_proprietary = 0;
            break;
        case Ntag4xxType424DNATT:
            has_tagtamper = true;
            /* fall-through */
        case Ntag4xxType424DNA:
            size_ndef = 256;
            size_proprietary = 128;
            break;
        case Ntag4xxType426QDNATT:
            has_tagtamper = true;
            /* fall-through */
        case Ntag4xxType426QDNA:
            size_ndef = 768;
            size_proprietary = 128;
            break;
        default:
            break;
        }
        furi_string_cat_printf(
            str, "\nMemory Size: %zu bytes\n", size_cc + size_ndef + size_proprietary);
        furi_string_cat_printf(str, "Usable NDEF Size: %zu bytes\n", size_ndef - sizeof(uint16_t));
        furi_string_cat_printf(str, "Capability Cont.: %zu bytes\n", size_cc);
        if(size_proprietary) {
            furi_string_cat_printf(str, "Proprietary File: %zu bytes\n", size_proprietary);
        }
        furi_string_cat_printf(str, "TagTamper: %ssupported", has_tagtamper ? "" : "not ");
    }

    if(format_type != NfcProtocolFormatTypeFull) return;

    furi_string_cat(str, "\n\e#ISO14443-4 data");
    nfc_render_iso14443_4a_extra(ntag4xx_get_base_data(data), str);
}

void nfc_render_ntag4xx_data(const Ntag4xxData* data, FuriString* str) {
    nfc_render_ntag4xx_version(&data->version, str);
}

void nfc_render_ntag4xx_version(const Ntag4xxVersion* data, FuriString* str) {
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
        "batch %02x:%02x:%02x:%02x:%01x\n"
        "week %d year %d\n"
        "fab key %02x id %02x\n",
        data->batch[0],
        data->batch[1],
        data->batch[2],
        data->batch[3],
        data->batch_extra,
        data->prod_week,
        data->prod_year,
        (data->fab_key_4b << 1) | (data->fab_key_1b),
        data->optional.fab_key_id);
}
