#include <power/power_service/power.h>
#include <storage/storage.h>

#include "../desktop_i.h"
#include "../views/desktop_view_first_start.h"
#include "../views/desktop_events.h"

void desktop_scene_first_start_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

void desktop_scene_first_start_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    DesktopFirstStartView* first_start_view = desktop->first_start_view;

    desktop_first_start_set_callback(
        first_start_view, desktop_scene_first_start_callback, desktop);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdFirstStart);
}

bool desktop_scene_first_start_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;
    Storage* storage = NULL;
    Power* power = NULL;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopFirstStartCompleted:
            storage = furi_record_open("storage");
            storage_common_remove(storage, "/int/first_start");
            furi_record_close("storage");
            scene_manager_previous_scene(desktop->scene_manager);
            consumed = true;
            break;
        case DesktopFirstStartPoweroff:
            power = furi_record_open("power");
            power_off(power);
            furi_record_close("power");
            consumed = true;
            break;

        default:
            break;
        }
    }
    return consumed;
}

void desktop_scene_first_start_on_exit(void* context) {
    UNUSED(context);
}
