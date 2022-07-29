#include "../wifi_marauder_app_i.h"

void wifi_marauder_console_output_handle_rx_data_cb(uint8_t *buf, size_t len, void* context) {
    furi_assert(context);
    WifiMarauderApp* app = context;

    // If text box store gets too big, then truncate it
    app->text_box_store_strlen += len;
    if (app->text_box_store_strlen >= WIFI_MARAUDER_TEXT_BOX_STORE_SIZE - 1) {
        string_right(app->text_box_store, app->text_box_store_strlen / 2);
        app->text_box_store_strlen = string_size(app->text_box_store);
    }

    // Null-terminate buf and append to text box store
    buf[len] = '\0';
    string_cat_printf(app->text_box_store, "%s", buf);

    view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventRefreshConsoleOutput);
}

void wifi_marauder_scene_console_output_on_enter(void* context) {
    WifiMarauderApp* app = context;

    TextBox* text_box = app->text_box;
    text_box_reset(app->text_box);
    text_box_set_font(text_box, TextBoxFontText);
    if (app->focus_console_start) {
        text_box_set_focus(text_box, TextBoxFocusStart);
    } else {
        text_box_set_focus(text_box, TextBoxFocusEnd);
    }
    if (app->is_command) {
        string_reset(app->text_box_store);
        app->text_box_store_strlen = 0;
    } else { // "View Log" menu action
        text_box_set_text(app->text_box, string_get_cstr(app->text_box_store));
    }

    scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneConsoleOutput, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewConsoleOutput);

    // Register callback to receive data
    wifi_marauder_uart_set_handle_rx_data_cb(app->uart, wifi_marauder_console_output_handle_rx_data_cb); // setup callback for rx thread

    // Send command with newline '\n'
    if (app->is_command && app->selected_tx_string) {
        wifi_marauder_uart_tx((uint8_t*)(app->selected_tx_string), strlen(app->selected_tx_string));
        wifi_marauder_uart_tx((uint8_t*)("\n"), 1);
    }
}

bool wifi_marauder_scene_console_output_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        text_box_set_text(app->text_box, string_get_cstr(app->text_box_store));
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    }

    return consumed;
}

void wifi_marauder_scene_console_output_on_exit(void* context) {
    WifiMarauderApp* app = context;

    // Unregister rx callback
    wifi_marauder_uart_set_handle_rx_data_cb(app->uart, NULL);

    // Automatically stop the scan when exiting view
    if (app->is_command) {
        wifi_marauder_uart_tx((uint8_t*)("stopscan\n"), strlen("stopscan\n"));
    }
}