#include "../hid.h"
#include "../views.h"

static void hid_scene_exit_confirm_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    Hid* app = context;
    if(result == DialogExResultLeft) {
        view_dispatcher_stop(app->view_dispatcher);
    } else if(result == DialogExResultRight) {
        scene_manager_previous_scene(app->scene_manager);
    } else if(result == DialogExResultCenter) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, HidSceneMain);
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewSubmenu);
    }
}

void hid_scene_exit_confirm_on_enter(void* context) {
    Hid* app = context;

    // Exit dialog view
    dialog_ex_reset(app->dialog);
    dialog_ex_set_result_callback(app->dialog, hid_scene_exit_confirm_dialog_callback);
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_left_button_text(app->dialog, "Exit");
    dialog_ex_set_right_button_text(app->dialog, "Stay");
    dialog_ex_set_center_button_text(app->dialog, "Menu");
    dialog_ex_set_header(app->dialog, "Close Current App?", 16, 12, AlignLeft, AlignTop);

    view_dispatcher_switch_to_view(app->view_dispatcher, HidViewDialog);
}

bool hid_scene_exit_confirm_on_event(void* context, SceneManagerEvent event) {
    Hid* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void hid_scene_exit_confirm_on_exit(void* context) {
    Hid* app = context;

    dialog_ex_reset(app->dialog);
}
