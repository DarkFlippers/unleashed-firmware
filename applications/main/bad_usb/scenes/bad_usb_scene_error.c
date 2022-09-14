#include "../bad_usb_app_i.h"

typedef enum {
    BadUsbCustomEventErrorBack,
} BadUsbCustomEvent;

static void
    bad_usb_scene_error_event_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    BadUsbApp* app = context;

    if((result == GuiButtonTypeLeft) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(app->view_dispatcher, BadUsbCustomEventErrorBack);
    }
}

void bad_usb_scene_error_on_enter(void* context) {
    BadUsbApp* app = context;

    if(app->error == BadUsbAppErrorNoFiles) {
        widget_add_icon_element(app->widget, 0, 0, &I_SDQuestion_35x43);
        widget_add_string_multiline_element(
            app->widget,
            81,
            4,
            AlignCenter,
            AlignTop,
            FontSecondary,
            "No SD card or\napp data found.\nThis app will not\nwork without\nrequired files.");
        widget_add_button_element(
            app->widget, GuiButtonTypeLeft, "Back", bad_usb_scene_error_event_callback, app);
    } else if(app->error == BadUsbAppErrorCloseRpc) {
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
            "Disconnect from\nPC or phone to\nuse this function.");
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, BadUsbAppViewError);
}

bool bad_usb_scene_error_on_event(void* context, SceneManagerEvent event) {
    BadUsbApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BadUsbCustomEventErrorBack) {
            view_dispatcher_stop(app->view_dispatcher);
            consumed = true;
        }
    }
    return consumed;
}

void bad_usb_scene_error_on_exit(void* context) {
    BadUsbApp* app = context;
    widget_reset(app->widget);
}
