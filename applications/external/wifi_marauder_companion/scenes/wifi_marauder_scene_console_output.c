#include "../wifi_marauder_app_i.h"

#include "../wifi_marauder_flasher.h"

char* _wifi_marauder_get_prefix_from_cmd(const char* command) {
    int end = strcspn(command, " ");
    char* prefix = (char*)malloc(sizeof(char) * (end + 1));
    strncpy(prefix, command, end);
    prefix[end] = '\0';
    return prefix;
}

bool _wifi_marauder_is_save_pcaps_enabled(WifiMarauderApp* app) {
    if(!app->ok_to_save_pcaps) {
        return false;
    }
    // If it is a script that contains a sniff function
    if(app->script != NULL) {
        return wifi_marauder_script_has_stage(app->script, WifiMarauderScriptStageTypeSniffRaw) ||
               wifi_marauder_script_has_stage(
                   app->script, WifiMarauderScriptStageTypeSniffBeacon) ||
               wifi_marauder_script_has_stage(
                   app->script, WifiMarauderScriptStageTypeSniffDeauth) ||
               wifi_marauder_script_has_stage(app->script, WifiMarauderScriptStageTypeSniffEsp) ||
               wifi_marauder_script_has_stage(
                   app->script, WifiMarauderScriptStageTypeSniffPmkid) ||
               wifi_marauder_script_has_stage(app->script, WifiMarauderScriptStageTypeSniffPwn);
    }
    // If it is a sniff function
    return app->is_command && app->selected_tx_string &&
           strncmp("sniff", app->selected_tx_string, strlen("sniff")) == 0;
}

void wifi_marauder_console_output_handle_rx_data_cb(uint8_t* buf, size_t len, void* context) {
    furi_assert(context);
    WifiMarauderApp* app = context;

    if(app->is_writing_log) {
        app->has_saved_logs_this_session = true;
        storage_file_write(app->log_file, buf, len);
    }

    // If text box store gets too big, then truncate it
    app->text_box_store_strlen += len;
    if(app->text_box_store_strlen >= WIFI_MARAUDER_TEXT_BOX_STORE_SIZE - 1) {
        furi_string_right(app->text_box_store, app->text_box_store_strlen / 2);
        app->text_box_store_strlen = furi_string_size(app->text_box_store) + len;
    }

    // Null-terminate buf and append to text box store
    buf[len] = '\0';
    furi_string_cat_printf(app->text_box_store, "%s", buf);
    view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventRefreshConsoleOutput);
}

void wifi_marauder_console_output_handle_rx_packets_cb(uint8_t* buf, size_t len, void* context) {
    furi_assert(context);
    WifiMarauderApp* app = context;

    if(app->is_writing_pcap) {
        storage_file_write(app->capture_file, buf, len);
    }
}

