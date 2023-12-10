#include "felica_render.h"

void nfc_render_felica_info(
    const FelicaData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    furi_string_cat_printf(str, "IDm:");

    for(size_t i = 0; i < FELICA_IDM_SIZE; i++) {
        furi_string_cat_printf(str, " %02X", data->idm.data[i]);
    }

    if(format_type == NfcProtocolFormatTypeFull) {
        furi_string_cat_printf(str, "\nPMm:");
        for(size_t i = 0; i < FELICA_PMM_SIZE; ++i) {
            furi_string_cat_printf(str, " %02X", data->pmm.data[i]);
        }
    }
}
