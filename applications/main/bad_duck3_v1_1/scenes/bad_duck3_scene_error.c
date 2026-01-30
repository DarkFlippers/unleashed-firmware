#include "../bad_duck3_app_i.h"

void bad_duck3_scene_error_on_enter(void* context) {
    BadDuck3App* app = context;

    widget_reset(app->widget);

    if(app->error == BadDuck3AppErrorCloseRpc) {
        widget_add_icon_element(app->widget, 78, 0, &I_ActiveConnection_50x64);
        widget_add_string_multiline_element(
            app->widget, 3, 2, AlignLeft, AlignTop, FontPrimary, "Connection\nis active!");
        widget_add_string_multiline_element(
            app->widget,
            3,
            30,
            AlignLeft,
            AlignTop,
            FontSecondary,
            "Interrupted by\nanother app.\nClose it to continue.");
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, BadDuck3AppViewWidget);
}

bool bad_duck3_scene_error_on_event(void* context, SceneManagerEvent event) {
    BadDuck3App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        view_dispatcher_stop(app->view_dispatcher);
        consumed = true;
    }

    return consumed;
}

void bad_duck3_scene_error_on_exit(void* context) {
    BadDuck3App* app = context;
    widget_reset(app->widget);
}
