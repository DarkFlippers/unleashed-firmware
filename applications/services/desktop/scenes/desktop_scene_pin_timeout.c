#include <furi.h>
#include <gui/scene_manager.h>

#include "../desktop_i.h"
#include "../views/desktop_view_pin_timeout.h"
#include "desktop_scene.h"

static void desktop_scene_pin_timeout_callback(void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopPinTimeoutExit);
}

void desktop_scene_pin_timeout_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    uint32_t timeout =
        scene_manager_get_scene_state(desktop->scene_manager, DesktopScenePinTimeout);
    desktop_view_pin_timeout_start(desktop->pin_timeout_view, timeout);
    desktop_view_pin_timeout_set_callback(
        desktop->pin_timeout_view, desktop_scene_pin_timeout_callback, desktop);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdPinTimeout);
}

bool desktop_scene_pin_timeout_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopPinTimeoutExit:
            scene_manager_previous_scene(desktop->scene_manager);
            consumed = true;
            break;
        }
    }

    return consumed;
}

void desktop_scene_pin_timeout_on_exit(void* context) {
    UNUSED(context);
}
