#include "../uart_terminal_app_i.h"

void uart_terminal_console_output_handle_rx_data_cb(uint8_t* buf, size_t len, void* context) {
    furi_assert(context);
    UART_TerminalApp* app = context;

    // If text box store gets too big, then truncate it
    app->text_box_store_strlen += len;
    if(app->text_box_store_strlen >= UART_TERMINAL_TEXT_BOX_STORE_SIZE - 1) {
        furi_string_right(app->text_box_store, app->text_box_store_strlen / 2);
        app->text_box_store_strlen = furi_string_size(app->text_box_store) + len;
    }

    // Null-terminate buf and append to text box store
    buf[len] = '\0';
    furi_string_cat_printf(app->text_box_store, "%s", buf);

    view_dispatcher_send_custom_event(
        app->view_dispatcher, UART_TerminalEventRefreshConsoleOutput);
}

void uart_terminal_scene_console_output_on_enter(void* context) {
    UART_TerminalApp* app = context;

    TextBox* text_box = app->text_box;
    text_box_reset(app->text_box);
    text_box_set_font(text_box, TextBoxFontText);
    if(app->focus_console_start) {
        text_box_set_focus(text_box, TextBoxFocusStart);
    } else {
        text_box_set_focus(text_box, TextBoxFocusEnd);
    }

    //Change baudrate ///////////////////////////////////////////////////////////////////////////
    if(0 == strncmp("2400", app->selected_tx_string, strlen("2400")) && app->BAUDRATE != 2400) {
        uart_terminal_uart_free(app->uart);
        app->BAUDRATE = 2400;
        app->uart = uart_terminal_uart_init(app);
    }
    if(0 == strncmp("9600", app->selected_tx_string, strlen("9600")) && app->BAUDRATE != 9600) {
        uart_terminal_uart_free(app->uart);
        app->BAUDRATE = 9600;
        app->uart = uart_terminal_uart_init(app);
    }
    if(0 == strncmp("19200", app->selected_tx_string, strlen("19200")) && app->BAUDRATE != 19200) {
        uart_terminal_uart_free(app->uart);
        app->BAUDRATE = 19200;
        app->uart = uart_terminal_uart_init(app);
    }
    if(0 == strncmp("38400", app->selected_tx_string, strlen("38400")) && app->BAUDRATE != 38400) {
        uart_terminal_uart_free(app->uart);
        app->BAUDRATE = 38400;
        app->uart = uart_terminal_uart_init(app);
    }
    if(0 == strncmp("57600", app->selected_tx_string, strlen("57600")) && app->BAUDRATE != 57600) {
        uart_terminal_uart_free(app->uart);
        app->BAUDRATE = 57600;
        app->uart = uart_terminal_uart_init(app);
    }
    if(0 == strncmp("115200", app->selected_tx_string, strlen("115200")) &&
       app->BAUDRATE != 115200) {
        uart_terminal_uart_free(app->uart);
        app->BAUDRATE = 115200;
        app->uart = uart_terminal_uart_init(app);
    }
    if(0 == strncmp("230400", app->selected_tx_string, strlen("230400")) &&
       app->BAUDRATE != 230400) {
        uart_terminal_uart_free(app->uart);
        app->BAUDRATE = 230400;
        app->uart = uart_terminal_uart_init(app);
    }
    if(0 == strncmp("460800", app->selected_tx_string, strlen("460800")) &&
       app->BAUDRATE != 460800) {
        uart_terminal_uart_free(app->uart);
        app->BAUDRATE = 460800;
        app->uart = uart_terminal_uart_init(app);
    }
    if(0 == strncmp("921600", app->selected_tx_string, strlen("921600")) &&
       app->BAUDRATE != 921600) {
        uart_terminal_uart_free(app->uart);
        app->BAUDRATE = 921600;
        app->uart = uart_terminal_uart_init(app);
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    if(app->is_command) {
        furi_string_reset(app->text_box_store);
        app->text_box_store_strlen = 0;

        if(0 == strncmp("help", app->selected_tx_string, strlen("help"))) {
            const char* help_msg =
                "UART terminal for Flipper\n\nI'm in github: cool4uma\n\nThis app is a modified\nWiFi Marauder companion,\nThanks 0xchocolate(github)\nfor great code and app.\n\n";
            furi_string_cat_str(app->text_box_store, help_msg);
            app->text_box_store_strlen += strlen(help_msg);
        }

        if(app->show_stopscan_tip) {
            const char* help_msg = "Press BACK to return\n";
            furi_string_cat_str(app->text_box_store, help_msg);
            app->text_box_store_strlen += strlen(help_msg);
        }
    }

    // Set starting text - for "View Log", this will just be what was already in the text box store
    text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));

    scene_manager_set_scene_state(app->scene_manager, UART_TerminalSceneConsoleOutput, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, UART_TerminalAppViewConsoleOutput);

    // Register callback to receive data
    uart_terminal_uart_set_handle_rx_data_cb(
        app->uart, uart_terminal_console_output_handle_rx_data_cb); // setup callback for rx thread

    // Send command with CR+LF or newline '\n'
    if(app->is_command && app->selected_tx_string) {
        if(app->TERMINAL_MODE == 1) {
            uart_terminal_uart_tx(
                (uint8_t*)(app->selected_tx_string), strlen(app->selected_tx_string));
            uart_terminal_uart_tx((uint8_t*)("\r\n"), 2);
        } else {
            uart_terminal_uart_tx(
                (uint8_t*)(app->selected_tx_string), strlen(app->selected_tx_string));
            uart_terminal_uart_tx((uint8_t*)("\n"), 1);
        }
    }
}

bool uart_terminal_scene_console_output_on_event(void* context, SceneManagerEvent event) {
    UART_TerminalApp* app = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    }

    return consumed;
}

void uart_terminal_scene_console_output_on_exit(void* context) {
    UART_TerminalApp* app = context;

    // Unregister rx callback
    uart_terminal_uart_set_handle_rx_data_cb(app->uart, NULL);
}