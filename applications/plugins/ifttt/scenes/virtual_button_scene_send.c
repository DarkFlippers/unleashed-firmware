#include "../ifttt_virtual_button.h"

static void virtual_button_scene_send_view_update_model(VirtualButtonApp* app) {
    power_get_info(app->power, &app->info);
}

void virtual_button_scene_send_view_on_enter(void* context) {
    VirtualButtonApp* app = context;
    virtual_button_scene_send_view_update_model(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, VirtualButtonAppViewSendView);
}

bool virtual_button_scene_send_view_on_event(void* context, SceneManagerEvent event) {
    VirtualButtonApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        virtual_button_scene_send_view_update_model(app);
        consumed = true;
    }
    return consumed;
}

void virtual_button_scene_send_view_on_exit(void* context) {
    UNUSED(context);
}
