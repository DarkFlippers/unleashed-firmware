#include "iso_3166.h"

#include <flipper_format.h>

#define RESOURCE_FILE_PATH (EXT_PATH("lfrfid/assets/iso3166.lfrfid"))

static bool lfrfid_search_data(Storage* storage, uint16_t country_code, FuriString* out_line) {
    static const char* lfrfid_resources_header = "Flipper LFRFID resources";
    static const uint32_t lfrfid_resources_file_version = 1;

    FuriString* key = furi_string_alloc_printf("%04d", country_code);
    bool found = false;
    furi_string_reset(out_line);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* temp_str = furi_string_alloc();

    do {
        // Open file
        if(!flipper_format_file_open_existing(file, RESOURCE_FILE_PATH)) break;
        // Read file header and version
        uint32_t version = 0;
        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(furi_string_cmp_str(temp_str, lfrfid_resources_header) ||
           (version != lfrfid_resources_file_version))
            break;
        if(!flipper_format_read_string(file, furi_string_get_cstr(key), out_line)) break;
        found = true;
    } while(false);

    furi_string_free(key);
    furi_string_free(temp_str);
    flipper_format_free(file);

    if(found) {
        furi_string_replace_all(out_line, " ", "");
    }

    return found;
}

bool iso_3166_get_two_letter(Storage* storage, uint16_t country_code, FuriString* out_two_letter) {
    // We'll fetch the entire line from iso3166.lfrfid
    FuriString* line = furi_string_alloc();
    bool found = lfrfid_search_data(storage, country_code, line);

    if(found) {
        if(furi_string_size(line) < 2) {
            furi_string_free(line);
            FURI_LOG_E("Lfrifd:Iso_3166", "Not enough data for two-letter code");
            return false; // Not enough data for a two-letter code
        }
        // AFAFGAfghanistan
        furi_string_left(line, 2); // AF
        furi_string_set(out_two_letter, line);
    }
    furi_string_free(line);
    return found;
}

bool iso_3166_get_three_letter(
    Storage* storage,
    uint16_t country_code,
    FuriString* out_three_letter) {
    FuriString* line = furi_string_alloc();
    bool found = lfrfid_search_data(storage, country_code, line);

    if(found) {
        if(furi_string_size(line) < 5) {
            furi_string_free(line);
            FURI_LOG_E("Lfrifd:Iso_3166", "Not enough data for three-letter code");
            return false; // Not enough data for a three-letter code
        }
        // AFAFGAfghanistan
        furi_string_left(line, 5); // AFAFG
        furi_string_right(line, 2); // AFG
        furi_string_set(out_three_letter, line);
    }
    furi_string_free(line);
    return found;
}

bool iso_3166_get_full_name(Storage* storage, uint16_t country_code, FuriString* out_full_name) {
    FuriString* line = furi_string_alloc();
    bool found = lfrfid_search_data(storage, country_code, line);

    if(found) {
        if(furi_string_size(line) < 6) {
            furi_string_free(line);
            FURI_LOG_E("Lfrifd:Iso_3166", "Not enough data for full name");
            return false; // Not enough data for a full name
        }
        // AFAFGAfghanistan
        furi_string_right(line, 5); // Afghanistan
        furi_string_set(out_full_name, line);
    }
    furi_string_free(line);
    return found;
}
