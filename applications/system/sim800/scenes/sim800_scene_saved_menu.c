#include "../sim800_app_i.h"

static void sim800_saved_menu_button_callback(Sim800EntryButton button, void* context) {
    Sim800App* app = context;
    uint32_t event = Sim800CustomEventButtonCenter;

    if(button == Sim800EntryButtonSms) {
        event = Sim800CustomEventButtonLeft;
    } else if(button == Sim800EntryButtonEdit) {
        event = Sim800CustomEventButtonRight;
    }

    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

static void sim800_saved_menu_task(Sim800App* app) {
    FuriString* value = furi_string_alloc();

    app->net_rssi = sim800_at_rssi(app->uart);
    sim800_entry_view_set_rssi(app->entry_view, app->net_rssi);
    sim800_at_operator(app->uart, value);
    furi_string_set(app->net_operator, value);
    sim800_entry_view_set_operator(app->entry_view, furi_string_get_cstr(value));

    furi_string_free(value);
}

void sim800_scene_saved_menu_on_enter(void* context) {
    Sim800App* app = context;

    sim800_entry_view_set_callback(app->entry_view, sim800_saved_menu_button_callback, app);
    sim800_entry_view_set_entry(
        app->entry_view, furi_string_get_cstr(app->entry_name), furi_string_get_cstr(app->number));
    sim800_entry_view_set_redial(app->entry_view, app->redial);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewEntry);
    sim800_task_start(app, sim800_saved_menu_task);
}

bool sim800_scene_saved_menu_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeTick) {
        sim800_entry_view_set_redial(app->entry_view, app->redial);
        if(!sim800_task_is_busy(app)) sim800_task_start(app, sim800_saved_menu_task);
        return true;
    }

    if(event.type != SceneManagerEventTypeCustom) return false;

    switch(event.event) {
    case Sim800CustomEventButtonCenter:
        app->mode = Sim800ModeCall;
        scene_manager_set_scene_state(app->scene_manager, Sim800SceneCall, 0);
        scene_manager_next_scene(app->scene_manager, Sim800SceneCall);
        return true;

    case Sim800CustomEventButtonLeft:
        app->mode = Sim800ModeSms;
        sim800_sms_compose(app);
        return true;

    case Sim800CustomEventButtonRight:
        scene_manager_next_scene(app->scene_manager, Sim800SceneEntryEdit);
        return true;

    default:
        return false;
    }
}

void sim800_scene_saved_menu_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
}
