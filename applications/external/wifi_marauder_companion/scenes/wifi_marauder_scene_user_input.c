#include "../wifi_marauder_app_i.h"

bool wifi_marauder_scene_user_input_validator_number_callback(
    const char* text,
    FuriString* error,
    void* context) {
    UNUSED(context);
    for(int i = 0; text[i] != '\0'; i++) {
        if(text[i] < '0' || text[i] > '9') {
            furi_string_printf(error, "This is not\na valid\nnumber!");
            return false;
        }
    }
    return true;
}

bool wifi_marauder_scene_user_input_validator_file_callback(
    const char* text,
    FuriString* error,
    void* context) {
    UNUSED(context);
    if(strlen(text) == 0) {
        furi_string_printf(error, "File name\ncannot be\nblank!");
        return false;
    }
    return true;
}

void wifi_marauder_scene_user_input_ok_callback(void* context) {
    WifiMarauderApp* app = context;

    File* file = NULL;
    char* file_path = NULL;

    switch(app->user_input_type) {
    // Writes the string value of the reference
    case WifiMarauderUserInputTypeString:
        if(app->user_input_string_reference != NULL) {
            strncpy(
                *app->user_input_string_reference,
                app->text_input_store,
                strlen(app->text_input_store) + 1);
            app->user_input_string_reference = NULL;
        }
        break;
    // Writes the numerical value of the reference
    case WifiMarauderUserInputTypeNumber:
        if(app->user_input_number_reference != NULL) {
            *app->user_input_number_reference = atoi(app->text_input_store);
            app->user_input_number_reference = NULL;
        }
        break;
    // Creates a file with the name entered by the user, if it does not exist
    case WifiMarauderUserInputTypeFileName:
        file = storage_file_alloc(app->storage);
        // Use application directory if not specified
        if(app->user_input_file_dir == NULL) {
            app->user_input_file_dir = strdup(MARAUDER_APP_FOLDER);
        }
        if(app->user_input_file_extension != NULL) {
            size_t file_path_len = strlen(app->user_input_file_dir) +
                                   strlen(app->text_input_store) +
                                   strlen(app->user_input_file_extension) + 3;
            file_path = (char*)malloc(file_path_len);
            snprintf(
                file_path,
                file_path_len,
                "%s/%s.%s",
                app->user_input_file_dir,
                app->text_input_store,
                app->user_input_file_extension);
        } else {
            size_t file_path_len =
                strlen(app->user_input_file_dir) + strlen(app->text_input_store) + 2;
            file_path = (char*)malloc(file_path_len);
            snprintf(
                file_path, file_path_len, "%s/%s", app->user_input_file_dir, app->text_input_store);
        }
        if(storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_NEW)) {
            storage_file_close(file);
        }
        // Free memory
        free(app->user_input_file_dir);
        app->user_input_file_dir = NULL;
        free(app->user_input_file_extension);
        app->user_input_file_extension = NULL;
        free(file_path);
        storage_file_free(file);
        break;
    default:
        break;
    }

    scene_manager_previous_scene(app->scene_manager);
}

void wifi_marauder_scene_user_input_on_enter(void* context) {
    WifiMarauderApp* app = context;

    switch(app->user_input_type) {
    // Loads the string value of the reference
    case WifiMarauderUserInputTypeString:
        wifi_text_input_set_header_text(app->text_input, "Enter value:");
        wifi_text_input_set_validator(app->text_input, NULL, app);
        if(app->user_input_string_reference != NULL) {
            strncpy(
                app->text_input_store,
                *app->user_input_string_reference,
                strlen(*app->user_input_string_reference) + 1);
        }
        break;
    // Loads the numerical value of the reference
    case WifiMarauderUserInputTypeNumber:
        wifi_text_input_set_header_text(app->text_input, "Enter a valid number:");
        wifi_text_input_set_validator(
            app->text_input, wifi_marauder_scene_user_input_validator_number_callback, app);
        if(app->user_input_number_reference != NULL) {
            char number_str[32];
            snprintf(number_str, sizeof(number_str), "%d", *app->user_input_number_reference);
            strncpy(app->text_input_store, number_str, strlen(number_str) + 1);
        }
        break;
    // File name
    case WifiMarauderUserInputTypeFileName:
        wifi_text_input_set_header_text(app->text_input, "Enter file name:");
        wifi_text_input_set_validator(
            app->text_input, wifi_marauder_scene_user_input_validator_file_callback, app);
        break;
    default:
        scene_manager_previous_scene(app->scene_manager);
        return;
    }

    wifi_text_input_set_result_callback(
        app->text_input,
        wifi_marauder_scene_user_input_ok_callback,
        app,
        app->text_input_store,
        WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE,
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewTextInput);
}

bool wifi_marauder_scene_user_input_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void wifi_marauder_scene_user_input_on_exit(void* context) {
    WifiMarauderApp* app = context;
    memset(app->text_input_store, 0, sizeof(app->text_input_store));
    wifi_text_input_reset(app->text_input);
}
