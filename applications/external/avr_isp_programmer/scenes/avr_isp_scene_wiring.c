#include "../avr_isp_app_i.h"

void avr_isp_scene_wiring_on_enter(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    widget_add_icon_element(app->widget, 0, 0, &I_avr_wiring);
    view_dispatcher_switch_to_view(app->view_dispatcher, AvrIspViewWidget);
}

bool avr_isp_scene_wiring_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}
void avr_isp_scene_wiring_on_exit(void* context) {
    furi_assert(context);

    AvrIspApp* app = context;
    widget_reset(app->widget);
}
