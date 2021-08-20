#include "nfc_emv_parser.h"

#include <file-worker.h>

static bool
    nfc_emv_parser_get_value(const char* file_path, string_t key, char delimiter, string_t value) {
    bool found = false;
    FileWorker* file_worker = file_worker_alloc(true);

    if(file_worker_open(file_worker, file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(file_worker_get_value_from_key(file_worker, key, delimiter, value)) {
            found = true;
        }
    }

    file_worker_close(file_worker);
    file_worker_free(file_worker);
    return found;
}

bool nfc_emv_parser_get_aid_name(uint8_t* aid, uint8_t aid_len, string_t aid_name) {
    bool result = false;
    string_t key;
    string_init(key);
    for(uint8_t i = 0; i < aid_len; i++) {
        string_cat_printf(key, "%02X", aid[i]);
    }
    result = nfc_emv_parser_get_value("/ext/nfc/emv/aid.nfc", key, ' ', aid_name);
    string_clear(key);
    return result;
}

bool nfc_emv_parser_get_country_name(uint16_t country_code, string_t country_name) {
    bool result = false;
    string_t key;
    string_init_printf(key, "%04X", country_code);
    result = nfc_emv_parser_get_value("/ext/nfc/emv/country_code.nfc", key, ' ', country_name);
    string_clear(key);
    return result;
}

bool nfc_emv_parser_get_currency_name(uint16_t currency_code, string_t currency_name) {
    bool result = false;
    string_t key;
    string_init_printf(key, "%04X", currency_code);
    result = nfc_emv_parser_get_value("/ext/nfc/emv/currency_code.nfc", key, ' ', currency_name);
    string_clear(key);
    return result;
}
