#include <gui/scene_manager.h>
#include <furi_hal.h>

#include "desktop_scene.h"
#include "../desktop_i.h"

void desktop_scene_secure_enclave_callback(void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopEnclaveExit);
}

void desktop_scene_secure_enclave_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    furi_assert(desktop);

    Popup* popup = desktop->popup;
    popup_set_context(popup, desktop);
    popup_set_header(
        popup, "No Factory Keys Found", 64, 12 + STATUS_BAR_Y_SHIFT, AlignCenter, AlignBottom);
    popup_set_text(
        popup,
        "Secure Enclave is damaged.\n"
        "Some apps will not work.",
        64,
        33 + STATUS_BAR_Y_SHIFT,
        AlignCenter,
        AlignCenter);
    popup_set_callback(popup, desktop_scene_secure_enclave_callback);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdPopup);
}

bool desktop_scene_secure_enclave_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopEnclaveExit:
            scene_manager_previous_scene(desktop->scene_manager);
            consumed = true;
            break;

        default:
            break;
        }
    }
    return consumed;
}

void desktop_scene_secure_enclave_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;
    furi_assert(desktop);

    Popup* popup = desktop->popup;
    popup_reset(popup);
}
