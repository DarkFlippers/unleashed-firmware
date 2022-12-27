#include "../wifi_deauther_app_i.h"

void wifi_deauther_console_output_handle_rx_data_cb(uint8_t* buf, size_t len, void* context) {
    furi_assert(context);
    WifideautherApp* app = context;

    // If text box store gets too big, then truncate it
    app->text_box_store_strlen += len;
    if(app->text_box_store_strlen >= WIFI_deauther_TEXT_BOX_STORE_SIZE - 1) {
        furi_string_right(app->text_box_store, app->text_box_store_strlen / 2);
        app->text_box_store_strlen = furi_string_size(app->text_box_store);
    }

    // Null-terminate buf and append to text box store
    buf[len] = '\0';
    furi_string_cat_printf(app->text_box_store, "%s", buf);

    view_dispatcher_send_custom_event(app->view_dispatcher, WifideautherEventRefreshConsoleOutput);
}

void wifi_deauther_scene_console_output_on_enter(void* context) {
    WifideautherApp* app = context;

    // Register callback to receive data
    wifi_deauther_uart_set_handle_rx_data_cb(
        app->uart, wifi_deauther_console_output_handle_rx_data_cb); // setup callback for rx thread

    // Give a small delay to allow UART to settle.
    furi_delay_ms(600);

    TextBox* text_box = app->text_box;
    text_box_reset(app->text_box);
    text_box_set_font(text_box, TextBoxFontText);
    if(app->focus_console_start) {
        text_box_set_focus(text_box, TextBoxFocusStart);
    } else {
        text_box_set_focus(text_box, TextBoxFocusEnd);
    }
    if(app->is_command) {
        furi_string_reset(app->text_box_store);
        app->text_box_store_strlen = 0;

    } else { // "View Log" menu action
        text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
    }

    scene_manager_set_scene_state(app->scene_manager, WifideautherSceneConsoleOutput, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, WifideautherAppViewConsoleOutput);

    // Send command with newline '\n'
    if(app->is_command && app->selected_tx_string) {
        wifi_deauther_uart_tx(
            (uint8_t*)(app->selected_tx_string), strlen(app->selected_tx_string));
        wifi_deauther_uart_tx((uint8_t*)("\n"), 1);
    }
}

bool wifi_deauther_scene_console_output_on_event(void* context, SceneManagerEvent event) {
    WifideautherApp* app = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    }

    return consumed;
}

void wifi_deauther_scene_console_output_on_exit(void* context) {
    WifideautherApp* app = context;

    // Unregister rx callback
    wifi_deauther_uart_set_handle_rx_data_cb(app->uart, NULL);

    // Automatically stop the scan when exiting view
    if(app->is_command) {
        wifi_deauther_uart_tx((uint8_t*)("stopscan\n"), strlen("stopscan\n"));
    }
}