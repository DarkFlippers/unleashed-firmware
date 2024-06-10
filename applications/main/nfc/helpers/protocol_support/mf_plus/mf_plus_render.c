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

void nfc_render_mf_plus_data(const MfPlusData* data, FuriString* str) {
    nfc_render_mf_plus_version(&data->version, str);
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
