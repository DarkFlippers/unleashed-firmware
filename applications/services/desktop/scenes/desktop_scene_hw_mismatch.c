#include <gui/scene_manager.h>
#include <furi_hal.h>

#include "desktop_scene.h"
#include "../desktop_i.h"

void desktop_scene_hw_mismatch_callback(void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopHwMismatchExit);
}

void desktop_scene_hw_mismatch_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    furi_assert(desktop);
    Popup* popup = desktop->popup;

    char* text_buffer = malloc(256);
    scene_manager_set_scene_state(
        desktop->scene_manager, DesktopSceneHwMismatch, (uint32_t)text_buffer);

    snprintf(
        text_buffer,
        256,
        "HW target: %d\nFW target: %d",
        furi_hal_version_get_hw_target(),
        version_get_target(NULL));
    popup_set_context(popup, desktop);
    popup_set_header(
        popup, "!!!! HW Mismatch !!!!", 64, 12 + STATUS_BAR_Y_SHIFT, AlignCenter, AlignBottom);
    popup_set_text(popup, text_buffer, 64, 33 + STATUS_BAR_Y_SHIFT, AlignCenter, AlignCenter);
    popup_set_callback(popup, desktop_scene_hw_mismatch_callback);
    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdPopup);
}

bool desktop_scene_hw_mismatch_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopHwMismatchExit:
            scene_manager_previous_scene(desktop->scene_manager);
            consumed = true;
            break;
        default:
            break;
        }
    }
    return consumed;
}

void desktop_scene_hw_mismatch_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;
    furi_assert(desktop);

    Popup* popup = desktop->popup;
    popup_reset(popup);

    char* text_buffer =
        (char*)scene_manager_get_scene_state(desktop->scene_manager, DesktopSceneHwMismatch);
    free(text_buffer);
    scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneHwMismatch, 0);
}
