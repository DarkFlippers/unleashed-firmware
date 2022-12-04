#include "namechangersrv.h"
#include "m-string.h"
#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

void namechanger_on_system_start() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);

    FuriString* NAMEHEADER;
    NAMEHEADER = furi_string_alloc_set("Flipper Name File");

    FuriString* folderpath;
    folderpath = furi_string_alloc_set("/ext/dolphin");

    FuriString* filepath;
    filepath = furi_string_alloc_set("/ext/dolphin/name.txt");

    //Make dir if doesn't exist
    if(storage_simply_mkdir(storage, furi_string_get_cstr(folderpath))) {
        bool result = false;

        FuriString* data;
        data = furi_string_alloc();

        do {
            if(!flipper_format_file_open_existing(file, furi_string_get_cstr(filepath))) {
                break;
            }

            // header
            uint32_t version;

            if(!flipper_format_read_header(file, data, &version)) {
                break;
            }

            if(furi_string_cmp_str(data, furi_string_get_cstr(NAMEHEADER)) != 0) {
                break;
            }

            if(version != 1) {
                break;
            }

            // get Name
            if(!flipper_format_read_string(file, "Name", data)) {
                break;
            }

            result = true;
        } while(false);

        flipper_format_free(file);

        if(!result) {
            //file not good - write new one
            FlipperFormat* file = flipper_format_file_alloc(storage);

            bool res = false;

            FuriString* name;
            name = furi_string_alloc_set(furi_hal_version_get_name_ptr());

            do {
                // Open file for write
                if(!flipper_format_file_open_always(file, furi_string_get_cstr(filepath))) {
                    break;
                }

                // Write header
                if(!flipper_format_write_header_cstr(file, furi_string_get_cstr(NAMEHEADER), 1)) {
                    break;
                }

                // Write comments
                if(!flipper_format_write_comment_cstr(
                       file,
                       "Changing the value below will change your FlipperZero device name.")) {
                    break;
                }

                if(!flipper_format_write_comment_cstr(
                       file,
                       "Note: This is limited to 8 characters using the following: a-z, A-Z, 0-9, and _")) {
                    break;
                }

                if(!flipper_format_write_comment_cstr(
                       file, "It can contain other characters but use at your own risk.")) {
                    break;
                }

                //Write name
                if(!flipper_format_write_string_cstr(file, "Name", furi_string_get_cstr(name))) {
                    break;
                }

                res = true;
            } while(false);

            flipper_format_free(file);

            if(!res) {
                FURI_LOG_E(TAG, "Save failed.");
            }

            furi_string_free(name);
        } else {
            furi_string_trim(data);
            FURI_LOG_I(TAG, "data: %s", furi_string_get_cstr(data));

            if(!furi_string_size(data)) {
                //Empty file - get default name and write to file.
                FlipperFormat* file = flipper_format_file_alloc(storage);

                bool res = false;

                FuriString* name;
                name = furi_string_alloc_set(furi_hal_version_get_name_ptr());

                do {
                    // Open file for write
                    if(!flipper_format_file_open_always(file, furi_string_get_cstr(filepath))) {
                        break;
                    }

                    // Write header
                    if(!flipper_format_write_header_cstr(
                           file, furi_string_get_cstr(NAMEHEADER), 1)) {
                        break;
                    }

                    // Write comments
                    if(!flipper_format_write_comment_cstr(
                           file,
                           "Changing the value below will change your FlipperZero device name.")) {
                        break;
                    }

                    if(!flipper_format_write_comment_cstr(
                           file,
                           "Note: This is limited to 8 characters using the following: a-z, A-Z, 0-9, and _")) {
                        break;
                    }

                    if(!flipper_format_write_comment_cstr(
                           file, "It cannot contain any other characters.")) {
                        break;
                    }

                    //Write name
                    if(!flipper_format_write_string_cstr(
                           file, "Name", furi_string_get_cstr(name))) {
                        break;
                    }

                    res = true;
                } while(false);

                flipper_format_free(file);

                if(!res) {
                    FURI_LOG_E(TAG, "Save failed.");
                }

                furi_string_free(name);
            } else {
                //set name from file
                furi_hal_version_set_custom_name(furi_string_get_cstr(data));
            }
        }

        furi_string_free(data);
    }

    furi_string_free(filepath);
    furi_string_free(folderpath);
    furi_record_close(RECORD_STORAGE);
}