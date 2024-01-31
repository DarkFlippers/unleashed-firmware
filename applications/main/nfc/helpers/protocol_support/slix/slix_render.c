#include "slix_render.h"

void nfc_render_slix_info(const SlixData* data, NfcProtocolFormatType format_type, FuriString* str) {
    nfc_render_iso15693_3_brief(slix_get_base_data(data), str);

    if(format_type != NfcProtocolFormatTypeFull) return;
    const SlixType slix_type = slix_get_type(data);

    furi_string_cat(str, "\n::::::::::::::::::[Passwords]:::::::::::::::::\n");

    static const char* slix_password_names[] = {
        "Read",
        "Write",
        "Privacy",
        "Destroy",
        "EAS/AFI",
    };

    for(uint32_t i = 0; i < SlixPasswordTypeCount; ++i) {
        if(slix_type_supports_password(slix_type, i)) {
            furi_string_cat_printf(
                str, "%s :  %08lX\n", slix_password_names[i], data->passwords[i]);
        }
    }

    furi_string_cat(str, ":::::::::::::::::::[Lock bits]::::::::::::::::::::\n");

    if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_EAS)) {
        furi_string_cat_printf(
            str, "EAS: %s locked\n", data->system_info.lock_bits.eas ? "" : "not");
    }

    if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_PROTECTION)) {
        furi_string_cat_printf(
            str, "PPL: %s locked\n", data->system_info.lock_bits.ppl ? "" : "not");

        const SlixProtection protection = data->system_info.protection;

        furi_string_cat(str, "::::::::::::[Page protection]::::::::::::\n");
        furi_string_cat_printf(str, "Pointer: H >= %02X\n", protection.pointer);

        const char* rh = (protection.condition & SLIX_PP_CONDITION_RH) ? "" : "un";
        const char* rl = (protection.condition & SLIX_PP_CONDITION_RL) ? "" : "un";

        const char* wh = (protection.condition & SLIX_PP_CONDITION_WH) ? "" : "un";
        const char* wl = (protection.condition & SLIX_PP_CONDITION_WL) ? "" : "un";

        furi_string_cat_printf(str, "R:  H %sprotec. L %sprotec.\n", rh, rl);
        furi_string_cat_printf(str, "W: H %sprotec. L %sprotec.\n", wh, wl);
    }

    if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_PRIVACY)) {
        furi_string_cat(str, "::::::::::::::::::::[Privacy]::::::::::::::::::::::\n");
        furi_string_cat_printf(str, "Privacy mode: %sabled\n", data->privacy ? "en" : "dis");
    }

    if(slix_type_has_features(slix_type, SLIX_TYPE_FEATURE_SIGNATURE)) {
        furi_string_cat(str, ":::::::::::::::::::[Signature]::::::::::::::::::\n");
        for(uint32_t i = 0; i < 4; ++i) {
            furi_string_cat_printf(str, "%02X ", data->signature[i]);
        }

        furi_string_cat(str, "[ ... ]");

        for(uint32_t i = 0; i < 3; ++i) {
            furi_string_cat_printf(str, " %02X", data->signature[sizeof(SlixSignature) - i - 1]);
        }
    }

    furi_string_cat(str, "\n\e#ISO15693-3 data");
    nfc_render_iso15693_3_extra(slix_get_base_data(data), str);
}
