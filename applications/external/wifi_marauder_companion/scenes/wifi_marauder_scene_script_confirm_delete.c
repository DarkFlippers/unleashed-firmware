#include "../wifi_marauder_app_i.h"

void wifi_marauder_scene_script_confirm_delete_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    WifiMarauderApp* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void wifi_marauder_scene_script_confirm_delete_on_enter(void* context) {
    WifiMarauderApp* app = context;

    widget_add_button_element(
        app->widget,
        GuiButtonTypeLeft,
        "No",
        wifi_marauder_scene_script_confirm_delete_widget_callback,
        app);
    widget_add_button_element(
        app->widget,
        GuiButtonTypeRight,
        "Yes",
        wifi_marauder_scene_script_confirm_delete_widget_callback,
        app);

    widget_add_string_element(
        app->widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Are you sure?");
    widget_add_text_box_element(
        app->widget,
        0,
        12,
        128,
        38,
        AlignCenter,
        AlignCenter,
        "The script will be\npermanently deleted",
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewWidget);
}

bool wifi_marauder_scene_script_confirm_delete_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        // get which button press: "Yes" or "No"
        if(event.event == GuiButtonTypeRight) {
            // Yes
            if(app->script != NULL) {
                char script_path[256];
                snprintf(
                    script_path,
                    sizeof(script_path),
                    "%s/%s.json",
                    MARAUDER_APP_FOLDER_SCRIPTS,
                    app->script->name);
                storage_simply_remove(app->storage, script_path);
                wifi_marauder_script_free(app->script);
                app->script = NULL;

                DialogMessage* message = dialog_message_alloc();
                dialog_message_set_text(message, "Deleted!", 88, 32, AlignCenter, AlignCenter);
                dialog_message_set_icon(message, &I_DolphinCommon_56x48, 5, 6);
                dialog_message_set_buttons(message, NULL, "Ok", NULL);
                dialog_message_show(app->dialogs, message);
                dialog_message_free(message);
            }
        }
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void wifi_marauder_scene_script_confirm_delete_on_exit(void* context) {
    WifiMarauderApp* app = context;
    widget_reset(app->widget);
}
