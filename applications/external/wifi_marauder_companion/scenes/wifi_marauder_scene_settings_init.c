#include "../wifi_marauder_app_i.h"

const char* Y = "Y";
const char* N = "N";

#define PROMPT_PCAPS 0
#define PROMPT_LOGS 1

void wifi_marauder_scene_settings_init_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    WifiMarauderApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void wifi_marauder_scene_settings_init_setup_widget(WifiMarauderApp* app) {
    Widget* widget = app->widget;

    widget_reset(widget);

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "No", wifi_marauder_scene_settings_init_widget_callback, app);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "Yes", wifi_marauder_scene_settings_init_widget_callback, app);

    if(app->which_prompt == PROMPT_PCAPS) {
        widget_add_string_element(widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Save pcaps?");
        widget_add_text_scroll_element(
            widget,
            0,
            12,
            128,
            38,
            "With compatible marauder\nfirmware, you can choose to\nsave captures (pcaps) to the\nflipper sd card here:\n" MARAUDER_APP_FOLDER_USER_PCAPS
            "\n\nYou can change this setting in the app at any time. Would\nyou like to enable this feature now?");
    } else {
        widget_add_string_element(widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Save logs?");
        widget_add_text_scroll_element(
            widget,
            0,
            12,
            128,
            38,
            "This app supports saving text\nlogs of console output to the\nflipper sd card here:\n" MARAUDER_APP_FOLDER_USER_LOGS
            "\n\nYou can change this setting in the app at any time. Would\nyou like to enable this feature now?");
    }
}

void wifi_marauder_scene_settings_init_on_enter(void* context) {
    WifiMarauderApp* app = context;

    app->which_prompt = PROMPT_PCAPS;
    wifi_marauder_scene_settings_init_setup_widget(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewWidget);
}

bool wifi_marauder_scene_settings_init_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        // get which button press: "Yes" or "No"
        if(event.event == GuiButtonTypeRight) {
            // Yes
            if(app->which_prompt == PROMPT_PCAPS) {
                app->ok_to_save_pcaps = true;
            } else {
                app->ok_to_save_logs = true;
            }
        } else if(event.event == GuiButtonTypeLeft) {
            // No
            if(app->which_prompt == PROMPT_PCAPS) {
                app->ok_to_save_pcaps = false;
            } else {
                app->ok_to_save_logs = false;
            }
        }

        // save setting to file, load next widget or scene
        if(app->which_prompt == PROMPT_PCAPS) {
            if(storage_file_open(
                   app->save_pcap_setting_file,
                   SAVE_PCAP_SETTING_FILEPATH,
                   FSAM_WRITE,
                   FSOM_CREATE_ALWAYS)) {
                const char* ok = app->ok_to_save_pcaps ? Y : N;
                storage_file_write(app->save_pcap_setting_file, ok, sizeof(ok));
            } else {
                dialog_message_show_storage_error(app->dialogs, "Cannot save settings");
            }
            storage_file_close(app->save_pcap_setting_file);
            // same scene, different-looking widget
            app->which_prompt = PROMPT_LOGS;
            wifi_marauder_scene_settings_init_setup_widget(app);
        } else {
            if(storage_file_open(
                   app->save_logs_setting_file,
                   SAVE_LOGS_SETTING_FILEPATH,
                   FSAM_WRITE,
                   FSOM_CREATE_ALWAYS)) {
                const char* ok = app->ok_to_save_logs ? Y : N;
                storage_file_write(app->save_logs_setting_file, ok, sizeof(ok));
            } else {
                dialog_message_show_storage_error(app->dialogs, "Cannot save settings");
            }
            storage_file_close(app->save_logs_setting_file);
            // go back to start scene (main menu)
            app->need_to_prompt_settings_init = false;
            scene_manager_previous_scene(app->scene_manager);
        }
        consumed = true;
    }

    return consumed;
}

void wifi_marauder_scene_settings_init_on_exit(void* context) {
    WifiMarauderApp* app = context;
    widget_reset(app->widget);
    if(storage_file_is_open(app->save_pcap_setting_file)) {
        storage_file_close(app->save_pcap_setting_file);
    }
    if(storage_file_is_open(app->save_logs_setting_file)) {
        storage_file_close(app->save_logs_setting_file);
    }
}
