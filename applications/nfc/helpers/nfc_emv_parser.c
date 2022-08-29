#include "nfc_emv_parser.h"
#include <flipper_format/flipper_format.h>

static const char* nfc_resources_header = "Flipper EMV resources";
static const uint32_t nfc_resources_file_version = 1;

static bool nfc_emv_parser_search_data(
    Storage* storage,
    const char* file_name,
    string_t key,
    string_t data) {
    bool parsed = false;
    FlipperFormat* file = flipper_format_file_alloc(storage);
    string_t temp_str;
    string_init(temp_str);

    do {
        // Open file
        if(!flipper_format_file_open_existing(file, file_name)) break;
        // Read file header and version
        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(string_cmp_str(temp_str, nfc_resources_header) ||
           (version != nfc_resources_file_version))
            break;
        if(!flipper_format_read_string(file, string_get_cstr(key), data)) break;
        parsed = true;
    } while(false);

    string_clear(temp_str);
    flipper_format_free(file);
    return parsed;
}

bool nfc_emv_parser_get_aid_name(
    Storage* storage,
    uint8_t* aid,
    uint8_t aid_len,
    string_t aid_name) {
    furi_assert(storage);
    bool parsed = false;
    string_t key;
    string_init(key);
    for(uint8_t i = 0; i < aid_len; i++) {
        string_cat_printf(key, "%02X", aid[i]);
    }
    if(nfc_emv_parser_search_data(storage, EXT_PATH("nfc/assets/aid.nfc"), key, aid_name)) {
        parsed = true;
    }
    string_clear(key);
    return parsed;
}

bool nfc_emv_parser_get_country_name(
    Storage* storage,
    uint16_t country_code,
    string_t country_name) {
    bool parsed = false;
    string_t key;
    string_init_printf(key, "%04X", country_code);
    if(nfc_emv_parser_search_data(
           storage, EXT_PATH("nfc/assets/country_code.nfc"), key, country_name)) {
        parsed = true;
    }
    string_clear(key);
    return parsed;
}

bool nfc_emv_parser_get_currency_name(
    Storage* storage,
    uint16_t currency_code,
    string_t currency_name) {
    bool parsed = false;
    string_t key;
    string_init_printf(key, "%04X", currency_code);
    if(nfc_emv_parser_search_data(
           storage, EXT_PATH("nfc/assets/currency_code.nfc"), key, currency_name)) {
        parsed = true;
    }
    string_clear(key);
    return parsed;
}
