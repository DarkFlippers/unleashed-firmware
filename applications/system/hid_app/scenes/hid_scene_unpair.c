#include "../hid.h"
#include "../views.h"
#include "hid_icons.h"

static void hid_scene_unpair_dialog_callback(DialogExResult result, void* context) {
    Hid* app = context;

    if(result == DialogExResultRight) {
        // Unpair all devices
        bt_hid_remove_pairing(app);

        // Show popup
        view_dispatcher_switch_to_view(app->view_dispatcher, HidViewPopup);
    } else if(result == DialogExResultLeft) {
        scene_manager_previous_scene(app->scene_manager);
    }
}

void hid_scene_unpair_popup_callback(void* context) {
    Hid* app = context;

    scene_manager_previous_scene(app->scene_manager);
}

void hid_scene_unpair_on_enter(void* context) {
    Hid* app = context;

    // Un-pair dialog view
    dialog_ex_reset(app->dialog);
    dialog_ex_set_result_callback(app->dialog, hid_scene_unpair_dialog_callback);
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_header(app->dialog, "Unpair the Device?", 64, 3, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(app->dialog, "Back");
    dialog_ex_set_right_button_text(app->dialog, "Unpair");

    // Un-pair success popup view
    popup_set_icon(app->popup, 48, 6, &I_DolphinDone_80x58);
    popup_set_header(app->popup, "Done", 14, 15, AlignLeft, AlignTop);
    popup_set_timeout(app->popup, 1500);
    popup_set_context(app->popup, app);
    popup_set_callback(app->popup, hid_scene_unpair_popup_callback);
    popup_enable_timeout(app->popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, HidViewDialog);
}

bool hid_scene_unpair_on_event(void* context, SceneManagerEvent event) {
    Hid* app = context;
    bool consumed = false;
    UNUSED(app);
    UNUSED(event);

    return consumed;
}

void hid_scene_unpair_on_exit(void* context) {
    Hid* app = context;

    dialog_ex_reset(app->dialog);
    popup_reset(app->popup);
}
