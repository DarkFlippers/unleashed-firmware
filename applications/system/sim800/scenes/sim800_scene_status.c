#include "../sim800_app_i.h"

static void sim800_scene_status_task(Sim800App* app) {
    if(!sim800_uart_is_open(app->uart) || !sim800_uart_probe(app->uart)) {
        sim800_status_view_set_linked(app->status_view, false);
        return;
    }

    FuriString* value = furi_string_alloc();

    sim800_status_view_set_linked(app->status_view, true);
    sim800_status_view_set_signal(app->status_view, sim800_at_rssi(app->uart));

    sim800_at_operator(app->uart, value);
    sim800_status_view_set_operator(app->status_view, furi_string_get_cstr(value));

    sim800_at_registration(app->uart, value);
    sim800_status_view_set_registration(app->status_view, furi_string_get_cstr(value));

    sim800_at_sim_status(app->uart, value);
    sim800_status_view_set_sim(app->status_view, furi_string_get_cstr(value));

    sim800_at_power(app->uart, value);
    // Latched for the session: a brownout that already dropped the network is the whole reason
    // this screen gets opened, so it has to still be here when the user arrives
    if(app->power_alarm[0] != '\0') {
        furi_string_cat_str(
            value, (strstr(app->power_alarm, "UNDER") != NULL) ? " LOW!" : " HIGH!");
    }
    sim800_status_view_set_power(app->status_view, furi_string_get_cstr(value));

    furi_string_free(value);
}

static void sim800_scene_status_refresh(void* context) {
    Sim800App* app = context;
    if(!sim800_task_is_busy(app)) sim800_task_start(app, sim800_scene_status_task);
}

void sim800_scene_status_on_enter(void* context) {
    Sim800App* app = context;

    sim800_status_view_set_callback(app->status_view, sim800_scene_status_refresh, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewStatus);
    sim800_scene_status_refresh(app);
}

bool sim800_scene_status_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeTick) {
        sim800_scene_status_refresh(app);
        return true;
    }
    return false;
}

void sim800_scene_status_on_exit(void* context) {
    Sim800App* app = context;
    sim800_task_join(app);
}
