#include "../wifi_marauder_app_i.h"


void wifi_marauder_scene_text_input_callback(void* context) {
    WifiMarauderApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventStartConsole);
}

void wifi_marauder_scene_text_input_on_enter(void* context) {
    WifiMarauderApp* app = context;

    if (false == app->is_custom_tx_string) {
        // Fill text input with selected string so that user can add to it
        size_t length = strlen(app->selected_tx_string);
        furi_assert(length < WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE);
        bzero(app->text_input_store, WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE);
        strncpy(app->text_input_store, app->selected_tx_string, length);

        // Add space - because flipper keyboard currently doesn't have a space
        app->text_input_store[length] = ' ';
        app->text_input_store[length+1] = '\0';
        app->is_custom_tx_string = true;
    }

    // Setup view
    TextInput* text_input = app->text_input;
    text_input_set_header_text(text_input, "Add command arguments");
    text_input_set_result_callback(text_input, wifi_marauder_scene_text_input_callback, app, app->text_input_store, WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE, false);

    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewTextInput);
}

bool wifi_marauder_scene_text_input_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;

    if (event.type == SceneManagerEventTypeCustom) {
        if (event.event == WifiMarauderEventStartConsole) {
            // Point to custom string to send
            app->selected_tx_string = app->text_input_store;
            scene_manager_next_scene(app->scene_manager, WifiMarauderAppViewConsoleOutput);
            consumed = true;
        }
    }

    return consumed;
}

void wifi_marauder_scene_text_input_on_exit(void* context) {
    WifiMarauderApp* app = context;

    text_input_reset(app->text_input);
}
