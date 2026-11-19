#include "../sim800_app_i.h"

void sim800_scene_howto_on_enter(void* context) {
    Sim800App* app = context;

    bool lpuart = app->settings.uart_port == Sim800UartLpuart;
    sim800_howto_view_set_pins(app->howto_view, lpuart ? 15 : 13, lpuart ? 16 : 14);
    sim800_howto_view_reset(app->howto_view);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewHowto);
}

bool sim800_scene_howto_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void sim800_scene_howto_on_exit(void* context) {
    UNUSED(context);
}
