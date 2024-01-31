#include "iso14443_4a_render.h"

#include "../iso14443_3a/iso14443_3a_render.h"

void nfc_render_iso14443_4a_info(
    const Iso14443_4aData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    nfc_render_iso14443_4a_brief(data, str);

    if(format_type != NfcProtocolFormatTypeFull) return;

    nfc_render_iso14443_4a_extra(data, str);
}

void nfc_render_iso14443_4a_brief(const Iso14443_4aData* data, FuriString* str) {
    nfc_render_iso14443_tech_type(iso14443_4a_get_base_data(data), str);
    nfc_render_iso14443_3a_brief(iso14443_4a_get_base_data(data), str);
}

void nfc_render_iso14443_4a_extra(const Iso14443_4aData* data, FuriString* str) {
    furi_string_cat_printf(str, "\n::::::::::::::::[Protocol info]:::::::::::::::\n");

    if(iso14443_4a_supports_bit_rate(data, Iso14443_4aBitRateBoth106Kbit)) {
        furi_string_cat(str, "Bit rate PICC <-> PCD:\n  106 kBit/s supported\n");
    } else {
        furi_string_cat(str, "Bit rate PICC -> PCD:\n");
        if(iso14443_4a_supports_bit_rate(data, Iso14443_4aBitRatePiccToPcd212Kbit)) {
            furi_string_cat(str, "  212 kBit/s supported\n");
        }
        if(iso14443_4a_supports_bit_rate(data, Iso14443_4aBitRatePiccToPcd424Kbit)) {
            furi_string_cat(str, "  424 kBit/s supported\n");
        }
        if(iso14443_4a_supports_bit_rate(data, Iso14443_4aBitRatePiccToPcd848Kbit)) {
            furi_string_cat(str, "  848 kBit/s supported\n");
        }

        furi_string_cat(str, "Bit rate PICC <- PCD:\n");
        if(iso14443_4a_supports_bit_rate(data, Iso14443_4aBitRatePcdToPicc212Kbit)) {
            furi_string_cat(str, "  212 kBit/s supported\n");
        }
        if(iso14443_4a_supports_bit_rate(data, Iso14443_4aBitRatePcdToPicc424Kbit)) {
            furi_string_cat(str, "  424 kBit/s supported\n");
        }
        if(iso14443_4a_supports_bit_rate(data, Iso14443_4aBitRatePcdToPicc848Kbit)) {
            furi_string_cat(str, "  848 kBit/s supported\n");
        }
    }

    furi_string_cat(str, "Max frame size: ");

    const uint16_t max_frame_size = iso14443_4a_get_frame_size_max(data);
    if(max_frame_size != 0) {
        furi_string_cat_printf(str, "%u bytes\n", max_frame_size);
    } else {
        furi_string_cat(str, "? (RFU)\n");
    }

    const uint32_t fwt_fc = iso14443_4a_get_fwt_fc_max(data);
    if(fwt_fc != 0) {
        furi_string_cat_printf(str, "Max waiting time: %4.2g s\n", (double)(fwt_fc / 13.56e6));
    }

    const char* nad_support_str =
        iso14443_4a_supports_frame_option(data, Iso14443_4aFrameOptionNad) ? "" : "not ";
    furi_string_cat_printf(str, "NAD: %ssupported\n", nad_support_str);

    const char* cid_support_str =
        iso14443_4a_supports_frame_option(data, Iso14443_4aFrameOptionCid) ? "" : "not ";
    furi_string_cat_printf(str, "CID: %ssupported", cid_support_str);

    uint32_t hist_bytes_count;
    const uint8_t* hist_bytes = iso14443_4a_get_historical_bytes(data, &hist_bytes_count);

    if(hist_bytes_count > 0) {
        furi_string_cat_printf(str, "\n:::::::::::::[Historical bytes]:::::::::::::\nRaw:");

        for(size_t i = 0; i < hist_bytes_count; ++i) {
            furi_string_cat_printf(str, " %02X", hist_bytes[i]);
        }
    }

    furi_string_cat(str, "\n\e#ISO14443-3A data");
    nfc_render_iso14443_3a_extra(iso14443_4a_get_base_data(data), str);
}
