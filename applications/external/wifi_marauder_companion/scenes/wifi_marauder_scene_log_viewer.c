#include "../wifi_marauder_app_i.h"

void wifi_marauder_scene_log_viewer_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    WifiMarauderApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

static void _read_log_page_into_text_store(WifiMarauderApp* app) {
    char temp[64 + 1];
    storage_file_seek(
        app->log_file, WIFI_MARAUDER_TEXT_BOX_STORE_SIZE * (app->open_log_file_page - 1), true);
    furi_string_reset(app->text_box_store);
    for(uint16_t i = 0; i < (WIFI_MARAUDER_TEXT_BOX_STORE_SIZE / (sizeof(temp) - 1)); i++) {
        uint16_t num_bytes = storage_file_read(app->log_file, temp, sizeof(temp) - 1);
        if(num_bytes == 0) {
            break;
        }
        temp[num_bytes] = '\0';
        furi_string_cat_str(app->text_box_store, temp);
    }
}

void wifi_marauder_scene_log_viewer_setup_widget(WifiMarauderApp* app, bool called_from_browse) {
    Widget* widget = app->widget;
    bool is_open = storage_file_is_open(app->log_file);
    bool should_open_log = (app->has_saved_logs_this_session || called_from_browse);
    if(is_open) {
        _read_log_page_into_text_store(app);
    } else if(
        should_open_log &&
        storage_file_open(app->log_file, app->log_file_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint64_t filesize = storage_file_size(app->log_file);
        app->open_log_file_num_pages = filesize / WIFI_MARAUDER_TEXT_BOX_STORE_SIZE;
        int extra_page = (filesize % WIFI_MARAUDER_TEXT_BOX_STORE_SIZE != 0) ? 1 : 0;
        app->open_log_file_num_pages = (filesize / WIFI_MARAUDER_TEXT_BOX_STORE_SIZE) + extra_page;
        app->open_log_file_page = 1;
        _read_log_page_into_text_store(app);
    } else {
        app->open_log_file_page = 0;
        app->open_log_file_num_pages = 0;
    }

    widget_reset(widget);

    if(furi_string_empty(app->text_box_store)) {
        char help_msg[256];
        snprintf(
            help_msg,
            sizeof(help_msg),
            "The log is empty! :(\nTry sending a command?\n\nSaving pcaps to flipper sdcard: %s\nSaving logs to flipper sdcard: %s",
            app->ok_to_save_pcaps ? "ON" : "OFF",
            app->ok_to_save_logs ? "ON" : "OFF");
        furi_string_set_str(app->text_box_store, help_msg);
    }

    widget_add_text_scroll_element(
        widget, 0, 0, 128, 53, furi_string_get_cstr(app->text_box_store));

    if(1 < app->open_log_file_page && app->open_log_file_page < app->open_log_file_num_pages) {
        // hide "Browse" text for middle pages
        widget_add_button_element(
            widget, GuiButtonTypeCenter, "", wifi_marauder_scene_log_viewer_widget_callback, app);
    } else {
        // only show "Browse" text on first and last page
        widget_add_button_element(
            widget,
            GuiButtonTypeCenter,
            "Browse",
            wifi_marauder_scene_log_viewer_widget_callback,
            app);
    }

    char pagecounter[100];
    snprintf(
        pagecounter,
        sizeof(pagecounter),
        "%d/%d",
        app->open_log_file_page,
        app->open_log_file_num_pages);
    if(app->open_log_file_page > 1) {
        if(app->open_log_file_page == app->open_log_file_num_pages) {
            // only show left side page-count on last page
            widget_add_button_element(
                widget,
                GuiButtonTypeLeft,
                pagecounter,
                wifi_marauder_scene_log_viewer_widget_callback,
                app);
        } else {
            widget_add_button_element(
                widget, GuiButtonTypeLeft, "", wifi_marauder_scene_log_viewer_widget_callback, app);
        }
    }
    if(app->open_log_file_page < app->open_log_file_num_pages) {
        widget_add_button_element(
            widget,
            GuiButtonTypeRight,
            pagecounter,
            wifi_marauder_scene_log_viewer_widget_callback,
            app);
    }
}

void wifi_marauder_scene_log_viewer_on_enter(void* context) {
    WifiMarauderApp* app = context;

    app->open_log_file_page = 0;
    app->open_log_file_num_pages = 0;
    bool saved_logs_exist = false;
    if(!app->has_saved_logs_this_session && furi_string_empty(app->text_box_store)) {
        // no commands sent yet this session, find last saved log
        if(storage_dir_open(app->log_file, MARAUDER_APP_FOLDER_LOGS)) {
            char name[70];
            char lastname[70];
            while(storage_dir_read(app->log_file, NULL, name, sizeof(name))) {
                // keep reading directory until last file is reached
                strlcpy(lastname, name, sizeof(lastname));
                saved_logs_exist = true;
            }
            if(saved_logs_exist) {
                snprintf(
                    app->log_file_path,
                    sizeof(app->log_file_path),
                    "%s/%s",
                    MARAUDER_APP_FOLDER_LOGS,
                    lastname);
            }
        }
        storage_dir_close(app->log_file);
    }

    wifi_marauder_scene_log_viewer_setup_widget(app, saved_logs_exist);

    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewWidget);
}

bool wifi_marauder_scene_log_viewer_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter) {
            // Browse
            FuriString* predefined_filepath = furi_string_alloc_set_str(MARAUDER_APP_FOLDER_LOGS);
            FuriString* selected_filepath = furi_string_alloc();
            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(&browser_options, ".log", &I_Text_10x10);
            if(dialog_file_browser_show(
                   app->dialogs, selected_filepath, predefined_filepath, &browser_options)) {
                strncpy(
                    app->log_file_path,
                    furi_string_get_cstr(selected_filepath),
                    sizeof(app->log_file_path));
                if(storage_file_is_open(app->log_file)) {
                    storage_file_close(app->log_file);
                }
                wifi_marauder_scene_log_viewer_setup_widget(app, true);
            }
            furi_string_free(selected_filepath);
            furi_string_free(predefined_filepath);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            // Advance page
            ++app->open_log_file_page;
            wifi_marauder_scene_log_viewer_setup_widget(app, false);
        } else if(event.event == GuiButtonTypeLeft) {
            // Previous page
            --app->open_log_file_page;
            wifi_marauder_scene_log_viewer_setup_widget(app, false);
        }
    }

    return consumed;
}

void wifi_marauder_scene_log_viewer_on_exit(void* context) {
    WifiMarauderApp* app = context;
    widget_reset(app->widget);
    if(storage_file_is_open(app->log_file)) {
        storage_file_close(app->log_file);
    }
}