void wifi_marauder_scene_console_output_on_enter(void* context) {
    WifiMarauderApp* app = context;

    // Reset text box and set font
    TextBox* text_box = app->text_box;
    text_box_reset(text_box);
    text_box_set_font(text_box, TextBoxFontText);

    // Set focus on start or end
    if(app->focus_console_start) {
        text_box_set_focus(text_box, TextBoxFocusStart);
    } else {
        text_box_set_focus(text_box, TextBoxFocusEnd);
    }

    // Set command-related messages
    if(app->is_command) {
        furi_string_reset(app->text_box_store);
        app->text_box_store_strlen = 0;
        // Help message
        if(0 == strncmp("help", app->selected_tx_string, strlen("help"))) {
            const char* help_msg = "Marauder companion " WIFI_MARAUDER_APP_VERSION "\n";
            furi_string_cat_str(app->text_box_store, help_msg);
            app->text_box_store_strlen += strlen(help_msg);
        }
        // Stopscan message
        if(app->show_stopscan_tip) {
            const char* help_msg = "Press BACK to send stopscan\n";
            furi_string_cat_str(app->text_box_store, help_msg);
            app->text_box_store_strlen += strlen(help_msg);
        }
    }

    // Set starting text
    text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));

    // Set scene state and switch view
    scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneConsoleOutput, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewConsoleOutput);

    // Register callbacks to receive data
    // setup callback for general log rx thread
    if(app->flash_mode) {
        wifi_marauder_uart_set_handle_rx_data_cb(
            app->uart,
            wifi_marauder_flash_handle_rx_data_cb); // setup callback for general log rx thread
    } else {
        wifi_marauder_uart_set_handle_rx_data_cb(
            app->uart,
            wifi_marauder_console_output_handle_rx_data_cb); // setup callback for general log rx thread
    }
    wifi_marauder_uart_set_handle_rx_data_cb(
        app->lp_uart,
        wifi_marauder_console_output_handle_rx_packets_cb); // setup callback for packets rx thread

    if(app->flash_mode) {
        wifi_marauder_flash_start_thread(app);
    }

    // Get ready to send command
    if((app->is_command && app->selected_tx_string) || app->script) {
        const char* prefix =
            strlen(app->selected_tx_string) > 0 ?
                _wifi_marauder_get_prefix_from_cmd(app->selected_tx_string) : // Function name
                app->script->name; // Script name

        // Create files *before* sending command
        // (it takes time to iterate through the directory)
        if(app->ok_to_save_logs) {
            strcpy(
                app->log_file_path,
                sequential_file_resolve_path(
                    app->storage, MARAUDER_APP_FOLDER_LOGS, prefix, "log"));
            if(app->log_file_path != NULL) {
                if(storage_file_open(
                       app->log_file, app->log_file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
                    app->is_writing_log = true;
                } else {
                    dialog_message_show_storage_error(app->dialogs, "Cannot open log file");
                }
            } else {
                dialog_message_show_storage_error(app->dialogs, "Cannot resolve log path");
            }
        }

        // If it is a sniff function or script, open the pcap file for recording
        if(_wifi_marauder_is_save_pcaps_enabled(app)) {
            if(sequential_file_open(
                   app->storage, app->capture_file, MARAUDER_APP_FOLDER_PCAPS, prefix, "pcap")) {
                app->is_writing_pcap = true;
            } else {
                dialog_message_show_storage_error(app->dialogs, "Cannot open pcap file");
            }
        }

        // Send command with newline '\n'
        if(app->selected_tx_string) {
            wifi_marauder_uart_tx(
                (uint8_t*)(app->selected_tx_string), strlen(app->selected_tx_string));
            wifi_marauder_uart_tx((uint8_t*)("\n"), 1);
        }

        // Run the script if the file with the script has been opened
        if(app->script != NULL) {
            app->script_worker = wifi_marauder_script_worker_alloc();
            wifi_marauder_script_worker_start(app->script_worker, app->script);
        }
    }
}

bool wifi_marauder_scene_console_output_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        text_box_set_text(app->text_box, furi_string_get_cstr(app->text_box_store));
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    } else {
        if(app->flash_worker_busy) {
            // ignore button presses while flashing
            consumed = true;
        }
    }

    return consumed;
}

void wifi_marauder_scene_console_output_on_exit(void* context) {
    WifiMarauderApp* app = context;

    // Automatically stop the scan when exiting view
    if(app->is_command) {
        wifi_marauder_uart_tx((uint8_t*)("stopscan\n"), strlen("stopscan\n"));
        furi_delay_ms(50);
    }

    if(app->flash_mode) {
        wifi_marauder_flash_stop_thread(app);
    }

    // Unregister rx callback
    wifi_marauder_uart_set_handle_rx_data_cb(app->uart, NULL);
    wifi_marauder_uart_set_handle_rx_data_cb(app->lp_uart, NULL);

    wifi_marauder_script_worker_free(app->script_worker);
    app->script_worker = NULL;

    app->is_writing_pcap = false;
    if(app->capture_file && storage_file_is_open(app->capture_file)) {
        storage_file_close(app->capture_file);
    }

    app->is_writing_log = false;
    if(app->log_file && storage_file_is_open(app->log_file)) {
        storage_file_close(app->log_file);
    }
}
