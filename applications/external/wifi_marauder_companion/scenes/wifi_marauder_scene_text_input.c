#include "../wifi_marauder_app_i.h"

void wifi_marauder_scene_text_input_callback(void* context) {
    WifiMarauderApp* app = context;

    switch(app->special_case_input_step) {
    case 0: // most commands
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventStartConsole);
        break;
    case 1: // special case for deauth: save source MAC
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventSaveSourceMac);
        break;
    case 2: // special case for deauth: save destination MAC
        view_dispatcher_send_custom_event(
            app->view_dispatcher, WifiMarauderEventSaveDestinationMac);
        break;
    default:
        break;
    }
}

void wifi_marauder_scene_text_input_on_enter(void* context) {
    WifiMarauderApp* app = context;

    if(0 ==
       strncmp("attack -t deauth -s", app->selected_tx_string, strlen("attack -t deauth -s"))) {
        // Special case for manual deauth input
        app->special_case_input_step = 1;
        bzero(app->text_input_store, WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE);
    } else if(false == app->is_custom_tx_string) {
        // Most commands
        app->special_case_input_step = 0;

        // Fill text input with selected string so that user can add to it
        size_t length = strlen(app->selected_tx_string);
        furi_assert(length < WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE);
        bzero(app->text_input_store, WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE);
        strncpy(app->text_input_store, app->selected_tx_string, length);

        // Add space - because flipper keyboard currently doesn't have a space
        app->text_input_store[length] = ' ';
        app->text_input_store[length + 1] = '\0';
        app->is_custom_tx_string = true;
    }

    // Setup view
    WIFI_TextInput* text_input = app->text_input;
    // Add help message to header
    if(app->special_case_input_step == 1) {
        wifi_text_input_set_header_text(text_input, "Enter source MAC");
    } else if(0 == strncmp("ssid -a -g", app->selected_tx_string, strlen("ssid -a -g"))) {
        wifi_text_input_set_header_text(text_input, "Enter # SSIDs to generate");
    } else if(0 == strncmp("ssid -a -n", app->selected_tx_string, strlen("ssid -a -n"))) {
        wifi_text_input_set_header_text(text_input, "Enter SSID name to add");
    } else if(0 == strncmp("ssid -r", app->selected_tx_string, strlen("ssid -r"))) {
        wifi_text_input_set_header_text(text_input, "Remove target from SSID list");
    } else if(0 == strncmp("select -a", app->selected_tx_string, strlen("select -a"))) {
        wifi_text_input_set_header_text(text_input, "Add target from AP list");
    } else if(0 == strncmp("select -s", app->selected_tx_string, strlen("select -s"))) {
        wifi_text_input_set_header_text(text_input, "Add target from SSID list");
    } else {
        wifi_text_input_set_header_text(text_input, "Add command arguments");
    }
    wifi_text_input_set_result_callback(
        text_input,
        wifi_marauder_scene_text_input_callback,
        app,
        app->text_input_store,
        WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE,
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewTextInput);
}

bool wifi_marauder_scene_text_input_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == WifiMarauderEventStartConsole) {
            // Point to custom string to send
            app->selected_tx_string = app->text_input_store;
            scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
            consumed = true;
        } else if(event.event == WifiMarauderEventSaveSourceMac) {
            if(12 != strlen(app->text_input_store)) {
                wifi_text_input_set_header_text(app->text_input, "MAC must be 12 hex chars!");
            } else {
                snprintf(
                    app->special_case_input_src_addr,
                    sizeof(app->special_case_input_src_addr),
                    "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
                    app->text_input_store[0],
                    app->text_input_store[1],
                    app->text_input_store[2],
                    app->text_input_store[3],
                    app->text_input_store[4],
                    app->text_input_store[5],
                    app->text_input_store[6],
                    app->text_input_store[7],
                    app->text_input_store[8],
                    app->text_input_store[9],
                    app->text_input_store[10],
                    app->text_input_store[11]);

                // Advance scene to input destination MAC, clear text input
                app->special_case_input_step = 2;
                bzero(app->text_input_store, WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE);
                wifi_text_input_set_header_text(app->text_input, "Enter destination MAC");
            }
            consumed = true;
        } else if(event.event == WifiMarauderEventSaveDestinationMac) {
            if(12 != strlen(app->text_input_store)) {
                wifi_text_input_set_header_text(app->text_input, "MAC must be 12 hex chars!");
            } else {
                snprintf(
                    app->special_case_input_dst_addr,
                    sizeof(app->special_case_input_dst_addr),
                    "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
                    app->text_input_store[0],
                    app->text_input_store[1],
                    app->text_input_store[2],
                    app->text_input_store[3],
                    app->text_input_store[4],
                    app->text_input_store[5],
                    app->text_input_store[6],
                    app->text_input_store[7],
                    app->text_input_store[8],
                    app->text_input_store[9],
                    app->text_input_store[10],
                    app->text_input_store[11]);

                // Construct command with source and destination MACs
                snprintf(
                    app->text_input_store,
                    WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE,
                    "attack -t deauth -s %18s -d %18s",
                    app->special_case_input_src_addr,
                    app->special_case_input_dst_addr);
                app->selected_tx_string = app->text_input_store;
                scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
            }
            consumed = true;
        }
    }

    return consumed;
}

void wifi_marauder_scene_text_input_on_exit(void* context) {
    WifiMarauderApp* app = context;

    wifi_text_input_reset(app->text_input);
}
