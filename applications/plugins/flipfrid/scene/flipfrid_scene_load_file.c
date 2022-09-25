#include "flipfrid_scene_load_file.h"
#include "flipfrid_scene_entrypoint.h"

#define LFRFID_APP_EXTENSION ".rfid"
#define LFRFID_APP_PATH_FOLDER "/ext/lfrfid"

bool flipfrid_load(FlipFridState* context, const char* file_path) {
    bool result = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    string_t temp_str;
    string_init(temp_str);
    do {
        if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
            FURI_LOG_E(TAG, "Error open file %s", file_path);
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Error open file");
            break;
        }

        // FileType
        if(!flipper_format_read_string(fff_data_file, "Filetype", temp_str)) {
            FURI_LOG_E(TAG, "Missing or incorrect Filetype");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Missing or incorrect Filetypes");
            break;
        } else {
            FURI_LOG_I(TAG, "Filetype: %s", string_get_cstr(temp_str));
        }

        // Key type
        if(!flipper_format_read_string(fff_data_file, "Key type", temp_str)) {
            FURI_LOG_E(TAG, "Missing or incorrect Key type");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Missing or incorrect Key type");
            break;
        } else {
            FURI_LOG_I(TAG, "Key type: %s", string_get_cstr(temp_str));

            if(context->proto == EM4100) {
                if(strcmp(string_get_cstr(temp_str), "EM4100") != 0) {
                    FURI_LOG_E(TAG, "Unsupported Key type");
                    string_reset(context->notification_msg);
                    string_set_str(context->notification_msg, "Unsupported Key type");
                    break;
                }
            } else {
                if(strcmp(string_get_cstr(temp_str), "HIDProx") != 0) {
                    FURI_LOG_E(TAG, "Unsupported Key type");
                    string_reset(context->notification_msg);
                    string_set_str(context->notification_msg, "Unsupported Key type");
                    break;
                }
            }
        }

        // Data
        if(!flipper_format_read_string(fff_data_file, "Data", context->data_str)) {
            FURI_LOG_E(TAG, "Missing or incorrect Data");
            string_reset(context->notification_msg);
            string_set_str(context->notification_msg, "Missing or incorrect Key");
            break;
        } else {
            FURI_LOG_I(TAG, "Key: %s", string_get_cstr(context->data_str));

            if(context->proto == EM4100) {
                if(string_size(context->data_str) != 14) {
                    FURI_LOG_E(TAG, "Incorrect Key length");
                    string_reset(context->notification_msg);
                    string_set_str(context->notification_msg, "Incorrect Key length");
                    break;
                }
            } else {
                if(string_size(context->data_str) != 17) {
                    FURI_LOG_E(TAG, "Incorrect Key length");
                    string_reset(context->notification_msg);
                    string_set_str(context->notification_msg, "Incorrect Key length");
                    break;
                }
            }

            // String to uint8_t
            for(uint8_t i = 0; i < 6; i++) {
                char temp_str2[3];
                temp_str2[0] = string_get_cstr(context->data_str)[i * 3];
                temp_str2[1] = string_get_cstr(context->data_str)[i * 3 + 1];
                temp_str2[2] = '\0';
                context->data[i] = (uint8_t)strtol(temp_str2, NULL, 16);
            }
        }

        result = true;
    } while(0);
    string_clear(temp_str);
    flipper_format_free(fff_data_file);
    if(result) {
        FURI_LOG_I(TAG, "Loaded successfully");
        string_reset(context->notification_msg);
        string_set_str(context->notification_msg, "Source loaded.");
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
            }
        }
    }
}

void flipfrid_scene_load_file_on_draw(Canvas* canvas, FlipFridState* context) {
    UNUSED(context);
    UNUSED(canvas);
}

bool flipfrid_load_protocol_from_file(FlipFridState* context) {
    string_t user_file_path;
    string_init(user_file_path);
    string_set_str(user_file_path, LFRFID_APP_PATH_FOLDER);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, LFRFID_APP_EXTENSION, &I_125_10px);

    // Input events and views are managed by file_select
    bool res = dialog_file_browser_show(
        context->dialogs, user_file_path, user_file_path, &browser_options);

    if(res) {
        res = flipfrid_load(context, string_get_cstr(user_file_path));
    }

    string_clear(user_file_path);

    return res;
}