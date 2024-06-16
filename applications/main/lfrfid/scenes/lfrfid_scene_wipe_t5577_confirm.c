#include "../lfrfid_i.h"
#include <dialogs/dialogs.h>

#define LFRFID_PAGE0_MAX_SIZE 8
#define LFRFID_PAGE1_MAX_SIZE 4
#define SCREEN_WIDTH_CENTER 64
#define SCREEN_HEIGHT_CENTER 32

void lfrfid_scene_wipe_t5577_confirm_on_enter(void* context) {
    LfRfid* app = context;

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(
        message, "This will delete all T5577 user blocks data.", 0, 0, AlignLeft, AlignTop);
    dialog_message_set_buttons(message, NULL, NULL, "Yes");
    dialog_message_set_text(
        message,
        "Are you sure?",
        SCREEN_WIDTH_CENTER,
        SCREEN_HEIGHT_CENTER,
        AlignCenter,
        AlignCenter);
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);

    Widget* widget = app->widget;

    widget_add_button_element(widget, GuiButtonTypeLeft, "Exit", lfrfid_widget_callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "Start", lfrfid_widget_callback, app);
    widget_add_string_multiline_element(
        widget, 64, 22, AlignCenter, AlignBottom, FontPrimary, "Apply tag to\nFlipper's back");
    widget_add_string_multiline_element(
        widget,
        64,
        45,
        AlignCenter,
        AlignBottom,
        FontSecondary,
        "And don't move it\nwhile process is running");

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewWidget);
}

bool lfrfid_scene_wipe_t5577_confirm_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true; // Ignore Back button presses
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_search_and_switch_to_previous_scene(
                scene_manager, LfRfidSceneExtraActions);
        } else if(event.event == GuiButtonTypeRight)
            scene_manager_next_scene(scene_manager, LfRfidSceneWipeT5577);
    }

    return consumed;
}

void lfrfid_scene_wipe_t5577_confirm_on_exit(void* context) {
    LfRfid* app = context;
    widget_reset(app->widget);
}