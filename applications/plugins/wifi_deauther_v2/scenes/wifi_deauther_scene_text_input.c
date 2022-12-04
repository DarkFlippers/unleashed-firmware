#include "../wifi_deauther_app_i.h"

void wifi_deauther_scene_text_input_callback(void* context) {
    WifideautherApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, WifideautherEventStartConsole);
}

void wifi_deauther_scene_text_input_on_enter(void* context) {
    WifideautherApp* app = context;

    if(false == app->is_custom_tx_string) {
        // Fill text input with selected string so that user can add to it
        size_t length = strlen(app->selected_tx_string);
        furi_assert(length < WIFI_deauther_TEXT_INPUT_STORE_SIZE);
        bzero(app->text_input_store, WIFI_deauther_TEXT_INPUT_STORE_SIZE);
        strncpy(app->text_input_store, app->selected_tx_string, length);

        // Add space - because flipper keyboard currently doesn't have a space
        app->text_input_store[length] = ' ';
        app->text_input_store[length + 1] = '\0';
        app->is_custom_tx_string = true;
    }

    // Setup view
    TextInput* text_input = app->text_input;
    // Add help message to header
    if(0 == strncmp("ssid -a -g", app->selected_tx_string, strlen("ssid -a -g"))) {
        text_input_set_header_text(text_input, "Enter # SSIDs to generate");
    } else if(0 == strncmp("ssid -a -n", app->selected_tx_string, strlen("ssid -a -n"))) {
        text_input_set_header_text(text_input, "Enter SSID name to add");
    } else if(0 == strncmp("ssid -r", app->selected_tx_string, strlen("ssid -r"))) {
        text_input_set_header_text(text_input, "Remove target from SSID list");
    } else if(0 == strncmp("select -a", app->selected_tx_string, strlen("select -a"))) {
        text_input_set_header_text(text_input, "Add target from AP list");
    } else if(0 == strncmp("select -s", app->selected_tx_string, strlen("select -s"))) {
        text_input_set_header_text(text_input, "Add target from SSID list");
    } else {
        text_input_set_header_text(text_input, "Add command arguments");
    }
    text_input_set_result_callback(
        text_input,
        wifi_deauther_scene_text_input_callback,
        app,
        app->text_input_store,
        WIFI_deauther_TEXT_INPUT_STORE_SIZE,
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, WifideautherAppViewTextInput);
}

bool wifi_deauther_scene_text_input_on_event(void* context, SceneManagerEvent event) {
    WifideautherApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == WifideautherEventStartConsole) {
            // Point to custom string to send
            app->selected_tx_string = app->text_input_store;
            scene_manager_next_scene(app->scene_manager, WifideautherAppViewConsoleOutput);
            consumed = true;
        }
    }

    return consumed;
}

void wifi_deauther_scene_text_input_on_exit(void* context) {
    WifideautherApp* app = context;

    text_input_reset(app->text_input);
}
