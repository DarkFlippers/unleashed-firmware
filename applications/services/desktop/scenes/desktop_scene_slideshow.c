#include <storage/storage.h>

#include "../desktop_i.h"
#include "../views/desktop_view_slideshow.h"
#include "../views/desktop_events.h"
#include <power/power_service/power.h>

void desktop_scene_slideshow_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

void desktop_scene_slideshow_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    DesktopSlideshowView* slideshow_view = desktop->slideshow_view;

    desktop_view_slideshow_set_callback(slideshow_view, desktop_scene_slideshow_callback, desktop);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdSlideshow);
}

bool desktop_scene_slideshow_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;
    Power* power = NULL;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopSlideshowCompleted:
            scene_manager_previous_scene(desktop->scene_manager);
            consumed = true;
            break;
        case DesktopSlideshowPoweroff:
            power = furi_record_open(RECORD_POWER);
            power_off(power);
            furi_record_close(RECORD_POWER);
            consumed = true;
            break;

        default:
            break;
        }
    }
    return consumed;
}

void desktop_scene_slideshow_on_exit(void* context) {
    Desktop* desktop = context;
    storage_common_remove(desktop->storage, SLIDESHOW_FS_PATH);
}
