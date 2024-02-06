#include "iso14443_3b_render.h"

void nfc_render_iso14443_3b_info(
    const Iso14443_3bData* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    if(format_type == NfcProtocolFormatTypeFull) {
        const char iso_type = iso14443_3b_supports_iso14443_4(data) ? '4' : '3';
        furi_string_cat_printf(str, "Tech: ISO 14443-%c (NFC-B)\n", iso_type);
    }

    furi_string_cat_printf(str, "UID:");

    size_t uid_size;
    const uint8_t* uid = iso14443_3b_get_uid(data, &uid_size);

    for(size_t i = 0; i < uid_size; i++) {
        furi_string_cat_printf(str, " %02X", uid[i]);
    }

    if(format_type != NfcProtocolFormatTypeFull) return;

    furi_string_cat_printf(str, "\n::::::::::::::::[Protocol info]:::::::::::::::\n");

    if(iso14443_3b_supports_bit_rate(data, Iso14443_3bBitRateBoth106Kbit)) {
        furi_string_cat(str, "Bit rate PICC <-> PCD:\n  106 kBit/s supported\n");
    } else {
        furi_string_cat(str, "Bit rate PICC -> PCD:\n");
        if(iso14443_3b_supports_bit_rate(data, Iso14443_3bBitRatePiccToPcd212Kbit)) {
            furi_string_cat(str, "  212 kBit/s supported\n");
        }
        if(iso14443_3b_supports_bit_rate(data, Iso14443_3bBitRatePiccToPcd424Kbit)) {
            furi_string_cat(str, "  424 kBit/s supported\n");
        }
        if(iso14443_3b_supports_bit_rate(data, Iso14443_3bBitRatePiccToPcd848Kbit)) {
            furi_string_cat(str, "  848 kBit/s supported\n");
        }

        furi_string_cat(str, "Bit rate PICC <- PCD:\n");
        if(iso14443_3b_supports_bit_rate(data, Iso14443_3bBitRatePcdToPicc212Kbit)) {
            furi_string_cat(str, "  212 kBit/s supported\n");
        }
        if(iso14443_3b_supports_bit_rate(data, Iso14443_3bBitRatePcdToPicc424Kbit)) {
            furi_string_cat(str, "  424 kBit/s supported\n");
        }
        if(iso14443_3b_supports_bit_rate(data, Iso14443_3bBitRatePcdToPicc848Kbit)) {
            furi_string_cat(str, "  848 kBit/s supported\n");
        }
    }

    furi_string_cat(str, "Max frame size: ");

    const uint16_t max_frame_size = iso14443_3b_get_frame_size_max(data);
    if(max_frame_size != 0) {
        furi_string_cat_printf(str, "%u bytes\n", max_frame_size);
    } else {
        furi_string_cat(str, "? (RFU)\n");
    }

    const double fwt = iso14443_3b_get_fwt_fc_max(data) / 13.56e6;
    furi_string_cat_printf(str, "Max waiting time: %4.2g s\n", fwt);

    const char* nad_support_str =
        iso14443_3b_supports_frame_option(data, Iso14443_3bFrameOptionNad) ? "" : "not ";
    furi_string_cat_printf(str, "NAD: %ssupported\n", nad_support_str);

    const char* cid_support_str =
        iso14443_3b_supports_frame_option(data, Iso14443_3bFrameOptionCid) ? "" : "not ";
    furi_string_cat_printf(str, "CID: %ssupported", cid_support_str);

    furi_string_cat_printf(str, "\n::::::::::::[Application data]::::::::::::\nRaw:");

    size_t app_data_size;
    const uint8_t* app_data = iso14443_3b_get_application_data(data, &app_data_size);
    for(size_t i = 0; i < app_data_size; ++i) {
        furi_string_cat_printf(str, " %02X", app_data[i]);
    }
}
