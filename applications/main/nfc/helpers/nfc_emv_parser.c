#include "nfc_emv_parser.h"
#include <flipper_format/flipper_format.h>

static const char* nfc_resources_header = "Flipper EMV resources";
static const uint32_t nfc_resources_file_version = 1;

static bool nfc_emv_parser_search_data(
    Storage* storage,
    const char* file_name,
    FuriString* key,
    FuriString* data) {
    bool parsed = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* temp_str;
    temp_str = furi_string_alloc();

    do {
        // Open file
        if(!flipper_format_file_open_existing(file, file_name)) break;
        // Read file header and version
        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(furi_string_cmp_str(temp_str, nfc_resources_header) ||
           (version != nfc_resources_file_version))
            break;
        if(!flipper_format_read_string(file, furi_string_get_cstr(key), data)) break;
        parsed = true;
    } while(false);

    furi_string_free(temp_str);
    flipper_format_free(file);
    return parsed;
}

bool nfc_emv_parser_get_aid_name(
    Storage* storage,
    uint8_t* aid,
    uint8_t aid_len,
    FuriString* aid_name) {
    furi_assert(storage);
    bool parsed = false;
    FuriString* key;
    key = furi_string_alloc();
    for(uint8_t i = 0; i < aid_len; i++) {
        furi_string_cat_printf(key, "%02X", aid[i]);
    }
    if(nfc_emv_parser_search_data(storage, EXT_PATH("nfc/assets/aid.nfc"), key, aid_name)) {
        parsed = true;
    }
    furi_string_free(key);
    return parsed;
}

bool nfc_emv_parser_get_country_name(
    Storage* storage,
    uint16_t country_code,
    FuriString* country_name) {
    bool parsed = false;
    FuriString* key;
    key = furi_string_alloc_printf("%04X", country_code);
    if(nfc_emv_parser_search_data(
           storage, EXT_PATH("nfc/assets/country_code.nfc"), key, country_name)) {
        parsed = true;
    }
    furi_string_free(key);
    return parsed;
}

bool nfc_emv_parser_get_currency_name(
    Storage* storage,
    uint16_t currency_code,
    FuriString* currency_name) {
    bool parsed = false;
    FuriString* key;
    key = furi_string_alloc_printf("%04X", currency_code);
    if(nfc_emv_parser_search_data(
           storage, EXT_PATH("nfc/assets/currency_code.nfc"), key, currency_name)) {
        parsed = true;
    }
    furi_string_free(key);
    return parsed;
}
