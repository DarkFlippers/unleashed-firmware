#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>

static ViewDispatcher* view_dispatcher;
static osMessageQueueId_t event_queue;

typedef enum {
    EventTypeGoAway,
    EventTypeGoToMainMenu,
    EventTypeSwitchToVertical,
    EventTypeSwitchToHorizontal,
} EventType;

// Nothing dangerous in settings some vars and flags inside callback
static void submenu_callback(void* context, uint32_t index) {
    EventType event = EventTypeGoAway;
    switch(index) {
    case 1:
        event = EventTypeSwitchToVertical;
        break;
    case 2:
        event = EventTypeSwitchToHorizontal;
        break;
    default:
        break;
    }

    osMessageQueuePut(event_queue, &event, 0, 0);
}

uint32_t previous_exit_callback(void* context) {
    EventType event = EventTypeGoAway;
    osMessageQueuePut(event_queue, &event, 0, 0);
    return VIEW_IGNORE;
}

uint32_t previous_callback(void* context) {
    EventType event = EventTypeGoToMainMenu;
    osMessageQueuePut(event_queue, &event, 0, 0);
    return VIEW_IGNORE;
}

int32_t application_vertical_screen(void* p) {
    event_queue = osMessageQueueNew(8, sizeof(EventType), NULL);

    view_dispatcher = view_dispatcher_alloc();
    Gui* gui = furi_record_open("gui");
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    Submenu* submenu = submenu_alloc();
    View* submenu_view = submenu_get_view(submenu);
    view_set_previous_callback(submenu_view, previous_exit_callback);
    view_set_orientation(submenu_view, ViewOrientationVertical);
    submenu_add_item(submenu, "VerSubm", 1, submenu_callback, view_dispatcher);
    submenu_add_item(submenu, "HorSubm", 2, submenu_callback, view_dispatcher);
    view_dispatcher_add_view(view_dispatcher, 1, submenu_view);

    Submenu* submenu_vertical = submenu_alloc();
    View* submenu_vertical_view = submenu_get_view(submenu_vertical);
    view_set_previous_callback(submenu_vertical_view, previous_callback);
    view_set_orientation(submenu_vertical_view, ViewOrientationVertical);
    submenu_add_item(submenu_vertical, "Vert1", 1, NULL, view_dispatcher);
    submenu_add_item(submenu_vertical, "Vert2", 2, NULL, view_dispatcher);
    view_dispatcher_add_view(view_dispatcher, 2, submenu_vertical_view);

    Submenu* submenu_horizontal = submenu_alloc();
    View* submenu_horizontal_view = submenu_get_view(submenu_horizontal);
    view_set_previous_callback(submenu_horizontal_view, previous_callback);
    view_set_orientation(submenu_horizontal_view, ViewOrientationHorizontal);
    submenu_add_item(submenu_horizontal, "Horiz1", 1, NULL, view_dispatcher);
    submenu_add_item(submenu_horizontal, "Horiz2", 2, NULL, view_dispatcher);
    view_dispatcher_add_view(view_dispatcher, 3, submenu_horizontal_view);

    view_dispatcher_switch_to_view(view_dispatcher, 1);

    while(1) {
        EventType event;
        furi_check(osMessageQueueGet(event_queue, &event, NULL, osWaitForever) == osOK);
        if(event == EventTypeGoAway) {
            break;
        } else if(event == EventTypeGoToMainMenu) {
            view_dispatcher_switch_to_view(view_dispatcher, 1);
        } else if(event == EventTypeSwitchToVertical) {
            view_dispatcher_switch_to_view(view_dispatcher, 2);
        } else if(event == EventTypeSwitchToHorizontal) {
            view_dispatcher_switch_to_view(view_dispatcher, 3);
        }
    }

    view_dispatcher_remove_view(view_dispatcher, 1);
    view_dispatcher_remove_view(view_dispatcher, 2);
    view_dispatcher_remove_view(view_dispatcher, 3);
    submenu_free(submenu);
    submenu_free(submenu_vertical);
    submenu_free(submenu_horizontal);
    view_dispatcher_free(view_dispatcher);
    osMessageQueueDelete(event_queue);
    furi_record_close("gui");

    return 0;
}
