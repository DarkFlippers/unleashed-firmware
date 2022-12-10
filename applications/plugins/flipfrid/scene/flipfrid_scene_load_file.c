#include "flipfrid_scene_load_file.h"
#include "flipfrid_scene_entrypoint.h"
#include "RFID_Fuzzer_icons.h"

#define LFRFID_APP_EXTENSION ".rfid"
#define LFRFID_APP_PATH_FOLDER "/ext/lfrfid"

bool flipfrid_load(FlipFridState* context, const char* file_path) {
    bool result = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    FuriString* temp_str;
    temp_str = furi_string_alloc();
    do {
        if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
            FURI_LOG_E(TAG, "Error open file %s", file_path);
            furi_string_reset(context->notification_msg);
            furi_string_set(context->notification_msg, "Error open file");
            break;
        }

        // FileType
        if(!flipper_format_read_string(fff_data_file, "Filetype", temp_str)) {
            FURI_LOG_E(TAG, "Missing or incorrect Filetype");
            furi_string_reset(context->notification_msg);
            furi_string_set(context->notification_msg, "Missing or incorrect Filetypes");
            break;
        } else {
            FURI_LOG_I(TAG, "Filetype: %s", furi_string_get_cstr(temp_str));
        }

        // Key type
        if(!flipper_format_read_string(fff_data_file, "Key type", temp_str)) {
            FURI_LOG_E(TAG, "Missing or incorrect Key type");
            furi_string_reset(context->notification_msg);
            furi_string_set(context->notification_msg, "Missing or incorrect Key type");
            break;
        } else {
            FURI_LOG_I(TAG, "Key type: %s", furi_string_get_cstr(temp_str));

            if(context->proto == EM4100) {
                if(strcmp(furi_string_get_cstr(temp_str), "EM4100") != 0) {
                    FURI_LOG_E(TAG, "Unsupported Key type");
                    furi_string_reset(context->notification_msg);
                    furi_string_set(context->notification_msg, "Unsupported Key type");
                    break;
                }
            } else if(context->proto == PAC) {
                if(strcmp(furi_string_get_cstr(temp_str), "PAC/Stanley") != 0) {
                    FURI_LOG_E(TAG, "Unsupported Key type");
                    furi_string_reset(context->notification_msg);
                    furi_string_set(context->notification_msg, "Unsupported Key type");
                    break;
                }
            } else if(context->proto == H10301) {
                if(strcmp(furi_string_get_cstr(temp_str), "H10301") != 0) {
                    FURI_LOG_E(TAG, "Unsupported Key type");
                    furi_string_reset(context->notification_msg);
                    furi_string_set(context->notification_msg, "Unsupported Key type");
                    break;
                }
            } else {
                if(strcmp(furi_string_get_cstr(temp_str), "HIDProx") != 0) {
                    FURI_LOG_E(TAG, "Unsupported Key type");
                    furi_string_reset(context->notification_msg);
                    furi_string_set(context->notification_msg, "Unsupported Key type");
                    break;
                }
            }
        }

        // Data
        if(!flipper_format_read_string(fff_data_file, "Data", context->data_str)) {
            FURI_LOG_E(TAG, "Missing or incorrect Data");
            furi_string_reset(context->notification_msg);
            furi_string_set(context->notification_msg, "Missing or incorrect Key");
            break;
        } else {
            FURI_LOG_I(TAG, "Key: %s", furi_string_get_cstr(context->data_str));

            if(context->proto == EM4100) {
                if(furi_string_size(context->data_str) != 14) {
                    FURI_LOG_E(TAG, "Incorrect Key length");
                    furi_string_reset(context->notification_msg);
                    furi_string_set(context->notification_msg, "Incorrect Key length");
                    break;
                }
            } else if(context->proto == PAC) {
                if(furi_string_size(context->data_str) != 11) {
                    FURI_LOG_E(TAG, "Incorrect Key length");
                    furi_string_reset(context->notification_msg);
                    furi_string_set(context->notification_msg, "Incorrect Key length");
                    break;
                }
            } else if(context->proto == H10301) {
                if(furi_string_size(context->data_str) != 8) {
                    FURI_LOG_E(TAG, "Incorrect Key length");
                    furi_string_reset(context->notification_msg);
                    furi_string_set(context->notification_msg, "Incorrect Key length");
                    break;
                }
            } else {
                if(furi_string_size(context->data_str) != 17) {
                    FURI_LOG_E(TAG, "Incorrect Key length");
                    furi_string_reset(context->notification_msg);
                    furi_string_set(context->notification_msg, "Incorrect Key length");
                    break;
                }
            }

            // String to uint8_t
            for(uint8_t i = 0; i < 6; i++) {
                char temp_str2[3];
                temp_str2[0] = furi_string_get_cstr(context->data_str)[i * 3];
                temp_str2[1] = furi_string_get_cstr(context->data_str)[i * 3 + 1];
                temp_str2[2] = '\0';
                context->data[i] = (uint8_t)strtol(temp_str2, NULL, 16);
            }
        }

        result = true;
    } while(0);
    furi_string_free(temp_str);
    flipper_format_free(fff_data_file);
    if(result) {
        FURI_LOG_I(TAG, "Loaded successfully");
        furi_string_reset(context->notification_msg);
        furi_string_set(context->notification_msg, "Source loaded.");
    }
    return result;
}

void flipfrid_scene_load_file_on_enter(FlipFridState* context) {
    if(flipfrid_load_protocol_from_file(context)) {
        context->current_scene = SceneSelectField;
    } else {
        flipfrid_scene_entrypoint_on_enter(context);
        context->current_scene = SceneEntryPoint;
    }
}

void flipfrid_scene_load_file_on_exit(FlipFridState* context) {
    UNUSED(context);
}

void flipfrid_scene_load_file_on_tick(FlipFridState* context) {
    UNUSED(context);
}

void flipfrid_scene_load_file_on_event(FlipFridEvent event, FlipFridState* context) {
    if(event.evt_type == EventTypeKey) {
        if(event.input_type == InputTypeShort) {
            switch(event.key) {
            case InputKeyDown:
            case InputKeyUp:
            case InputKeyLeft:
            case InputKeyRight:
            case InputKeyOk:
            case InputKeyBack:
                context->current_scene = SceneEntryPoint;
                break;
            default:
                break;
            }
        }
    }
}

void flipfrid_scene_load_file_on_draw(Canvas* canvas, FlipFridState* context) {
    UNUSED(context);
    UNUSED(canvas);
}

bool flipfrid_load_protocol_from_file(FlipFridState* context) {
    FuriString* user_file_path;
    user_file_path = furi_string_alloc();
    furi_string_set(user_file_path, LFRFID_APP_PATH_FOLDER);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, LFRFID_APP_EXTENSION, &I_125_10px);
    browser_options.base_path = LFRFID_APP_PATH_FOLDER;

    // Input events and views are managed by file_select
    bool res = dialog_file_browser_show(
        context->dialogs, user_file_path, user_file_path, &browser_options);

    if(res) {
        res = flipfrid_load(context, furi_string_get_cstr(user_file_path));
    }

    furi_string_free(user_file_path);

    return res;
}