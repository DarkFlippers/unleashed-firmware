#include "../u2f_app_i.h"

static void u2f_scene_error_event_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    U2fApp* app = context;

    if((result == GuiButtonTypeLeft) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(app->view_dispatcher, U2fCustomEventErrorBack);
    }
}

void u2f_scene_error_on_enter(void* context) {
    U2fApp* app = context;

    if(app->error == U2fAppErrorNoFiles) {
        widget_add_icon_element(app->widget, 0, 0, &I_SDQuestion_35x43);
        widget_add_string_multiline_element(
            app->widget,
            81,
            4,
            AlignCenter,
            AlignTop,
            FontSecondary,
            "No SD card or\napp data found.\nThis app will not\nwork without\nrequired files.");
    } else if(app->error == U2fAppErrorCloseRpc) {
        widget_add_string_multiline_element(
            app->widget,
            63,
            10,
            AlignCenter,
            AlignTop,
            FontSecondary,
            "Disconnect from\ncompanion app\nto use this function");
    }

    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Back", u2f_scene_error_event_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, U2fAppViewError);
}

bool u2f_scene_error_on_event(void* context, SceneManagerEvent event) {
    U2fApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == U2fCustomEventErrorBack) {
            view_dispatcher_stop(app->view_dispatcher);
            consumed = true;
        }
    }

    return consumed;
}

void u2f_scene_error_on_exit(void* context) {
    U2fApp* app = context;
    widget_reset(app->widget);
}
