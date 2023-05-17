#include "namechanger.h"
#include <furi_hal_version.h>
#include <flipper_format/flipper_format.h>

#define TAG "NameChanger"

bool NameChanger_Init() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* str = furi_string_alloc();
    FlipperFormat* file = flipper_format_file_alloc(storage);

    bool res = false;

    do {
        uint32_t version;
        if(!flipper_format_file_open_existing(file, NAMECHANGER_PATH)) break;
        if(!flipper_format_read_header(file, str, &version)) break;
        if(furi_string_cmp_str(str, NAMECHANGER_HEADER)) break;
        if(version != NAMECHANGER_VERSION) break;

        if(!flipper_format_read_string(file, "Name", str)) break;
        // Check for size
        size_t temp_string_size = furi_string_size(str);
        if(temp_string_size > (size_t)8) break;
        if(temp_string_size < (size_t)2) break;

        // Check for forbidden characters
        const char* name_ptr = furi_string_get_cstr(str);
        bool chars_check_failed = false;

        for(; *name_ptr; ++name_ptr) {
            const char c = *name_ptr;
            if((c < '0' || c > '9') && (c < 'A' || c > 'Z') && (c < 'a' || c > 'z')) {
                chars_check_failed = true;
                break;
            }
        }

        if(chars_check_failed) break;

        // If all checks was good we can set the name
        version_set_custom_name(NULL, strdup(furi_string_get_cstr(str)));
        furi_hal_version_set_name(version_get_custom_name(NULL));

        res = true;
    } while(false);

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(str);

    return res;
}