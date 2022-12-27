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
    if(0 == strncmp("select aps", app->selected_tx_string, strlen("select aps"))) {
        text_input_set_header_text(text_input, "Enter SSID ID to attack");
    } else if(0 == strncmp("select stations", app->selected_tx_string, strlen("select stations"))) {
        text_input_set_header_text(text_input, "Enter Station ID to attack");
    }
    if(0 == strncmp("deselect aps", app->selected_tx_string, strlen("deselect aps"))) {
        text_input_set_header_text(text_input, "Enter SSID ID to remove");
    } else if(
        0 == strncmp("deselect stations", app->selected_tx_string, strlen("deselect stations"))) {
        text_input_set_header_text(text_input, "Enter Station ID to remove");
    } else if(0 == strncmp("get settings", app->selected_tx_string, strlen("get settings"))) {
        text_input_set_header_text(text_input, "Get setting. Enter for all");
    } else if(
        0 ==
        strncmp(
            "set webinterface false", app->selected_tx_string, strlen("set webinterface false"))) {
        text_input_set_header_text(text_input, "Disable PWNED management AP");
    } else if(0 == strncmp("set ssid: pwned", app->selected_tx_string, strlen("set ssid: pwned"))) {
        text_input_set_header_text(text_input, "Change management SSID");
    } else if(
        0 ==
        strncmp(
            "set password: deauther", app->selected_tx_string, strlen("set password: deauther"))) {
        text_input_set_header_text(text_input, "Change management PWD");
    } else if(0 == strncmp("save settings", app->selected_tx_string, strlen("save settings"))) {
        text_input_set_header_text(text_input, "Save Settings");
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

///*
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
//*/